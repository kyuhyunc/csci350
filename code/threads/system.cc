// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *timer;				// the hardware timer device,
					// for invoking context switches

#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers
Lock* memlock;      // available physical memory
BitMap* memMap;
Table* locktable;		// other kernel data structures
Lock* locktablelock;
Table* cvtable;
Lock* cvtablelock;
Table* processTable;
Lock* processLock;

#ifdef USE_TLB
int currentTLB;		// TLB tracker
IPTentry* ipt;		// Inverted Page Table
List* iptFIFOqueue;	// IPT eviction
Lock* iptLock;      // IPT race condition prevention
OpenFile* swapfile;		// SWAP file
BitMap* swapMap;		// SWAP bitmap
evict_alg evict_type;
#endif // USE_TLB

#endif // USER_PROGRAM

#ifdef NETWORK
PostOffice *postOffice;
Lock* MailBoxInitNumLock;
int MailBoxInitNum;
int NumServers;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();


//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = "";
    bool randomYield = FALSE;

#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program

#ifdef USE_TLB
    evict_type = FIFO; // default evicting algorithm is FIFO
    srand(time(NULL));
#endif // USE_TLB

#endif // USER_PROGRAM
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
    MailBoxInitNumLock = new Lock("MailBoxInitNumLock");
    MailBoxInitNum = 0;
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	}
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#ifdef USE_TLB
    else if (!strcmp(*argv, "-P")) {
        ASSERT(argc > 1);
        char* type = *(argv + 1);
        if (!strcmp(type, "RAND")) {
            evict_type = RAND;
printf("evict_type = RAND\n");
        }
        else if (!strcmp(type, "FIFO")) {
            evict_type = FIFO;
printf("evict_type = FIFO\n");
        }
        else {
            printf("ERROR: -P can only take RAND or FIFO as arguments. Exiting now.\n");
            ASSERT(false);
        }
        argCount = 2;
    }
#endif // USE_TLB
#endif // USER_PROGRAM
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-numservers")) {
        NumServers = atoi(*(argv + 1));
        argCount = 2;
    }
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);

    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
	memlock = new Lock("MemoryLock");		// initialize kernel data trackers
	memMap = new BitMap(NumPhysPages);
	locktable = new Table(NumLocks);
	locktablelock = new Lock("LockTableLock");
	cvtable = new Table(NumCVs);
	cvtablelock = new Lock("CVTableLock");
	processTable = new Table(NumProcesses);
	processLock = new Lock("ProcessLock");

#ifdef USE_TLB
	currentTLB = 0;					// initialize TLB
	ipt = new IPTentry[NumPhysPages];	// initialize IPT
	iptFIFOqueue = new List();			// IPT eviction
	swapfile = fileSystem->Open("../vm/.swapfile");
	if (swapfile == NULL) {
		printf("Unable to open swapfile\n");
	}
	swapMap = new BitMap(SwapSize);
    iptLock = new Lock("IPTLock");
#endif // USE_TLB
#endif // USER_PROGRAM

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, NUM_MAILBOXES);
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
    delete memlock;
    delete memMap;
    delete locktable;
    delete cvtable;
    delete processTable;
    delete processLock;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    
    Exit(0);
}


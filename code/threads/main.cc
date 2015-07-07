// main.cc 
//	Bootstrap code to initialize the operating system kernel.
//
//	Allows direct calls into internal operating system functions,
//	to simplify debugging and testing.  In practice, the
//	bootstrap code would just initialize data structures,
//	and start a user program to print the login prompt.
//
// 	Most of this file is not needed until later assignments.
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//		-s -x <nachos file> -c <consoleIn> <consoleOut>
//		-f -cp <unix file> <nachos file>
//		-p <nachos file> -r <nachos file> -l -D -t
//              -n <network reliability> -m <machine id>
//              -o <other machine id>
//              -z
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//
//  USER_PROGRAM
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -c tests the console
//
//  FILESYS
//    -f causes the physical disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system 
//    -t tests the performance of the Nachos file system
//
//  NETWORK
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -o runs a simple test of the Nachos network software
//	  -s runs a server
//	  -c runs a client for Project 3
//
//  NOTE -- flags are ignored until the relevant assignment.
//  Some of the flags are interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"
#include <iostream>
#include <sstream>
#include "../network/post.h"

// External functions used by this file

extern void ThreadTest(void), Copy(char *unixFile, char *nachosFile);
extern void Print(char *file), PerformanceTest(void);
extern void StartProcess(char *file), ConsoleTest(char *in, char *out);
extern void MailTest(int networkID);

extern void AirportSim();
extern void AirTest();
extern void ClientSim(); // Project 3


#ifdef NETWORK
//----------------------------------------------------------------------
//
//	Server implementation starts
//	
//----------------------------------------------------------------------
void Server() {
	printf("Made it to server\n");

	int farAddr = 0;

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    char *data = "Hello there!";
    char *ack = "Got it!";

    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    // bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    // if ( !success ) {
    //   printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
    //   interrupt->Halt();
    // }

    while (true) {
    	// Receive the next message
    	printf("About to get my message\n");
    	postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);	
    	// printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    	printf("Received my message\n");
    	fflush(stdout);

    	// Decode the message
    	int type = 0; // TODO - grab the first two bytes

    	// Figure out which server function to run, switch-case statement
    	switch (type) {
    		case CreateLock_SF : 
    			printf("CreateLock_SF\n");
    			interrupt->Halt();
    			break;
    	}
    }

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    // outPktHdr.to = inPktHdr.from;
    // outMailHdr.to = inMailHdr.from;
    // outMailHdr.length = strlen(ack) + 1;
    // success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    // if ( !success ) {
    //   printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
    //   interrupt->Halt();
    // }

    // Wait for the ack from the other machine to the first message we sent.
    // postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    // printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    // fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

/*
	CreateLock
*/
// int CreateLock() {

// }

#endif
/* end Server */


//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.  
//	
//	Check command line arguments
//	Initialize data structures
//	(optionally) Call test procedure
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int
main(int argc, char **argv)
{
    int argCount;			// the number of arguments 
					// for a particular command

    DEBUG('t', "Entering main");
    (void) Initialize(argc, argv);
    
#ifdef THREADS
//    ThreadTest();
	while(true) {
		int i;
		std::cout<<"Select Menu"<<std::endl;

		std::cout<<"1. TESTING "<<std::endl;
		std::cout<<"2. SIMULATION"<<std::endl;
		std::cin>>i;
		if(i == 1) {
			std::cout<<"You chose TESTING"<<std::endl;
			AirTest();
			break;
		}else if(i == 2) {
			std::cout<<"You chose SIMUATION"<<std::endl;
			AirportSim();
			break;
		}else{
			std::cout<<"You select wrong number try again."<<std::endl;
			continue;
		}
	}

#endif

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
        if (!strcmp(*argv, "-z"))               // print copyright
            printf (copyright);
#ifdef USER_PROGRAM
        if (!strcmp(*argv, "-x")) {        	// run a user program
	    ASSERT(argc > 1);
            StartProcess(*(argv + 1));
            argCount = 2;
        } else if (!strcmp(*argv, "-c")) {      // test the console
	    if (argc == 1)
	        ConsoleTest(NULL, NULL);
	    else {
		ASSERT(argc > 2);
	        ConsoleTest(*(argv + 1), *(argv + 2));
	        argCount = 3;
	    }
	    interrupt->Halt();		// once we start the console, then 
					// Nachos will loop forever waiting 
					// for console input
	}
#endif // USER_PROGRAM
#ifdef FILESYS
	if (!strcmp(*argv, "-cp")) { 		// copy from UNIX to Nachos
	    ASSERT(argc > 2);
	    Copy(*(argv + 1), *(argv + 2));
	    argCount = 3;
	} else if (!strcmp(*argv, "-p")) {	// print a Nachos file
	    ASSERT(argc > 1);
	    Print(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-r")) {	// remove Nachos file
	    ASSERT(argc > 1);
	    fileSystem->Remove(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-l")) {	// list Nachos directory
            fileSystem->List();
	} else if (!strcmp(*argv, "-D")) {	// print entire filesystem
            fileSystem->Print();
	} else if (!strcmp(*argv, "-t")) {	// performance test
            PerformanceTest();
	}
#endif // FILESYS
#ifdef NETWORK
        if (!strcmp(*argv, "-o")) {
	    ASSERT(argc > 1);
            Delay(2); 				// delay for 2 seconds
						// to give the user time to 
						// start up another nachos
            MailTest(atoi(*(argv + 1)));
            argCount = 2;
        } else if (!strcmp(*argv, "-s")) {
        	Server();
        }
#endif // NETWORK
    }
    currentThread->Finish();	// NOTE: if the procedure "main" 
				// returns, then the program "nachos"
				// will exit (as any other normal program
				// would).  But there may be other
				// threads on the ready list.  We switch
				// to those threads by saying that the
				// "main" thread is finished, preventing
				// it from returning.
    return(0);			// Not reached...
}
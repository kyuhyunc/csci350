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
#include <string>
#include <vector>

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
//  Server Lock/CV implementation starts
//  
//----------------------------------------------------------------------

Lock* SLock = new Lock("SLock");
Lock* CVLock = new Lock("CVLock");

enum state {
    AVAIL, BUSY
};

class ServerLock {
    
public:
    ServerLock(int s, int o, std::string n) {
        state = s;
        owner = o;
        name = n;
    }
public:
    int state;
    int owner;
    std::string name;
    List * waitQ;
    bool toBeDeleted;

};

class ServerCV {
    /*
public:
    ServerLock(int s, int o, std::string n) {
        state = s;
        owner = o;
        name = n;
    }
public:
    int state;
    int owner;
    std::string name;
    List * waitQ;
    bool toBeDeleted;
    */

};

std::vector<ServerLock*> ServerLockVector;
std::vector<ServerCV*> ServerCVVector;

void initializeNetworkMessageHeaders(const PacketHeader &inPktHdr, PacketHeader &outPktHdr, const MailHeader &inMailHdr, MailHeader &outMailHdr, int dataLength) {
    outPktHdr.to = inPktHdr.from;
    outPktHdr.from = inPktHdr.to;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = inMailHdr.to;
    outMailHdr.length = dataLength + 1;
}

void CreateLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name) {

    SLock->Acquire();
    //iterating through serverlockVector to check lock is already in vector
    //if other program already create lock with the same name, don't create new lock
    //just return(send the message) the index to user(Client)
    int index = -1;
    for(unsigned int i = 0; i < ServerLockVector.size(); i++) {
        if(ServerLockVector[i] != NULL) {
            if(ServerLockVector[i]->name == name) {
            index = i;   
            break;
            }
        }
    }

    if(index = -1) {

        index = ServerLockVector.size();

        ServerLock * sLock = new ServerLock(AVAIL, inPktHdr.from, name);
        sLock->toBeDeleted = false;
        sLock->waitQ = new List();
        ServerLockVector.push_back(sLock);
    }

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    ss << index;

    char *data = new char[ss.str().length()];
    std::strcpy(data, ss.str().c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str().length());

    if(!postOffice->Send(outPktHdr, outMailHdr, data)) {
        DEBUG('n', "Something bad happens in Server. Unable to send message \n");
        SLock->Release();
    }

    DEBUG('n', "Server is returning a lock index of %d\n", index);

    SLock->Release();

}
void DestroyLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    //if no client currently acquire the lock, server simply delete the lock
    //otherwise, set boolean toBeDeleted true so it can be deleted when the lock is done using.
    SLock->Acquire();
    
    DEBUG('n', "Server is destroying a lock\n");

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    if(index < 0 || index >= ServerLockVector.size()) {
        //if invalid index is passed in, then you need to 
        printf("Lock does not exist in vector.(Destory)\n.");
        //send -1 to client so client know they can't properly destroy lock.
        ss << -1;
    }else if(ServerLockVector[index] == NULL ) {
        //if the lock is already deleted or null for some reasons, print error message
        printf("Lock does not exist in vector.(Destory)\n.");
        //send -1 to client so client know they can't properly destroy lock.
        ss << -1;

    } else {
        ss << index;
        
        if(ServerLockVector[index]->state == AVAIL) {
                ServerLockVector[index]->toBeDeleted = true;
                ServerLockVector[index] = NULL;
        }else{
                ServerLockVector[index]->toBeDeleted = true;
        }
    } 
    //send the message to client
    char *data = new char[ss.str().length()];
    std::strcpy(data, ss.str().c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));

    DEBUG('n', "Server destroyed the lock\n");

    if(!postOffice->Send(outPktHdr, outMailHdr, data)) {
        printf("Something bad happens in Server. Unable to send message \n");
        SLock->Release();
    }

    SLock->Release();

}
void CreateCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name) {
    /*CVLock->Acquire();
    //iterating through serverlockVector to check CV is already in vector
    //if other program already create lock with the same name, don't create new lock
    //just return(send the message) the index to user(Client)
    int index = -1;
    for(int i = 0; i < ServerCVVector.size(); i++) {
        if(ServerCVVector[i] != NULL) {
            if(ServerCVVector[i]->name == name) {
            index = i;   
            break;
            }
        }
    }

    if(index = -1) {

        index = ServerCVVector.size();

        ServerCV * sCV = new ServerCV(AVAIL, inPktHdr.from, name);
        sCV->toBeDeleted = false;
        sCV->waitQ = new List();
        ServerCVVector.push_back(sLock);
    }

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    ss << index;

    char *data = new char[ss.str().length()];
    std::strcpy(data, ss.str().c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));

    if(!postOffice->Send(outPktHdr, outMailHdr, data)) {
        printf("Something bad happens in Server. Unable to send message \n");
        CVLock->Release();
        interrupt->Halt();
    }

    CVLock->Release();
    */
}
void DestroyCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    //if no client currently acquire the lock, server simply delete the lock
    //otherwise, set boolean toBeDeleted true so it can be deleted when the lock is done using.
   /* CVLock->Acquire();
    
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;

    //if the lock is already deleted or null for some reasons, print error message
    if(ServerLockVector[index] == NULL) {
        printf("CV does not exist in vector.(Destory)\n.");
        //send -1 to client so client know they can't properly destroy lock.
        ss << -1;
        char *data = new char[ss.str().length()];
        std::strcpy(data, ss.str().c_str());

        initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));

        CVLock->Release();
        interrupt->Halt();
        return;
    }
    if(ServerCVVector[index]->state == AVAIL) {
            ServerCVVector[index]->toBeDeleted = true;
            ServerCVVector[index] = NULL;
    }else{
            ServerCVVector[index]->toBeDeleted = true;
    }

    ss << index;

    char *data = new char[ss.str().length()];
    std::strcpy(data, ss.str().c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));
    CVLock->Release();
    */
}
void Acquire(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    
    SLock->Acquire();
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock is null or in the vector. Send the error message if client can't properly acquire the lock
    if(index < 0 || index >= ServerLockVector.size()) {
        printf("Invalid index is passed in. Can't Acquire Lock.(Acquire)\n.");
        ss << -1;

    }else if(ServerLockVector[index] == NULL) {
        printf("Lock you try to acquire is already deleted. Can't Acquire Lock(Acquire)\n ");
        ss << -1;
    }else {
        ss << index;
        //if state is busy, that means lock is already acquired.
        //Therefore, put the id in waitqueue so it can be acquired
        //when current thread release lock.
        if(ServerLockVector[index]->state == BUSY) {
            ServerLockVector[index]->waitQ->Append((void*)inPktHdr.from);
        }
    } 

    char *data = new char[ss.str().length()];
    std::strcpy(data, ss.str().c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));
    if(!postOffice->Send(outPktHdr, outMailHdr, data)) {
        printf("Something bad happens in Server. Unable to send message \n");
    }
    SLock->Release();
    
}
void Release(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    SLock->Acquire();

    //Check if the lock is null or in the vector. send the error message if client can't properly release the lock
    if(ServerLockVector[index] == NULL) {
        printf("No Lock client can release!\n");
    }

    //check if client is waiting for acquire the lock after releasing the lock
    if(!ServerLockVector[index]->waitQ->IsEmpty()) {
        int nextClient = (int)ServerLockVector[index]->waitQ->Remove();
    }



    SLock->Release();
    
}
void Wait() {

}
void Signal() {

}
void BroadCast() {

}



//----------------------------------------------------------------------
//
//	Server implementation starts
//	
//----------------------------------------------------------------------
void Server() {

	// initialized global data
	SLock = new Lock("ServerLock");

	// instantiate Network Data
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    while (true) {
    	// Receive the next message
    	DEBUG('n', "Server about to receive a message\n");
    	postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);	
    	// printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    	DEBUG('n', "Server received a message\n");
    	fflush(stdout);

    	// Decode the message
    	int type = -1; 
        std::string name;
        int index;
    	std::stringstream ss(buffer);
        ss>>type;

    	// Figure out which server function to run, switch-case statement
    	switch (type) {
    		case CreateLock_SF : 
    			printf("CreateLock_SF\n");
                ss>>name;
                CreateLock(inPktHdr, inMailHdr, name);
    			break;
            case DestroyLock_SF : 
                printf("DestroyLock_SF\n");
                ss>>index;
                DestroyLock(inPktHdr, inMailHdr, index);
                break;
            case CreateCV_SF : 
                printf("CreateCV_SF\n");
                ss>>name;
                //CreateCV(inPktHdr, inMailHdr, name);
                interrupt->Halt();
                break;
            case DestroyCV_SF : 
                printf("DestroyCV_SF\n");
                ss>>index;
                //DestroyCV(inPktHdr, inMailHdr, index);
                interrupt->Halt();
                break;
            case Acquire_SF : 
                printf("Acquire_SF\n");
                ss>>index;
                Acquire(inPktHdr, inMailHdr, index);
                interrupt->Halt();
                break;
            case Release_SF : 
                printf("Release_SF\n");
                ss>>index;
                Release(inPktHdr, inMailHdr, index);
                interrupt->Halt();
                break;
            case Wait_SF : 
            	DEBUG('n', "Server is running Wait_SF\n");
                interrupt->Halt();
                break;
            case Signal_SF : 
            	DEBUG('n', "Server is running Signal_SF\n");
                interrupt->Halt();
                break;
            case BroadCast_SF : 
            	DEBUG('n', "Server is running BroadCast_SF\n");
                interrupt->Halt();
                break;
    	}
    }

    delete SLock;

    // Then we're done!
    interrupt->Halt();
}

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
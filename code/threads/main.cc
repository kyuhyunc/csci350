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

extern void AirportSim();   // Project 1 Airport Simulation
extern void AirTest();      // Project 1 Airport Testing
extern void ClientSim();    // Project 3 Networking

#ifdef NETWORK
//----------------------------------------------------------------------
//  Server Lock/CV Global Variables and Function Prototypes
//----------------------------------------------------------------------

//Server
List* sortedQueue;
int64_t* lastTimeStampReceived;

Lock* SLock;
Lock* MVLock;
Lock* CVLock;

int forwardedServer;
int currentFS; // current forwarded server

enum state {
    AVAIL, BUSY
};

struct SendDestination {
    MailBoxAddress mailbox;
    NetworkAddress machineID;
};

class ServerLock {
public:
    ServerLock(int s, int o, std::string n) {
        state = s;
        owner = o;
        name = n;
        clientCounter = 1;
    }
public:
    int state;
    int owner;
    std::string name;
    List * waitQ;
    int clientCounter;
};

class ServerCV {
public:
    ServerCV(int o, std::string n) {
        owner = o;
        name = n;
        toBeDeleted = false;
        waitQ = new List();
        waitingLock = NULL;
        clientCounter = 1;
    }
public:
    int owner;
    std::string name;
    List * waitQ;
    bool toBeDeleted;
    ServerLock * waitingLock;
    int CVCounter;
    int clientCounter;
};

class MonitorVariable {
public:
//    MonitorVariable(const int vectSize, const std::string &n) 
    MonitorVariable(const int size, const std::string &name)
    :_size(size), _name(name) {
        _data = new int[size];
        for (int i=0; i < size; i++) {
            _data[i] = 0;
        }
    }
    ~MonitorVariable() {
        delete _data;
    }
//    int size() { return vector.size(); }
//    int& at(const int &index) { return vector.at(index); }
//    void setAt(const int &index, const int &value) { vector.at(index) = value; }

    // accessors
    int getSize() const { return _size; }
    std::string getName() const { return _name; }

    // mutators
    int getData(const int index) const { return _data[index]; }
    void setData(const int index, const int val) { _data[index] = val; }

//    std::string name;
private:
    int * _data;
    int _size;
    std::string _name;
//    std::vector<int> vector;
};


std::vector<ServerLock*> ServerLockVector;
std::vector<ServerCV*> ServerCVVector;
std::vector< MonitorVariable* > MonitorVars;

// Function Prototypes
void initializeNetworkMessageHeaders(const PacketHeader &inPktHdr, PacketHeader &outPktHdr, const MailHeader &inMailHdr, MailHeader &outMailHdr, int dataLength);
void sendMessageToClient(const PacketHeader &inPktHdr, PacketHeader &outPktHdr, const MailHeader &inMailHdr, MailHeader &outMailHdr, const std::string msg);
int64_t GetTimeStamp();
void forwardMessageToAllServers();
void CreateLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name);
void DestroyLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index);
void CreateCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name);
void DestroyCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index);
void Acquire(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index);
void ReleaseFromWaitQ(const PacketHeader &inPktHdr, PacketHeader &outPktHdr, const MailHeader &inMailHdr, MailHeader &outMailHdr, const int &lockIndex);
void Release(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index);
void Wait(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex);
std::string SignalFunctionality(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex);
void Signal(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex);
void BroadCast(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex);
void CreateMV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &size,const std::string &name);
void GetMV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int mv, const int index);
void SetMV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int mv, const int index, const int value);
void DestroyMV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int mv);
void ServerFromClient();
void ServerFromServer();


#endif // NETWORK



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

    DEBUG('t', "Entering main\n");
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

#endif // THREADS

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
            // For distributed servers,
            // Create 2 threads:
                // 1 for receiving requests from clients
                // 1 for receiving requests from servers (including itself)
            Thread* sfc = new Thread("ServerFromClient");
            sfc->Fork((VoidFunctionPtr)ServerFromClient, 0);
            Thread* sfs = new Thread("ServerFromServer");
            sfs->Fork((VoidFunctionPtr)ServerFromServer, 1);
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

#ifdef NETWORK
//----------------------------------------------------------------------
//
//  Server Lock/CV implementation starts
//  
//----------------------------------------------------------------------


void initializeNetworkMessageHeaders(
		const PacketHeader &inPktHdr, 
		PacketHeader &outPktHdr, 
		const MailHeader &inMailHdr, 
		MailHeader &outMailHdr, 
		int dataLength
) {
    outPktHdr.to = inPktHdr.from;
    outPktHdr.from = inPktHdr.to;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = inMailHdr.to;
    outMailHdr.length = dataLength + 1;
}

void sendMessageToClient(
		const PacketHeader &inPktHdr, 
		PacketHeader &outPktHdr, 
    	const MailHeader &inMailHdr, 
    	MailHeader &outMailHdr, 
    	const std::string msg
) {
    char *data = new char[msg.length()];
    std::strcpy(data, msg.c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));
/*printf("inPktHdr.to = %i\n", inPktHdr.to);
printf("inPktHdr.from = %i\n", inPktHdr.from);
printf("inMailHdr.to = %i\n", inMailHdr.to);
printf("inMailHdr.from = %i\n", inMailHdr.from);*/
DEBUG('o', "About to send message to client\n");
    if(!postOffice->Send(outPktHdr, outMailHdr, data)) {
        printf("Something bad happens in Server. Unable to send message \n");
    }
    /*delete[] data;*/
}
/*
int64_t GetTimeStamp() {
    // Find # seconds from year 2000
    time_t t;
    time(&t);

    // Find # microseconds
    struct timeval tv;
    gettimeofday(&tv,NULL);
    time_t microseconds = tv.tv_usec;

//    printf("t = %ld\n", t);
//    printf("usec = %ld\n", tv.tv_usec);
    int64_t a = *((int64_t*)&t);
//    printf("a = %llu\n", a);
    int64_t b = a * 1000000;
//    printf("b = %llu\n", b);
    int64_t c = b + *((int64_t*)&tv.tv_usec);
//    printf("c = %llu\n", c);
    int64_t d = c * 10 + 0;

    return d;
}
*/
void forwardMessageToAllServers() {
    // append timestamp to message
    
}

void CreateLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name) {

    SLock->Acquire();
    DEBUG('o', "Server called CreateLock\n");
    //iterating through serverlockVector to check lock is already in vector
    //if other program already create lock with the same name, don't create new lock
    //just return(send the message) the index to user(Client)
    int index = -1;
    for(unsigned int i = 0; i < ServerLockVector.size(); i++) {
        if (ServerLockVector[i] != NULL) {
            if(ServerLockVector[i]->name.compare(name) == 0) {
	            index = i;
                ServerLockVector[i]->clientCounter++;
	            break;
            }
	    }
    }
    if(index == -1) {

        index = ServerLockVector.size();

        ServerLock * sLock = new ServerLock(AVAIL, inPktHdr.from, name);
        sLock->waitQ = new List();
        ServerLockVector.push_back(sLock);
    }
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    ss << index;

    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d a lock index of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    DEBUG('o', "Server is returning a lock index of %d\n", index);

    SLock->Release();

}
void DestroyLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    //if no client currently acquire the lock, server simply delete the lock
    //otherwise, set boolean toBeDeleted true so it can be deleted when the lock is done using.
    SLock->Acquire();
    
    DEBUG('o', "Server called DestroyLock\n");

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    if(index < 0 || static_cast<unsigned int>(index) >= ServerLockVector.size()) {
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
        ServerLockVector[index]->clientCounter--;
        if(ServerLockVector[index]->state == AVAIL && ServerLockVector[index]->clientCounter == 0) {
        	DEBUG('o', "SERVER is deleting lock %d\n", index);
            ServerLockVector[index] = NULL;
            delete ServerLockVector[index];
        } 
    } 
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    SLock->Release();
}
void CreateCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name) {
    CVLock->Acquire();
    DEBUG('o', "Server called CreateCV\n");
    //iterating through serverCVVector to check CV is already in vector
    //if other program already create CV with the same name, don't create new CV
    //just return(send the message) the index to user(Client)
    int index = -1;
    for(unsigned int i = 0; i < ServerCVVector.size(); i++) {
        if(ServerCVVector[i] != NULL) {
            if(ServerCVVector[i]->name == name) {
                ServerCVVector[i]->clientCounter++;
            	index = i;   
            break;
            }
        }
    }

    if(index == -1) {
        index = ServerCVVector.size();
        ServerCV * sCV = new ServerCV(inPktHdr.from, name);
        sCV->waitQ = new List();
        sCV->CVCounter = 0;
        ServerCVVector.push_back(sCV);
    }

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    ss << index;

    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d a CV index of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    DEBUG('o', "Server is returning a CV index of %d\n", index);

    CVLock->Release();
}
void DestroyCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    //if no client currently acquire the lock, server simply delete the lock
    //otherwise, set boolean toBeDeleted true so it can be deleted when the lock is done using.
    CVLock->Acquire();
    
    DEBUG('o', "Server called DestroyCV\n");

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    if(index < 0 || static_cast<unsigned int>(index) >= ServerCVVector.size()) {
        //if invalid index is passed in, then you need to 
        printf("CV does not exist in vector.(DestoryCV)\n.");
        //send -1 to client so client know they can't properly destroy lock.
        ss << -1;
    }else if(ServerCVVector[index] == NULL ) {
        //if the lock is already deleted or null for some reasons, print error message
        printf("CV does not exist in vector.(DestoryCV)\n.");
        //send -1 to client so client know they can't properly destroy lock.
        ss << -1;
    } else {
        ss << index;
        //if all clients that create CV try to destroy CV, then server delete CV
        ServerCVVector[index]->clientCounter--;
        if(ServerCVVector[index]->clientCounter == 0) {
            DEBUG('o', "Server is deleting CV %d\n", index);
            ServerCVVector[index] = NULL;
            delete ServerCVVector[index];
        }
    } 
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d a destroyed CV index of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    DEBUG('o', "Server is returning a destroyed CV index of %d\n", index);
    CVLock->Release();
}
void Acquire(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    SLock->Acquire();
    DEBUG('o', "Server called Acquire\n");
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock is null or in the vector. Issue error message if client can't properly acquire the lock
    if(index < 0 || static_cast<unsigned int>(index) >= ServerLockVector.size()) {
        printf("Invalid lock index: %d in Acquire, must be between 0 and %d, can't acquire lock\n", index, MonitorVars.size());
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
        	DEBUG('o', "Lock is busy, machine %d mailbox %d is going on waitQ\n", inPktHdr.from, inMailHdr.from);

            // store address destination
            SendDestination * sd = new SendDestination();
            sd->mailbox = inMailHdr.from;
            sd->machineID = inPktHdr.from;

            ServerLockVector[index]->waitQ->Append((void*) sd);
            SLock->Release();
            return;
        } else { // lock is available! "Acquire" this lock
        	DEBUG('o', "Lock is available, machine %d mailbox %d acquired lock\n", inPktHdr.from, inMailHdr.from);
        	ServerLockVector[index]->state = BUSY;
        }
    } 
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d an acquired lock index of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    DEBUG('o', "Server is returning an acquired lock index of %d\n", index);

    SLock->Release();
    
}

void ReleaseFromWaitQ(	const PacketHeader &inPktHdr, 
						PacketHeader &outPktHdr, 
    					const MailHeader &inMailHdr, 
						MailHeader &outMailHdr, 
    					const int &lockIndex
) {

	if(!ServerLockVector[lockIndex]->waitQ->IsEmpty()) {
    	SendDestination * nextClient = (SendDestination*) ServerLockVector[lockIndex]->waitQ->Remove();
        DEBUG('o', "Lock->Release -- Giving lock %d to thread %d in waitQueue\n", lockIndex, nextClient);
        std::stringstream ss;
        ss << lockIndex;

        PacketHeader waitPktHdr;
	    waitPktHdr.to = inPktHdr.to;
	    waitPktHdr.from = nextClient->machineID;

	    MailHeader waitMailHdr;
	    waitMailHdr.to = inMailHdr.to;
	    waitMailHdr.from = nextClient->mailbox; // TODO - Project 4

        // everyone creates the lock, but only the server that received the request
        // from the client sends a message back to the client
        // this is to preserve global data alignment
        if (currentFS == postOffice->getMachineID()) {
            sendMessageToClient(waitPktHdr, outPktHdr, waitMailHdr, outMailHdr, ss.str());
        }

    } else {
        ServerLockVector[lockIndex]->state = AVAIL;
    }

	
}
void Release(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    SLock->Acquire();
    DEBUG('o', "Server called Release\n");

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if invalid pointer is passed and Lock is NULL
    //if so, server needs to send -1 to client so client know there's something wrong 
    if(index < 0 || static_cast<unsigned int>(index) >= ServerLockVector.size()) {
        printf("Invalid index is passed in. Can't Acquire Lock.(Acquire)\n.");
        ss << -1;

    }else if(ServerLockVector[index] == NULL) {
        printf("Lock you try to release is already deleted. Can't Acquire Lock(Release)\n ");
        ss << -1;
    }else {
        if(!ServerLockVector[index]->waitQ->IsEmpty()) {
            ReleaseFromWaitQ(inPktHdr, outPktHdr, inMailHdr, outMailHdr, index);
        } else if (ServerLockVector[index]->clientCounter == 0) {
        	DEBUG('o', "Lock->Release() -- deleted isToBeDeleted lock\n");
        	ServerLock* sl = ServerLockVector[index];
        	ServerLockVector[index] = NULL;
        	delete sl;
        } else {
        	DEBUG('o', "Lock->Release -- Lock is now available\n");
            ServerLockVector[index]->state = AVAIL;
        }
        ss << index;
    }
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d a release lock index of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    DEBUG('o', "Server is returning a released lock index of %d\n", index);
    SLock->Release();
    
}
void Wait(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex) {
	CVLock->Acquire();
    DEBUG('o', "Server called Wait\n");

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock/CV is null or in the vector. Issue error message if client can't properly wait
    if(LockIndex < 0 || static_cast<unsigned int>(LockIndex) >= ServerLockVector.size()) {
        printf("Invalid Lock index is passed in. Can't process wait.(Wait)\n");
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;

    }else if(CVIndex < 0 || static_cast<unsigned int>(CVIndex) >= ServerCVVector.size()) {
        printf("Invalid CV index is passed in. Can't process wait.(Wait)\n");
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;

    }else if(ServerLockVector[LockIndex] == NULL) {
        printf("Lock you try to wait is already deleted. Can't process wait.(Wait)\n");
        ss << -1;  
    }else if(ServerCVVector[CVIndex] == NULL) {
        printf("Lock you try to wait is already deleted. Can't process wait.(Wait)\n");
        ss << -1;  
    }else {
        // If this is the first time Wait is being called with this lock, then set the lock variable
        if(ServerCVVector[CVIndex]->waitingLock == NULL) {
            ServerCVVector[CVIndex]->waitingLock = ServerLockVector[LockIndex];
        }
        // If this is not the first thread to call wait, AND they passed in the incorrect lock, send error
        if(ServerCVVector[CVIndex]->waitingLock != ServerLockVector[LockIndex]) {
            printf("Parameter lock does not match the waitingLock.(Wait)\n");
            printf("Parameter lock = %s, waitingLock = %s\n", ServerCVVector[CVIndex]->waitingLock->name.c_str(), ServerLockVector[LockIndex]->name.c_str());
            ss << -1;
        }else{
        	// everything is good to go! Thread will wait until Signal is called
            ReleaseFromWaitQ(inPktHdr, outPktHdr, inMailHdr, outMailHdr, LockIndex);
            
            // store send location
            SendDestination * sd = new SendDestination();
            sd->mailbox = inMailHdr.from;
            sd->machineID = inPktHdr.from;

            ServerCVVector[CVIndex]->waitQ->Append((void*) sd);
            ServerCVVector[CVIndex]->CVCounter++;
            CVLock->Release();
            return; // Don't send a message
        }
    }
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d a wait error message\n", inPktHdr.from, inMailHdr.from);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); // send error message
    }

    DEBUG('o', "Server is returning a wait error message\n");
    CVLock->Release();
}

// Signals the person waiting
std::string SignalFunctionality(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int &LockIndex, 
    const int &CVIndex
) {
    CVLock->Acquire();
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock/CV is null or in the vector. Issue error message if client can't properly signal
    if(LockIndex < 0 || static_cast<unsigned int>(LockIndex) >= ServerLockVector.size()) {
        printf("Invalid Lock index is passed in. Can't process Signal.(Signal)\n");
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;

    }else if(CVIndex < 0 || static_cast<unsigned int>(CVIndex) >= ServerCVVector.size()) {
        printf("Invalid CV index is passed in. Can't process Signal.(Signal)\n");
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;

    }else if(ServerLockVector[LockIndex] == NULL) {
        printf("Lock you try to signal is already deleted. Can't process Signal.(Signal)\n");
        ss << -1;  
    }else if(ServerCVVector[CVIndex] == NULL) {
        printf("Lock you try to signal is already deleted. Can't process Signal.(Signal)\n");
        ss << -1;  
    }else {
        //if there is nothing to signal, then send the error message to client.
        if(ServerCVVector[CVIndex]->waitQ->IsEmpty()) {
            printf("There is nothing to signal for Lock %d CV %d.(Signal)\n", LockIndex, CVIndex);
            printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
            ss << -1;
        }
        // If this is not the first thread to call wait, AND they passed in the incorrect lock, send error
        if(ServerCVVector[CVIndex]->waitingLock != ServerLockVector[LockIndex]) {
            printf("Condition Lock %d does not match waitingLock.(Signal)\n", LockIndex);
            printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
            ss << -1;
        }else{

            // everything is good to go! Waiting thread will be signaled
            SendDestination * nextClient = (SendDestination*) ServerCVVector[CVIndex]->waitQ->Remove();

            SLock->Acquire();
            ServerLockVector[LockIndex]->waitQ->Append((void*) nextClient);
            SLock->Release();
/*
            ss << nextClient;

            PacketHeader waitPktHdr;
            waitPktHdr.to = inPktHdr.to;
            waitPktHdr.from = nextClient->machineID;

            MailHeader waitMailHdr;
            waitMailHdr.to = inMailHdr.to;
            waitMailHdr.from = nextClient->mailbox; // TODO - Project 4

            // everyone creates the lock, but only the server that received the request
            // from the client sends a message back to the client
            // this is to preserve global data alignment
            if (currentFS == postOffice->getMachineID()) {
                sendMessageToClient(waitPktHdr, outPktHdr, waitMailHdr, outMailHdr, ss.str());
            }
*/
            ServerCVVector[CVIndex]->CVCounter--;

            if(ServerCVVector[CVIndex]->waitQ->IsEmpty()) {
                ServerCVVector[CVIndex]->waitingLock = NULL;
            }
            //if all clients that create CV try to destroy CV, then server delete CV 
            if(ServerCVVector[CVIndex]->clientCounter == 0) {
                ServerCV* sCV = ServerCVVector[CVIndex];
                ServerCVVector[CVIndex] = NULL;
                delete sCV;
            }
            ss.str("");
            ss << CVIndex;
        }

    }
    CVLock->Release();
    return ss.str();
}

void Signal(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex) {
    DEBUG('o', "Server called Signal\n");
    // Signals the thread waiting
    std::string ss = SignalFunctionality(inPktHdr, inMailHdr, LockIndex, CVIndex);

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        // Send message to the thread that called Signal
        DEBUG('o', "Server sent client %d mailbox %d a confirmation of signal\n", inPktHdr.from, inMailHdr.from);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss);
    }

    DEBUG('o', "Server is returning a confirmation of signal\n");
}
void BroadCast(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex) {
    DEBUG('o', "Server called BroadCast\n");
    while(ServerCVVector[CVIndex]->CVCounter != 0) {
        // Signals the thread waiting
        SignalFunctionality(inPktHdr, inMailHdr, LockIndex, CVIndex);
    }
    std::stringstream ss;
    ss << LockIndex;

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        // Send message to the thread that called Broadcast
        DEBUG('o', "Server sent client %d mailbox %d a confirmation of broadcast\n", inPktHdr.from, inMailHdr.from);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    }

    DEBUG('o', "Server is returning a confirmation of broadcast\n");
}

/*
*   Monitor Variable Methods
*/

void CreateMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int &size,
    const std::string &name
) {
    MVLock->Acquire();
    DEBUG('o', "Server called CreateMV\n");
    std::stringstream ss;
    int index = -1;

    // Try to find pre-existing MV
    for(unsigned int i = 0; i < MonitorVars.size(); i++) {
        if (MonitorVars[i] != NULL) {
            if(MonitorVars[i]->getName() == name && MonitorVars[i]->getSize() == size) {
                index = i;   
                ss << i;
                break;
            }
        }
    }

    // Create a new MV (if conditions are met)
    if(index == -1) {
        // If size is 0 or less, invalid size
        // Therefore, don't create MV
        if (size < 1) {
            printf("Invalid monitor variable size of %d in CreateMV\n", size);
            ss << -1;
        } else {
            MonitorVars.push_back(new MonitorVariable(size, name));
            index = MonitorVars.size() - 1;
            ss << index;
        } 
    }
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d an MV index of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    }

    DEBUG('o', "Server is returning an MV index of %d\n", index);

    MVLock->Release();
}

// Returns the value of the index at the MV
void GetMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int mv, 
    const int index) 
{
    MVLock->Acquire();
    DEBUG('o', "Server called GetMV\n");
    std::stringstream ss;
    // Check if client passed in MV within bounds
    if (mv < 0 || static_cast<unsigned int>(mv) >= MonitorVars.size()) {
        printf("Invalid MV: %d in GetMV, size = %d\n", mv, MonitorVars.size());
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;
    }
    // Check if MV exists
    else if (MonitorVars.at(mv) == NULL) {
        printf("Monitor Variable: %d is null inGetMV\n", mv);
        ss << -1;
    }
    // Check if index is within bounds
    else if (index < 0 || index >= MonitorVars.at(mv)->getSize()) {
        printf("Invalid index: %d in GetMV for mv %d (%s), size = %d\n", index, mv, MonitorVars.at(mv)->getName().c_str(), MonitorVars.at(mv)->getSize());
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;
    }
    // All checks passed
    else {
        ss << MonitorVars.at(mv)->getData(index);
    }
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d an MV value of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    }

    DEBUG('o', "Server is returning an MV value of %d\n", index);

    MVLock->Release();
}

// Sets the value of the index at the MV
void SetMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int mv, 
    const int index, 
    const int value) 
{
    MVLock->Acquire();
    DEBUG('o', "Server called SetMV\n");
    //
    std::stringstream ss;
    // Check if client passed in MV within bounds
    if ( mv < 0 || static_cast<unsigned int>(mv) >= MonitorVars.size() ) {
        printf("Invalid MV: %d in SetMV, size = %d\n", mv, MonitorVars.size());
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;
    }
    // Check if MV exists
    else if (MonitorVars.at(mv) == NULL) {
        printf("Monitor Variable: %d is null inGetMV\n", mv);
        ss << -1;
    }
    // Check if index is within bounds
    else if ( index < 0 || index >= MonitorVars.at(mv)->getSize() ) {
        printf("Invalid index: %d in SetMV for mv %d (%s), size = %d\n", index, mv, MonitorVars.at(mv)->getName().c_str(), MonitorVars.at(mv)->getSize());
        printf("Came from machine %d mailbox %d\n", inPktHdr.from, inMailHdr.from);
        ss << -1;
    }
    // All checks passed
    else {
//        MonitorVars.at(mv)->at(index) = value;
//        MonitorVars.at(mv)->setAt(index, value);
        MonitorVars.at(mv)->setData(index, value);
        ss << value;
    }
    //
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        DEBUG('o', "Server sent client %d mailbox %d a newly set MV value of %d\n", inPktHdr.from, inMailHdr.from, index);
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    }

    DEBUG('o', "Server set an MV value of %d\n", index);

    //
    MVLock->Release();
}

void DestroyMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int mv) 
{
    MVLock->Acquire();
    DEBUG('o', "Server called DestroyMV\n");
    //
    std::stringstream ss;

    // Check if client passed in valid MV number
    if (mv < 0 || static_cast<unsigned int>(mv) >= MonitorVars.size()) {
        printf("Invalid MV: %d in GetMV\n", mv);
        ss << -1;
    }
    // Otherwise, good to go on deleting MV
    else {
        MonitorVariable *monVar = MonitorVars.at(mv);
        MonitorVars.at(mv) = NULL;
        delete monVar;
        ss << mv;
    }
    //
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    // everyone creates the lock, but only the server that received the request
    // from the client sends a message back to the client
    // this is to preserve global data alignment
    if (currentFS == postOffice->getMachineID()) {
        sendMessageToClient(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    }

    //
    MVLock->Release();
}

//----------------------------------------------------------------------
//
//	Server implementation starts
//	
//----------------------------------------------------------------------
void ServerFromClient() {

	// initialized global data
	SLock = new Lock("ServerLock");
    MVLock = new Lock("MVLock");
    CVLock = new Lock("CVLock");

    sortedQueue = new List();
    lastTimeStampReceived = new int64_t[NumServers];

	// instantiate Network Data
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char * buffer = new char[MaxMailSize];
//    char * newbuf = new char[MaxMailSize];
    std::string newbuf;
    std::stringstream ss;

    while (true) {
    	// Receive the next message
DEBUG('o', "About to receive client's request\n");
    	postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
DEBUG('o', "Server %d received message from client %d mailbox %d\n", postOffice->getMachineID(), inPktHdr.from, inMailHdr.from);
    	fflush(stdout);

        // Tack on clientmachine# and clientmailbox# to buffer
        ss.str("");
        ss.clear();
        ss << inPktHdr.from;
        ss << " ";
        ss << inMailHdr.from;
        ss << " ";
        ss << buffer;

        newbuf = ss.str();
        std::copy(newbuf.begin(), newbuf.end(), buffer);
        buffer[newbuf.size()] = '\0';

        if ((inMailHdr.from / 10) > 0) {
            inPktHdr.length += 5;
            inMailHdr.length += 5;
        }
        else {
            inPktHdr.length += 4;
            inMailHdr.length += 4;
        }

        //forward the message from clients to the other servers
        for(int i = 0; i < NumServers; i++) {
            inPktHdr.to = i;
            inMailHdr.to = 1; //mailbox number is 1
            inMailHdr.from = 0; 
DEBUG('o', "Server %d is forwarding message to server %d\n", postOffice->getMachineID(), i);
            postOffice->Send(inPktHdr, inMailHdr, buffer);
        }


    }

    delete SLock;
    delete MVLock;
    delete CVLock;

    delete [] buffer;

    // Then we're done!
    interrupt->Halt();
}

void ServerFromServer() {
    // Initialize variables
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
//        char buffer[MaxMailSize];
    char * buffer = new char[MaxMailSize];
    std::stringstream ss;

    int64_t timestamp;
    int64_t smallestTS;

    // headers for server OK's (ACK's)
    PacketHeader ophOK, iphOK;
    MailHeader omhOK, imhOK;
    char * bufOK = new char[MaxMailSize];

    // data for processing request (prepended with "current" in name)
    int64_t currentTS; // current timestamp
    char * currentmsg; // current message

    int type = -1;
    std::string name;
    int index1;
    int index2;
    int index3;


    while (true) {
        // Receive a server forwarded message
DEBUG('o', "About to receive server's forwarded request\n");
        postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
        fflush(stdout);
/*printf("inPktHdr.to = %i\n", inPktHdr.to);
printf("inPktHdr.from = %i\n", inPktHdr.from);
printf("inMailHdr.to = %i\n", inMailHdr.to);
printf("inMailHdr.from = %i\n", inMailHdr.from);*/
DEBUG('o', "Server %d received forwarded message from server %d\n", postOffice->getMachineID(), inPktHdr.from);


        // Extract the timestamp and forwarding server machine ID
        ss.str(""); // clear stringstream
        ss.clear(); // for reuse
        ss << buffer;

        ss >> inPktHdr.from;
        ss >> inMailHdr.from;

        ss >> timestamp;
        forwardedServer = timestamp % 10;


        // Put the request message in the pending message queue in sorted order.
        sortedQueue->SortedInsert((void*) buffer, timestamp);


        // Update the last timestamp received table with the timestamp of the just received message 
        // for the forwarding server's machine ID that sent it.
        lastTimeStampReceived[forwardedServer] = timestamp;


        // Scan the Last Timestamp Received table 
        // and extract the smallest timestamp from any server.
        smallestTS = lastTimeStampReceived[0];
        for (int i=1; i < NumServers; i++) {
            if (smallestTS < lastTimeStampReceived[i]) {
                smallestTS = lastTimeStampReceived[i];
            }
        }


        // Retrieve the first message from pending msg queue
        currentmsg = (char*) sortedQueue->SortedRemove(&smallestTS);
        ss.str(""); // clear stringstream
        ss.clear(); // for reuse
        ss << currentmsg;
        ss >> inPktHdr.from;
        ss >> inMailHdr.from;
        ss >> currentTS;
        currentFS = currentTS % 10;


        // Process any message in out pending message queue 
        // having a timestamp less than or equal to the step 5 value (in timestamp order).
        while (currentTS <= smallestTS) {
            if (postOffice->getMachineID() == currentFS) {
                // if the request was from this server,
                // wait for all other servers to send OK
                for (int i=1; i < NumServers; i++) {
DEBUG('o', "About to receive server's OK acknowledgement\n");
                    postOffice->Receive(2, &iphOK, &imhOK, bufOK);
                    fflush(stdout);
                }
            }

            else {
                // if the request was from a different server,
                // send OK to that server
                iphOK.to = currentFS;
                imhOK.to = 2; // mailbox #2
                imhOK.from = 2;
                postOffice->Send(iphOK, imhOK, bufOK);

            }

            // process the request and respond to the client
            ss >> type;
            switch (type) {
                case CreateLock_SF : 
                    ss >> name;
                    CreateLock(inPktHdr, inMailHdr, name);
                    break;
                case DestroyLock_SF : 
                    ss >> index1;
                    DestroyLock(inPktHdr, inMailHdr, index1);
                    break;
                case CreateCV_SF : 
                    ss >> name;
                    CreateCV(inPktHdr, inMailHdr, name);
                    break;
                case DestroyCV_SF : 
                    ss >> index1;
                    DestroyCV(inPktHdr, inMailHdr, index1);
                    break;
                case Acquire_SF : 
                    ss >> index1;
                    Acquire(inPktHdr, inMailHdr, index1);
                    break;
                case Release_SF : 
                    ss >> index1;
                    Release(inPktHdr, inMailHdr, index1);
                    break;
                case Wait_SF : 
                    ss >> index1;
                    ss >> index2;
                    Wait(inPktHdr, inMailHdr, index1, index2);
                    break;
                case Signal_SF : 
                    ss >> index1;
                    ss >> index2;
                    Signal(inPktHdr, inMailHdr, index1, index2);
                    break;
                case BroadCast_SF : 
                    ss >> index1;
                    ss >> index2;
                    BroadCast(inPktHdr, inMailHdr, index1, index2);
                    break;
                case CreateMV_SF : 
                    ss >> name;
                    ss >> index1;
                    ss >> index2;
                    CreateMV(inPktHdr, inMailHdr, index2, name);
                    break;
                case GetMV_SF :
                    ss >> index1;
                    ss >> index2;
                    GetMV(inPktHdr, inMailHdr, index1, index2);
                    break;
                case SetMV_SF : 
                    ss >> index1;
                    ss >> index2;
                    ss >> index3;
                    SetMV(inPktHdr, inMailHdr, index1, index2, index3);
                    break;
                case DestroyMV_SF :
                    ss >> index1;
                    DestroyMV(inPktHdr, inMailHdr, index1);
                    break;
            }
            // Retrieve next message from pending message queue
            currentmsg = (char*) sortedQueue->SortedRemove(&smallestTS);
            ss.str(""); // clear stringstream
            ss.clear(); // for reuse
            if (currentmsg == NULL) {
                break;
            }
            ss << currentmsg;
            ss >> inPktHdr.from;
            ss >> inMailHdr.from;
            ss >> currentTS;
            currentFS = currentTS % 10;

        }
        // Prepend last message retrieved from pending message back into queue
        // This message isn't processed, since it's the last one you got out of the while loop
        if (currentmsg != NULL) {
            sortedQueue->SortedInsert((void*) currentmsg, currentTS);
        }

    }

}

#endif // NETWORK
/* end Server */

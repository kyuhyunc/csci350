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

Lock* SLock;
Lock* MVLock;
Lock* CVLock;

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
    int clientCounter;
};

class ServerCV {
    
public:
    ServerCV(int s, int o, std::string n) {
        state = s;
        owner = o;
        name = n;
        toBeDeleted = false;
        waitQ = new List();
    }
public:
    int state;
    int owner;
    std::string name;
    List * waitQ;
    bool toBeDeleted;
    ServerLock * waitingLock;
    int CVCounter;
};

class MonitorVariable {
public:
    MonitorVariable(const int &vectSize, const std::string &n) 
    :   vector(vectSize, 0),
        name(n) 
    {}
    int size() {
        return vector.size();
    }
    int& at(const int &index) {
        return vector.at(index);
    }
public: 
    std::string name;
    std::vector<int> vector;
};


std::vector<ServerLock*> ServerLockVector;
std::vector<ServerCV*> ServerCVVector;
std::vector< MonitorVariable* > MonitorVars;

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

void sendMessage(
		const PacketHeader &inPktHdr, 
		PacketHeader &outPktHdr, 
    	const MailHeader &inMailHdr, 
    	MailHeader &outMailHdr, 
    	const std::string msg
) {
    char *data = new char[msg.length()];
    std::strcpy(data, msg.c_str());

    initializeNetworkMessageHeaders(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(data));
    if(!postOffice->Send(outPktHdr, outMailHdr, data)) {
        printf("Something bad happens in Server. Unable to send message \n");
    }
    /*delete[] data;*/
}

void CreateLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name) {

    SLock->Acquire();
    //iterating through serverlockVector to check lock is already in vector
    //if other program already create lock with the same name, don't create new lock
    //just return(send the message) the index to user(Client)
    int index = -1;
    for(unsigned int i = 0; i < ServerLockVector.size(); i++) {
        if (ServerLockVector[i] != NULL) {
	        if(!ServerLockVector[i]->toBeDeleted) {
	            if(ServerLockVector[i]->name.compare(name) == 0) {
		            index = i;
                    ServerLockVector[i]->clientCounter++;   
                    std::cout << "    ****   FOUND AN OLD LOCK " << index << std::endl;
		            break;
	            } else {
                    std::cout << "    ****    names don't match!" << std::endl;
                }
	        } else {
                std::cout << "    ****    isToBeDeleted!" << std::endl;
            }
	    } else {
            std::cout << "    ****    it's null!" << std::endl;
        }
    }
    if(index == -1) {
        std::cout << "    ****   HAVE TO MAKE A NEW LOCK " << ServerLockVector.size() << std::endl;
        index = ServerLockVector.size();

        ServerLock * sLock = new ServerLock(AVAIL, inPktHdr.from, name);
        sLock->waitQ = new List();
        clientCounter = 1;
        ServerLockVector.push_back(sLock);
    }
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    ss << index;

    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());

    DEBUG('o', "Server is returning a lock index of %d\n", index);

    SLock->Release();

}
void DestroyLock(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    //if no client currently acquire the lock, server simply delete the lock
    //otherwise, set boolean toBeDeleted true so it can be deleted when the lock is done using.
    SLock->Acquire();
    
    DEBUG('o', "Server is destroying a lock\n");

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
        ServerLockVector[index]->clientCounter--;
        if(ServerLockVector[index]->state == AVAIL && ServerLockVector[index]->clientCounter == 0) {
        	DEBUG('o', "SERVER is deleting lock %d\n", index);
        	ServerLock * sl = ServerLockVector[index];
            ServerLockVector[index] = NULL; /* TODO - this probably isn't necessary */
            delete sl;
        } 
    } 
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    SLock->Release();
}
void CreateCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const std::string &name) {
    CVLock->Acquire();
    //iterating through serverCVVector to check CV is already in vector
    //if other program already create CV with the same name, don't create new CV
    //just return(send the message) the index to user(Client)
    int index = -1;
    for(unsigned int i = 0; i < ServerCVVector.size(); i++) {
        if(ServerCVVector[i] != NULL) {
            if(ServerCVVector[i]->name == name) {
            	index = i;   
            break;
            }
        }
    }

    if(index == -1) {
        index = ServerCVVector.size();
        ServerCV * sCV = new ServerCV(AVAIL, inPktHdr.from, name);
        sCV->waitQ = new List();
        sCV->CVCounter = 0;
        ServerCVVector.push_back(sCV);
    }

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    ss << index;

    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());

    DEBUG('o', "Server is returning a CV index of %d\n", index);

    CVLock->Release();
}
void DestroyCV(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    //if no client currently acquire the lock, server simply delete the lock
    //otherwise, set boolean toBeDeleted true so it can be deleted when the lock is done using.
    CVLock->Acquire();
    
    DEBUG('o', "Server is destroying a CV\n");

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    std::stringstream ss;
    if(index < 0 || index >= ServerCVVector.size()) {
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
        if(ServerCVVector[index]->state == AVAIL) {
            ServerCV* sCV = ServerCVVector[index];
            ServerCVVector[index] = NULL;
            delete sCV;
        }else{
            ServerCVVector[index]->toBeDeleted = true;
        }
    } 
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());

    CVLock->Release();
}
void Acquire(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    DEBUG('o', "Inside SERVERs Acquire!\n");
    SLock->Acquire();
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock is null or in the vector. Issue error message if client can't properly acquire the lock
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
        	DEBUG('o', "Lock->Acquire -- Lock is busy, %d is going on waitQ\n", inPktHdr.from);
            ServerLockVector[index]->waitQ->Append((void*)inPktHdr.from);
            SLock->Release();
            return;
        } else { // lock is available! "Acquire" this lock
        	DEBUG('o', "Lock->Acquire -- Lock is avaiable!, %d acquired lock\n", inPktHdr.from);
        	ServerLockVector[index]->state = BUSY;
        }
    } 
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    SLock->Release();
    
}

void ReleaseFromWaitQ(	const PacketHeader &inPktHdr, 
						PacketHeader &outPktHdr, 
    					const MailHeader &inMailHdr, 
						MailHeader &outMailHdr, 
    					const int &lockIndex
) {

	if(!ServerLockVector[lockIndex]->waitQ->IsEmpty()) {
    	int nextClient = (int)ServerLockVector[lockIndex]->waitQ->Remove();
        DEBUG('o', "Lock->Release -- Giving lock %d to thread %d in waitQueue\n", lockIndex, nextClient);
        std::stringstream ss;
        ss << lockIndex;

        PacketHeader waitPktHdr;
	    waitPktHdr.to = inPktHdr.to;
	    waitPktHdr.from = nextClient;

	    MailHeader waitMailHdr;
	    waitMailHdr.to = inMailHdr.to;
	    waitMailHdr.from = nextClient;

	    sendMessage(waitPktHdr, outPktHdr, waitMailHdr, outMailHdr, ss.str());
    } else {
        ServerLockVector[lockIndex]->state = AVAIL;
    }

	
}
void Release(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &index) {
    SLock->Acquire();

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if invalid pointer is passed and Lock is NULL
    //if so, server needs to send -1 to client so client know there's something wrong 
    if(index < 0 || index >= ServerLockVector.size()) {
        printf("Invalid index is passed in. Can't Acquire Lock.(Acquire)\n.");
        ss << -1;

    }else if(ServerLockVector[index] == NULL) {
        printf("Lock you try to release is already deleted. Can't Acquire Lock(Release)\n ");
        ss << -1;
    }else {
        printf("    ****    1\n");
        if(!ServerLockVector[index]->waitQ->IsEmpty()) {
        	printf("    ****    2\n");
            ReleaseFromWaitQ(inPktHdr, outPktHdr, inMailHdr, outMailHdr, index);
        } else if (ServerLockVector[index]->clientCounter == 0) {
        	DEBUG('o', "Lock->Release() -- deleted isToBeDeleted lock\n");
        	ServerLock* sl = ServerLockVector[index];
        	ServerLockVector[index] = NULL;
        	delete sl;
        } else {
        	printf("    ****    3, %d\n", inPktHdr.from);
        	DEBUG('o', "Lock->Release -- Lock is now available\n");
            ServerLockVector[index]->state = AVAIL;
        }
        ss << index;
    }
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str());
    SLock->Release();
    
}
void Wait(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex) {
	CVLock->Acquire();

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock/CV is null or in the vector. Issue error message if client can't properly wait
    if(LockIndex < 0 || LockIndex >= ServerLockVector.size()) {
        printf("Invalid Lock index is passed in. Can't process wait.(Wait)\n.");
        ss << -1;

    }else if(CVIndex < 0 || CVIndex >= ServerCVVector.size()) {
        printf("Invalid CV index is passed in. Can't process wait.(Wait)\n.");
        ss << -1;

    }else if(ServerLockVector[LockIndex] == NULL) {
        printf("Lock you try to wait is already deleted. Can't process wait.(Wait)\n ");
        ss << -1;  
    }else if(ServerCVVector[CVIndex] == NULL) {
        printf("Lock you try to wait is already deleted. Can't process wait.(Wait)\n ");
        ss << -1;  
    }else {
        // If this is the first time Wait is being called with this lock, then set the lock variable
        if(ServerCVVector[CVIndex]->waitingLock == NULL) {
            ServerCVVector[CVIndex]->waitingLock = ServerLockVector[LockIndex];
        }
        // If this is not the first thread to call wait, AND they passed in the incorrect lock, send error
        if(ServerCVVector[CVIndex]->waitingLock != ServerLockVector[LockIndex]) {
            ss << -1;
        }else{
        	// everything is good to go! Thread will wait until Signal is called
            ReleaseFromWaitQ(inPktHdr, outPktHdr, inMailHdr, outMailHdr, LockIndex);
            ServerCVVector[CVIndex]->waitQ->Append((void*)inPktHdr.from);
            ServerCVVector[CVIndex]->CVCounter++;
            CVLock->Release();
            return; // Don't send a message
        }
    }
	sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); // send error message
    CVLock->Release();
}
void Signal(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex) {
    CVLock->Acquire();

    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    std::stringstream ss;
    //check if the lock/CV is null or in the vector. Issue error message if client can't properly signal
    if(LockIndex < 0 || LockIndex >= ServerLockVector.size()) {
        printf("Invalid Lock index is passed in. Can't process Signal.(Signal)\n.");
        ss << -1;

    }else if(CVIndex < 0 || CVIndex >= ServerCVVector.size()) {
        printf("Invalid CV index is passed in. Can't process Signal.(Signal)\n.");
        ss << -1;

    }else if(ServerLockVector[LockIndex] == NULL) {
        printf("Lock you try to signal is already deleted. Can't process Signal.(Signal)\n ");
        ss << -1;  
    }else if(ServerCVVector[CVIndex] == NULL) {
        printf("Lock you try to signal is already deleted. Can't process Signal.(Signal)\n ");
        ss << -1;  
    }else {
        //if there is nothing to signal, then send the error message to client.
        if(ServerCVVector[CVIndex]->waitQ->IsEmpty()) {
            printf("There is nothing to signal. Can't process Signal.(Signal)\n");
            ss << -1;
        }
        // If this is not the first thread to call wait, AND they passed in the incorrect lock, send error
        if(ServerCVVector[CVIndex]->waitingLock != ServerLockVector[LockIndex]) {
            printf("Condition Lock does not match waitingLock\n");
            ss << -1;
        }else{

            // everything is good to go! Waiting thread will be signaled
            int nextClient = (int)ServerCVVector[CVIndex]->waitQ->Remove();
            ss << nextClient;

            PacketHeader waitPktHdr;
            waitPktHdr.to = inPktHdr.to;
            waitPktHdr.from = nextClient;

            MailHeader waitMailHdr;
            waitMailHdr.to = inMailHdr.to;
            waitMailHdr.from = nextClient;

            sendMessage(waitPktHdr, outPktHdr, waitMailHdr, outMailHdr, ss.str());
            
            ServerLockVector[LockIndex]->state = AVAIL; // Release the lock

            ServerCVVector[CVIndex]->CVCounter--;

            if(ServerCVVector[CVIndex]->waitQ->IsEmpty()) {
                ServerCVVector[CVIndex]->waitingLock = NULL;
            }

            ss.str("");
            ss << CVIndex;
        }

    }
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); // send error message
    CVLock->Release();
	
}
void BroadCast(const PacketHeader &inPktHdr, const MailHeader &inMailHdr, const int &LockIndex, const int &CVIndex) {
    CVLock->Acquire();
    
    while(ServerCVVector[CVIndex]->CVCounter != 0) {
        Signal(inPktHdr, inMailHdr, LockIndex, CVIndex);
    }
	CVLock->Release();
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
    SLock->Acquire();
    std::stringstream ss;
    int index = -1;
    for(unsigned int i = 0; i < MonitorVars.size(); i++) {
        if (MonitorVars[i] != NULL) {
            if(MonitorVars[i]->name == name && MonitorVars[i]->size() == size) {
                index = i;   
                ss << i;
                break;
            }
        }
    }
    if(index == -1) {
        if (size < 1) {
            printf("Invalid monitor variable size of %d in CreateMV\n", size);
            ss << -1;
        } else {
            MonitorVars.push_back(new MonitorVariable(size, name));
            ss << MonitorVars.size() - 1;
        } 
    }
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    SLock->Release();
}

void GetMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int mv, 
    const int index) 
{
    MVLock->Acquire();
    std::stringstream ss;
    if (mv < 0 || mv >= MonitorVars.size()) {
        printf("Invalid MV: %d in GetMV, array size: %d \n", mv, MonitorVars.size());
        ss << -1;
    } else if (MonitorVars.at(mv) == NULL) {
        printf("Monitor Variable: %d is null inGetMV\n", mv);
        ss << -1;
    } else if (index < 0 || index >= MonitorVars.at(mv)->size()) {
        printf("Invalid index: %d in GetMV\n", index);
        ss << -1;
    } else {
        ss << MonitorVars.at(mv)->at(index);
    }
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    MVLock->Release();
}

void SetMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int mv, 
    const int index, 
    const int value) 
{
    MVLock->Acquire();
    //
    std::stringstream ss;
    if ( mv < 0 || mv >= MonitorVars.size() ) {
        printf("Invalid MV: %d in SetMV , array size: %d \n", mv, MonitorVars.size());
        ss << -1;
    }else if (MonitorVars.at(mv) == NULL) {
        printf("Monitor Variable: %d is null inGetMV\n", mv);
        ss << -1;
    } else if ( index < 0 || index >= MonitorVars.at(mv)->size() ) {
        printf("Invalid index: %d in SetMV\n", index);
        ss << -1;
    } else {
        MonitorVars.at(mv)->at(index) = value;
        ss << value;
    }
    //
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    //
    MVLock->Release();
}

void DestroyMV(
    const PacketHeader &inPktHdr, 
    const MailHeader &inMailHdr, 
    const int mv) 
{
    MVLock->Acquire();
    //
    std::stringstream ss;
    if (mv < 0 || mv >= MonitorVars.size()) {
        printf("Invalid MV: %d in GetMV\n", mv);
        ss << -1;
    } else {
        MonitorVariable *monVar = MonitorVars.at(mv);
        MonitorVars.at(mv) = NULL;
        delete monVar;
        ss << mv;
    }
    //
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    sendMessage(inPktHdr, outPktHdr, inMailHdr, outMailHdr, ss.str()); 
    //
    MVLock->Release();
}

//----------------------------------------------------------------------
//
//	Server implementation starts
//	
//----------------------------------------------------------------------
void Server() {

	// initialized global data
	SLock = new Lock("ServerLock");
    MVLock = new Lock("MVLock");
    CVLock = new Lock("CVLock");

	// instantiate Network Data
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    while (true) {
    	// Receive the next message
    	postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);	
    	fflush(stdout);

    	// Decode the message
    	int type = -1; 
        std::string name;
        int index1;
        int index2;
        int index3;
    	std::stringstream ss(buffer);
        ss>>type;

    	// Figure out which server function to run, switch-case statement
    	switch (type) {
    		case CreateLock_SF : 
                ss>>name;
                CreateLock(inPktHdr, inMailHdr, name);
    			break;
            case DestroyLock_SF : 
                ss>>index1;
                DestroyLock(inPktHdr, inMailHdr, index1);
                break;
            case CreateCV_SF : 
                ss>>name;
                CreateCV(inPktHdr, inMailHdr, name);
                break;
            case DestroyCV_SF : 
                ss>>index1;
                DestroyCV(inPktHdr, inMailHdr, index1);
                break;
            case Acquire_SF : 
                ss>>index1;
                Acquire(inPktHdr, inMailHdr, index1);
                break;
            case Release_SF : 
                ss>>index1;
                Release(inPktHdr, inMailHdr, index1);
                break;
            case Wait_SF : 
                ss>>index1;
                ss>>index2;
                Wait(inPktHdr, inMailHdr, index1, index2);
                break;
            case Signal_SF : 
                ss>>index1;
                ss>>index2;
                Signal(inPktHdr, inMailHdr, index1, index2);
                break;
            case BroadCast_SF : 
                ss>>index1;
                ss>>index2;
                BroadCast(inPktHdr, inMailHdr, index1, index2);
                break;
            case CreateMV_SF : 
                ss>>name;
                ss>>index1;
                ss>>index2;
                CreateMV(inPktHdr, inMailHdr, index2, name);
                break;
            case GetMV_SF :
                ss>>index1;
                ss>>index2;
                GetMV(inPktHdr, inMailHdr, index1, index2);
                break;
            case SetMV_SF : 
                ss>>index1;
                ss>>index2;
                ss>>index3;
                SetMV(inPktHdr, inMailHdr, index1, index2, index3);
                break;
            case DestroyMV_SF :
                ss>>index1;
                DestroyMV(inPktHdr, inMailHdr, index1);
                break;
    	}
    }

    delete SLock;
    delete MVLock;
    delete CVLock;

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


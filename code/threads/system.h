// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#include "synch.h"
extern Lock* memlock;		// prevent race condition b/c memory is shared among processes
#include "bitmap.h"
extern BitMap* memMap;		// keeps track of pages in pageTable

#include "table.h"			// keep track of kernel data structures
#define NumLocks	1000
#define NumCVs		1000
#define NumProcesses	10
extern Table* locktable;
extern Lock* locktablelock;
extern Table* cvtable;
extern Lock* cvtablelock;
extern Lock* processLock;
extern Table* processTable;

struct kernelLock {			// kernel data structures
Lock * lock;
	AddrSpace * adds;
	bool isToBeDeleted;
	int counter;
};
struct kernelCV {
	Condition * condition;
	AddrSpace * adds;
	bool isToBeDeleted;
	int counter;
};
struct kernelProcess {
	kernelProcess() {
		adds = currentThread->space;
		threadCount = 0;
		locks = new bool[NumLocks];
		for (int i=0; i < NumLocks; i++) {
			locks[i] = false;
		}
		cvs = new bool[NumCVs];
		for (int i=0; i < NumCVs; i++) {
			cvs[i] = false;
		}
	}
	~kernelProcess() {
		delete [] locks;
		delete [] cvs;
	}
	AddrSpace * adds;
	int threadCount;
	bool* locks;
	bool* cvs;
};


extern int currentTLB;				// TLB tracker

class IPTentry {			// for IPT
  public:
	int virtualPage;
	int physicalPage;
	bool valid;
	bool readOnly;
	bool use;
	bool dirty;
	AddrSpace * space;
};

extern IPTentry* ipt;

enum evict_alg {
    FIFO, RAND
};

int evict_type;

#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H

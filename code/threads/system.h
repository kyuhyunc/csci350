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
#include "table.h"
#define NumLocks	1000
#define NumCVs		1000
#define NumProcesses	10
#define lockCounter 0
#define CVCounter	0
extern Table* locktable;
    
    struct kernelLock {
    	Lock * lock;
    	AddrSpace * adds;
    	bool isToBeDeleted;
    	int counter;
    };

extern Table* cvtable;
	
	struct kernelCV {
    	Condition * condition;
    	AddrSpace * adds;
    	bool isToBeDeleted;
    	int counter;
    };

extern Lock* processLock;

	struct kernelProcess {
		kernelProcess() {
			adds = currentThread->space;
			threadCount = 0;
//			isToBeDeleted = false;
//			lock = new Lock("pl");
//			cv = new Condition("pcv");
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
//			delete lock;
//			delete cv;
			delete [] locks;
			delete [] cvs;
		}

		AddrSpace * adds;
		int threadCount;

//		bool isToBeDeleted;
//		Lock * lock;
//		Condition * cv;

		bool* locks;
		bool* cvs;
	};

extern Table* processTable;

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

// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

class PTentry {
  public:
    int virtualPage;
    bool valid;	// initialized to false
					// when loaded into physical memory,
						// set to true
					// when evicted,
						// set to false
    bool readOnly;
    bool use;
    bool dirty;
	int byteoffset;	// only applies to virutal pages that are in
						// executable or swap
	OpenFile* location; // location of the virtual page
							// either in 1 of 3 places:
								// 1) executable
								// 2) swap
								// 3) neither (stack space)
};

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;			// Table of openfiles


 private:
	PTentry* pageTable;

    unsigned int numPages;		// Number of pages in the virtual 
					// address space
	
	OpenFile* executable;

	friend void StartProcess(char* filename);
	friend void Fork_Syscall(int pc, unsigned int vaddr, int size);
	friend void Exec_Syscall(unsigned int vaddr, int size);
	friend void Exit_Syscall(int status);
	friend void PFEhandle(unsigned int badvaddr);
	friend int IPTMissHandle(int vpn);
	friend int MemFullHandle(int vpn);

	void Dump(); // debugging
	int* AddStack(); // called by Fork_Syscall
					// returns the stack register's address
					// returns the last stack virtual page number
					// prevents race condition of multiple forks
};

#endif // ADDRSPACE_H

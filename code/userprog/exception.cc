// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
	// Copy len bytes from the current thread's virtual address vaddr.
	// Return the number of bytes so read, or -1 if an error occors.
	// Errors can generally mean a bad virtual address was passed in.
	bool result;
	int n=0;      // The number of bytes copied in
	int *paddr = new int;

	while ( n >= 0 && n < len) {
		result = machine->ReadMem( vaddr, 1, paddr );
		while(!result) // FALL 09 CHANGES
		{
			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
		} 

		buf[n++] = *paddr;

		if ( !result ) {
			//translation failed
			return -1;
		}

		vaddr++;
	}

	delete paddr;
	return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
	// Copy len bytes to the current thread's virtual address vaddr.
	// Return the number of bytes so written, or -1 if an error
	// occors.  Errors can generally mean a bad virtual address was
	// passed in.
	bool result;
	int n=0;      // The number of bytes copied in

	while ( n >= 0 && n < len) {
		// Note that we check every byte's address
		result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

		if ( !result ) {
			//translation failed
			return -1;
		}

		vaddr++;
	}

	return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
	// Create the file with the name in the user buffer pointed to by
	// vaddr.  The file name is at most MAXFILENAME chars long.  No
	// way to return errors, though...
	char *buf = new char[len+1];  // Kernel buffer to put the name in

	if (!buf) return;

	if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to Create\n");
		delete buf;
		return;
	}

	buf[len]='\0';

	fileSystem->Create(buf,0);
	delete[] buf;
	return;
}

int Open_Syscall(unsigned int vaddr, int len) {
	// Open the file with the name in the user buffer pointed to by
	// vaddr.  The file name is at most MAXFILENAME chars long.  If
	// the file is opened successfully, it is put in the address
	// space's file table and an id returned that can find the file
	// later.  If there are any errors, -1 is returned.
	char *buf = new char[len+1];  // Kernel buffer to put the name in
	OpenFile *f;      // The new open file
	int id;       // The openfile id

	if (!buf) {
		printf("%s","Can't allocate kernel buffer in Open\n");
		return -1;
	}

	if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to Open\n");
		delete[] buf;
		return -1;
	}

	buf[len]='\0';

	f = fileSystem->Open(buf);
	delete[] buf;

	if ( f ) {
		if ((id = currentThread->space->fileTable.Put(f)) == -1 )
			delete f;
		return id;
	}
	else
		return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
	// Write the buffer to the given disk file.  If ConsoleOutput is
	// the fileID, data goes to the synchronized console instead.  If
	// a Write arrives for the synchronized Console, and no such
	// console exists, create one. For disk files, the file is looked
	// up in the current address space's open file table and used as
	// the target of the write.

	char *buf;    // Kernel buffer for output
	OpenFile *f;  // Open file for output

	if ( id == ConsoleInput) return;

	if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
	} else {
		if ( copyin(vaddr,len,buf) == -1 ) {
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] buf;
			return;
		}
	}

	if ( id == ConsoleOutput) {
		for (int ii=0; ii<len; ii++) {
			printf("%c",buf[ii]);
	}

	} else {
		if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
			f->Write(buf, len);
		} else {
			printf("%s","Bad OpenFileId passed to Write\n");
			len = -1;
		}
	}

	delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
	// Write the buffer to the given disk file.  If ConsoleOutput is
	// the fileID, data goes to the synchronized console instead.  If
	// a Write arrives for the synchronized Console, and no such
	// console exists, create one.    We reuse len as the number of bytes
	// read, which is an unnessecary savings of space.
	char *buf;    // Kernel buffer for input
	OpenFile *f;  // Open file for output

	if ( id == ConsoleOutput) return -1;

	if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer in Read\n");
		return -1;
	}

	if ( id == ConsoleInput) {
		//Reading from the keyboard
		scanf("%s", buf);

		if ( copyout(vaddr, len, buf) == -1 ) {
			printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	} else {
		if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
			len = f->Read(buf, len);
			if ( len > 0 ) {
				//Read something from the file. Put into user's address space
				if ( copyout(vaddr, len, buf) == -1 ) {
					printf("%s","Bad pointer passed to Read: data not copied\n");
				}
			}
		} else {
			printf("%s","Bad OpenFileId passed to Read\n");
			len = -1;
		}
	}

	delete[] buf;
	return len;
}

void Close_Syscall(int fd) {
	// Close the file associated with id fd.  No error reporting.
	OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

	if ( f ) {
		delete f;
	} else {
		printf("%s","Tried to close an unopen file\n");
	}
}

void kernel_fork(int pc) {
	currentThread->space->InitRegisters();
	machine->WriteRegister(PCReg, pc);
	machine->WriteRegister(NextPCReg, pc+4);
	machine->WriteRegister(StackReg, currentThread->stackreg);

	currentThread->space->RestoreState();

	machine->Run();
}

void Fork_Syscall(int pc, unsigned int vaddr, int len) {
//printf("In Fork\n");
	// The Fork Syscall takes in a user program's function pointer,
	// creates a new Thread, allocates 8 pages in physical memory
	// for the Thread's stack.

	// Read in Fork name
	char* buf;
	if (!(buf = new char[len])) {
		printf("Error allocating kernel buffer for Fork!\n");
		return;
	}
	else {
		if (copyin(vaddr, len, buf) == -1) {
			printf("Bad pointer passed to write: data not written\n");
			delete [] buf;
			return;
		}
	}

	// *** update process table for multiprogramming part
	// increment current process's threadCount
	processLock->Acquire();
	int PID = -1;
	kernelProcess* kp;
	for (int i=0; i < NumProcesses; i++) {
		kp = (kernelProcess*) processTable->Get(i);
		if (kp == NULL) {
			continue;
		}
		if (kp->adds == currentThread->space) {
			PID = i;
			break;
		}
	}
	if (PID == -1) {
		printf("Error: invalid process identifier (Fork_Syscall)\n");
		processLock->Release();
		return;
	}
	kp->threadCount++;
	processLock->Release();

	// Verifies that the user program passes in a non-NULL pointer
	if (pc == 0) {
		printf("Error: cannot pass in NULL pointer to Fork\n");
	}

	// Creates new Thread
	Thread* t = new Thread(buf);
	// Allocates 8 pages to Thread's stack in physical memory
	int* stackdata = currentThread->space->AddStack();
	t->stackreg = stackdata[0];
	t->stackVP = stackdata[1];
	delete [] stackdata;
	//  t->stackreg = currentThread->space->AddStack(); // calls AddrSpace's private function as a friend
	// Thread of same process shares address space
	t->space = currentThread->space; // all threads of same process has same AddrSpace
	//  t->threadtype = FORK;
	t->Fork((VoidFunctionPtr)kernel_fork, pc);

	delete [] buf;
}

void kernel_exec(int pc) {
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
}

void Exec_Syscall(unsigned int vaddr, int len) {
//printf("In Exec\n");

	// read in char*
	char* filename;
	if (!(filename = new char[len])) {
		printf("Error allocating kernel buffer for Exec!\n");
		return;
	}
	else {
		if (copyin(vaddr, len, filename) == -1) {
			printf("Bad pointer passed to write: data not written\n");
			delete [] filename;
			return;
		}
	}

	// read in file
	OpenFile* executable = fileSystem->Open(filename);
	AddrSpace* space;

	if (executable == NULL) {
		printf("Unable to open file %s\n", filename);
		return;
	}

	// make new addrspace and new thread and fork
	Thread* t = new Thread(filename);
	memlock->Acquire();
		space = new AddrSpace(executable);
		t->stackVP = space->numPages - 1;
	memlock->Release();
	t->space = space;
	//  t->threadtype = MAIN;
	//  t->stackreg = currentThread->space->AddStack();

	// add process to processTable
	kernelProcess* kp = new kernelProcess();
	processLock->Acquire();
		kp->adds = space;
		kp->threadCount++;
	processLock->Release();
	int index = processTable->Put((void*)kp);
	if (index == -1) { // if no space in processtable
		printf("No more room in the processtable for %s\n", filename);
		return;
	}

	t->Fork((VoidFunctionPtr)kernel_exec, 0);

//	delete executable;
	delete [] filename;
}

void Exit_Syscall(int status) {
printf("Exit Value = %d\n", status);
	processLock->Acquire();

	// check if this is the last process
	bool lastProcess = false;
	if (processTable->NumUsed() == 1) {
		lastProcess = true;
	}

	// find the current process
	int PID = -1;
	kernelProcess* kp;
	for (int i=0; i < NumProcesses; i++) {
		kp = (kernelProcess*) processTable->Get(i);
		if (kp == NULL) {
			continue;
		}
		if (kp->adds == currentThread->space) {
			PID = i;
			break;
		}
	}
	if (PID == -1) {
		printf("Error: invalid process identifier (Fork_Syscall)\n");
		processLock->Release();
		return;
	}

	/*  Case 1: last executing thread in last process
	completely stop nachos
	interrupt->Halt();
	*/
	if (lastProcess && kp->threadCount == 1) {
		DEBUG('b', "last process and last thread\n");
		// reclaim all pages
		memlock->Acquire();
			DEBUG('b', "stackVP = %d\n", currentThread->stackVP);
			for (int vpn=0; vpn < currentThread->space->numPages; vpn++) {
				if (currentThread->space->pageTable[vpn].valid) {
					int ppn = -1;
					for (int j=0; j < NumPhysPages; j++) {
						if (ipt[j].valid
								&& ipt[j].virtualPage == vpn
									&& ipt[j].space == currentThread->space) {
							ppn = j;
							break;
						}
					}
					memMap->Clear(ppn);
					ipt[ppn].valid = FALSE;
					currentThread->space->pageTable[vpn].valid = FALSE;
				}
			}
			currentThread->space->Dump();
		memlock->Release();

		// delete executable
		delete currentThread->space->executable;

		// reclaim all locks
		DEBUG('b', "Deleted the following locks:\n");
		for (int i=0; i < NumLocks; i++) {
			if (kp->locks[i] == true) {
				kernelLock* kl = (kernelLock*) locktable->Remove(i);
				delete kl->lock;
				delete kl;
				DEBUG('b', "%d,", i);
			}
		}
		DEBUG('b', "\n");

		DEBUG('b', "Deleted the following cvs:\n");
		// reclaim all cvs
		for (int i=0; i < NumCVs; i++) {
			if (kp->cvs[i] == true) {
				kernelCV* kc = (kernelCV*) cvtable->Remove(i);
				delete kc->condition;
				delete kc;
				DEBUG('b', "%d,", i);
			}
		}
		DEBUG('b', "\n");

		// delete process
		processTable->Remove(PID);
		delete kp;
		processLock->Release();

		// delete all globally instantiated variables
		delete memlock;
		delete memMap;
		delete locktable;
		delete cvtable;
		delete processTable;
		delete processLock;

		// terminate program
		interrupt->Halt();
	}

	/*  Case 2: thread in process but not last thread
	reclaim 8 stack pages
	*/
	else if (kp->threadCount > 1) {
		// reclaim pages
		memlock->Acquire();
			DEBUG('b', "stackVP = %d\n", currentThread->stackVP);
			int vpn = currentThread->stackVP;
			for (int i=0; i < 8; i++) {
				if (currentThread->space->pageTable[vpn].valid) {
					int ppn = -1;
					for (int j=0; j < NumPhysPages; j++) {
						if (ipt[j].valid
								&& ipt[j].virtualPage == vpn
									&& ipt[j].space == currentThread->space) {
							ppn = j;
							break;
						}
					}
					memMap->Clear(ppn);
					ipt[ppn].valid = FALSE;
					currentThread->space->pageTable[vpn].valid = FALSE;
					vpn--;
				}
			}
			currentThread->space->Dump();
		memlock->Release();

		// decrement
		kp->threadCount--;

		DEBUG('b', "Done Exiting one thread\n");
	}

	/*  Case 3: last thread in process but not last process
	reclaim all memory not reclaimed
	reclaim all locks and cvs
	*/
	else if (!lastProcess && kp->threadCount == 1) {
		// reclaim pages
		DEBUG('b', "not last process and 1 thread left\n");
		memlock->Acquire();
			DEBUG('b', "stackVP = %d\n", currentThread->stackVP);
			for (int vpn=0; vpn < currentThread->space->numPages; vpn++) {
				if (currentThread->space->pageTable[vpn].valid) {
					int ppn = -1;
					for (int j=0; j < NumPhysPages; j++) {
						if (ipt[j].valid
								&& ipt[j].virtualPage == vpn
									&& ipt[j].space == currentThread->space) {
							ppn = j;
							break;
						}
					}
					memMap->Clear(ppn);
					ipt[ppn].valid = FALSE;
					currentThread->space->pageTable[vpn].valid = FALSE;
				}
			}
			currentThread->space->Dump();
		memlock->Release();

		// delete executable
		delete currentThread->space->executable;

		// reclaim all locks
		DEBUG('b', "Deleted the following locks:\n");
		for (int i=0; i < NumLocks; i++) {
			if (kp->locks[i] == true) {
				kernelLock* kl = (kernelLock*) locktable->Remove(i);
				delete kl->lock;
				delete kl;
				DEBUG('b', "%d,", i);
			}
		}
		DEBUG('b', "\n");

		DEBUG('b', "Deleted the following cvs:\n");
		// reclaim all cvs
		for (int i=0; i < NumCVs; i++) {
			if (kp->cvs[i] == true) {
				kernelCV* kc = (kernelCV*) cvtable->Remove(i);
				delete kc->condition;
				delete kc;
				DEBUG('b', "%d,", i);
			}
		}
		DEBUG('b', "\n");

		// delete process
		processTable->Remove(PID);
		delete kp;
	}

	else {
		printf("Some other case not accounted for in Exit_Syscall.. go back and debug!\n");
	}

	processLock->Release();

	currentThread->Finish();

}

void Yield_Syscall() {
	currentThread->Yield();
}

int CreateLock_Syscall(int vaddr, int size) {
	locktablelock->Acquire();

	//it returns -1 when user can't create lock for some reason.
	//otherwise, it returns index of table where the lock that user creates is located. 
	DEBUG('c',"CreateLock starts\n");

	//**********************************************************************
	//				PARSING NAME
	//**********************************************************************

	// reading in lock name
	char *buf = new char[size+1]; // Kernel buffer to put the name in
	if (!buf) {
		printf("%s","Can't allocate kernel buffer CreateLock(CreateLock)\n");
		locktablelock->Release();
		return -1;
	}
	if(copyin(vaddr, size, buf) == -1) {
		//check if the pointer is valid one. if pointer is not valid, then return.
		printf("error: Pointer is invalid(CreateLock)\n");
		locktablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	//check if the kernel lock table is full, then you can't put lock in there.
	if(locktable->NumUsed() >= NumLocks) {
		printf("ERROR: No more Locks available. Lock not created.\n");
	}

	//**********************************************************************
	//				CREATING LOCK
	//**********************************************************************

	Lock * l = new Lock(buf);
	kernelLock * kl = new kernelLock();
	kl->lock = l;
	kl->isToBeDeleted = false;
	kl->adds = currentThread->space;
	kl->counter = 0; //to check and to know when I can destroy!

	// search for available lock
	// returns -1 if there is no available lock
	int index = locktable->Put((void * )kl);

	//return when you can't put lock in the table.
	if(index == -1) {
		printf("ERROR: No more Locks available. Lock not created.\n");
		locktablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				UPDATE PROCESS TABLE
	//**********************************************************************

	// find the current process
	processLock->Acquire();
		int PID = -1;
		kernelProcess* kp;
		for (int i=0; i < NumProcesses; i++) {
			kp = (kernelProcess*) processTable->Get(i);
			if (kp == NULL) {
				continue;
			}
			if (kp->adds == currentThread->space) {
				PID = i;
				break;
			}
		}
		if (PID == -1) {
			printf("Error: invalid process identifier (CreateLock_Syscall)\n");
			processLock->Release();
			locktablelock->Release();
			return -1;
		}
		kp->locks[index] = true;
	processLock->Release();

	locktablelock->Release();
	return index;
}

int DestroyLock_Syscall(int index) {
	locktablelock->Acquire();

	// it returns -1 when lock can't be destroyed
	// otherwise, it returns index.
	//if it is ready to be destroyed, then set the boolean value true and make the lock pointer NULL
	DEBUG('c', "DestroyLock starts\n");

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	// checking index. if index is -1 then user did not properly create LOCK!
	if (index == -1) {
		printf("ERROR: Lock not created properly. Lock not destroyed.\n");
		locktablelock->Release();
		return -1;
	}
	// checking other indices to protect against garbage values
	if (index < 0 || index > NumLocks) {
		// index out of range
		printf("ERROR: Invalid index passed in. Lock not destroyed.\n");
		locktablelock->Release();
		return -1;
	}

	kernelLock* kl = (kernelLock*) locktable->Get(index);
	if (kl == NULL || kl->lock == NULL) {
		// lock does not exist. Return -1 since it can't be deleted
		printf("ERROR: Target Lock does not exist. Lock not destroyed.\n");
		locktablelock->Release();
		return -1;
	}

	// if current thread is not the thread that create lock, then it can't be destroyed.
	if (kl->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Lock belongs to a different process. Lock not destroyed.\n");
		locktablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				DESTROYING LOCK
	//**********************************************************************

	// if Lock is currently available, destroy it now
	if (kl->counter == 0) {
		kl = (kernelLock*) locktable->Remove(index);
		delete kl->lock;
		delete kl;
	}
	// if Lock is currently not released yet, destroy it later
	else {
		DEBUG('c',"Destory Lock call %d\n", ((kernelLock * )locktable->Get(index))->isToBeDeleted);
		DEBUG('c',"Lock index in DestoryLock   : %d \n", index);

		kl->isToBeDeleted = true;

		//**********************************************************************
		//				UPDATE PROCESS TABLE
		//**********************************************************************

		// find the current process
		processLock->Acquire();
			int PID = -1;
			kernelProcess* kp;
			for (int i=0; i < NumProcesses; i++) {
				kp = (kernelProcess*) processTable->Get(i);
				if (kp == NULL) {
					continue;
				}
				if (kp->adds == currentThread->space) {
					PID = i;
					break;
				}
			}
			if (PID == -1) {
				printf("Error: invalid process identifier (DestroyLock_Syscall)\n");
				processLock->Release();
				locktablelock->Release();
				return -1;
			}
			kp->locks[index] = false;
		processLock->Release();
	}

	locktablelock->Release();
	return index;
}

int Acquire_Syscall(int index) {
	locktablelock->Acquire();

	DEBUG('c', "%d in Acquire\n", index);
	//if there is error, return -1
	//otherwise, it returns lock index that user tries to acquire

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	// checking index. if index is -1 then user did not properly create LOCK!
	if (index == -1) {
		printf("ERROR: Lock not created properly. Acquire not called.\n");
		locktablelock->Release();
		return -1;
	}
	// checking other indices to protect against garbage values
	if (index < 0 || index > NumLocks) {
		// index out of range
		printf("ERROR: Invalid index passed in. Acquire not called.\n");
		locktablelock->Release();
		return -1;
	}

	kernelLock* kl = (kernelLock*) locktable->Get(index);
	if (kl == NULL || kl->lock == NULL) {
		// lock does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Lock does not exist. Acquire not called.\n");
		locktablelock->Release();
		return -1;
	}

	// if current thread is not the thread that create lock, then it can't be acquired
	if (kl->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Lock belongs to a different process. Acquire not called.\n");
		locktablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				ACQUIRING LOCK
	//**********************************************************************

	kl->counter++; // keeps track of how many threads attempt to acquire this lock
					// so it doesn't get destroyed and put correct user programs into deadlock!
	kl->lock->Acquire();

	locktablelock->Release();
	return index;
}

int Release_Syscall(int index) {
	locktablelock->Acquire();

	//if there is error, return -1
	//otherwise, it returns lock index that user tries to Release
	DEBUG('c', "%d in Release\n", index);

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	// checking index. if index is -1 then user did not properly create LOCK!
	if (index == -1) {
		printf("ERROR: Lock not created properly. Release not called.\n");
		locktablelock->Release();
		return -1;
	}
	// checking other indices to protect against garbage values
	if (index < 0 || index > NumLocks) {
		// index out of range
		printf("ERROR: Invalid index passed in. Release not called.\n");
		locktablelock->Release();
		return -1;
	}

	kernelLock* kl = (kernelLock*) locktable->Get(index);
	if (kl == NULL || kl->lock == NULL) {
		// lock does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Lock does not exist. Release not called.\n");
		locktablelock->Release();
		return -1;
	}

	// if current thread is not the thread that create lock, then it can't be acquired
	if (kl->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Lock belongs to a different process. Release not called.\n");
		locktablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				RELEASING LOCK
	//**********************************************************************

	kl->lock->Release();
	kl->counter--;

	// if DestroyLock was supposed to destroy this lock but wasn't able to,
	// check if nobody is trying to Acquire the lock, and destroy it if 
	// that condition is met!
	if (kl->isToBeDeleted == true && kl->counter == 0) {
		DEBUG('c', "Lock index in release  : %d \n", index);
		DEBUG('c', "Lock Counter : %d \n",((kernelLock * )locktable->Get(index))->counter);
		DEBUG('c', "Lock Boolean : %d \n",((kernelLock * )locktable->Get(index))->isToBeDeleted);
		DEBUG('c', "Lock is deleted\n");
		kl = (kernelLock*) locktable->Remove(index);
		delete kl->lock;
		delete kl;

		//**********************************************************************
		//				UPDATE PROCESS TABLE
		//**********************************************************************

		// find the current process
		processLock->Acquire();
			int PID = -1;
			kernelProcess* kp;
			for (int i=0; i < NumProcesses; i++) {
				kp = (kernelProcess*) processTable->Get(i);
				if (kp == NULL) {
					continue;
				}
				if (kp->adds == currentThread->space) {
					PID = i;
					break;
				}
			}
			if (PID == -1) {
				printf("Error: invalid process identifier (ReleaseLock_Syscall)\n");
				processLock->Release();
				locktablelock->Release();
				return -1;
			}
			kp->locks[index] = false;
		processLock->Release();
	}

	locktablelock->Release();
	return index;
}

int CreateCV_Syscall(int vaddr, int size) {
	cvtablelock->Acquire();

	//it returns -1 when user can't create CV for some reason.
	//otherwise, it returns index of table where the cv that user creates is located. 
	DEBUG('c', "createCV starts\n");
	
	//**********************************************************************
	//				PARSING NAME
	//**********************************************************************

	char *buf = new char[size+1]; // Kernel buffer to put the name in
	if (!buf) {
		printf("%s","Can't allocate kernel buffer CreateCV.(CreateCV)\n");
		cvtablelock->Release();
		return -1;
	}
	if(copyin(vaddr, size, buf) == -1) {
		//check if the pointer is valid one. if pointer is not valid, then return.
		printf("error: Pointer is invalid.(CreateCV)\n");
		cvtablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	//check if the kernel lock table is full, then you can't put lock in there.
	if(cvtable->NumUsed() >= NumCVs) {
		printf("ERROR: No more Conditions available. Condition not created.\n");
		cvtablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				CREATING CONDITION
	//**********************************************************************

	Condition * cv = new Condition(buf);
	kernelCV * kc = new kernelCV();
	kc->condition = cv;
	kc->isToBeDeleted = false;
	kc->adds = currentThread->space;
	//to check and to know when I can destroy!
	kc->counter = 0;

	int index = cvtable->Put((void * )kc);
	//return when you can't put lock in the table.
	if(index == -1) {
		printf("ERROR: No more Conditions available. Condition not created.\n");
		cvtablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				UPDATE PROCESS TABLE
	//**********************************************************************

	// find the current process
	processLock->Acquire();
		int PID = -1;
		kernelProcess* kp;
		for (int i=0; i < NumProcesses; i++) {
			kp = (kernelProcess*) processTable->Get(i);
			if (kp == NULL) {
				continue;
			}
			if (kp->adds == currentThread->space) {
				PID = i;
				break;
			}
		}
		if (PID == -1) {
			printf("Error: invalid process identifier (CreateCV_Syscall)\n");
			processLock->Release();
			cvtablelock->Release();
			return -1;
		}
		kp->cvs[index] = true;
	processLock->Release();



	cvtablelock->Release();
	return index;
}

int DestroyCV_Syscall(int index) {
	cvtablelock->Acquire();

    // it returns -1 when lock can't be destroyed
    // otherwise, it returns index.
    //if it is ready to be destroyed, then set the boolean value true and make the lock pointer NULL
    //it has to be checked whether the lock is already used or not AND the boolean(destroyed) is false
    DEBUG('c',"DestroyCV starts\n");

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

    //checking index. if index is -1 then user did not properly create CV!
    if(index == -1) {
		printf("ERROR: Condition not created properly. Condition not destroyed.\n");
		cvtablelock->Release();
        return -1;
    }
	// checking other indices to protect against garbage values
	if (index < 0 || index > NumLocks) {
		// index out of range
		printf("ERROR: Invalid index passed in. Condition not destroyed.\n");
		cvtablelock->Release();
		return -1;
	}

	kernelCV* kc = (kernelCV*) cvtable->Get(index);
	if (kc == NULL || kc->condition == NULL) {
		// lock does not exist. Return -1 since it can't be deleted
		printf("ERROR: Target Lock does not exist. Condition not destroyed.\n");
		cvtablelock->Release();
		return -1;
	}

	// if current thread is not the thread that create cv, then it can't be destroyed.
	if (kc->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Condition belongs to a different process. Condition not destroyed.\n");
		cvtablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				DESTROYING CONDITION
	//**********************************************************************

	// if Condition is currently available, destroy it now
	if (kc->counter == 0) {
		kc = (kernelCV*) cvtable->Remove(index);
		delete kc->condition;
		delete kc;


		//**********************************************************************
		//				UPDATE PROCESS TABLE
		//**********************************************************************

		// find the current process
		processLock->Acquire();
			int PID = -1;
			kernelProcess* kp;
			for (int i=0; i < NumProcesses; i++) {
				kp = (kernelProcess*) processTable->Get(i);
				if (kp == NULL) {
					continue;
				}
				if (kp->adds == currentThread->space) {
					PID = i;
					break;
				}
			}
			if (PID == -1) {
				printf("Error: invalid process identifier (DestroyCV_Syscall)\n");
				processLock->Release();
				cvtablelock->Release();
				return -1;
			}
			kp->cvs[index] = false;
		processLock->Release();

	}
	// if other threads are currently Waiting on this Condition, destroy it later
	else {
		kc->isToBeDeleted = true;

	}

	cvtablelock->Release();
    return index;
}

int Wait_Syscall(int lockIndex, int CVIndex) {
	cvtablelock->Acquire();

    //if you can't call wait properly, then it returns -1 so we know if there is something wrong.
    //Otherwise, it returns CV index number
    //first you need to check if the valid index is passed in
    DEBUG('c', "lock index %d and CVIndex %d in Wait\n", lockIndex, CVIndex);

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	// validate lock index
	if (lockIndex == -1) {
		printf("ERROR: Lock not created properly. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}
	if (lockIndex < 0 || lockIndex > NumLocks) {
		// index out of range
		printf("ERROR: Invalid Lock index passed in. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate lock
	kernelLock* kl = (kernelLock*) locktable->Get(lockIndex);
	if (kl == NULL || kl->lock == NULL) {
		// lock does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Lock does not exist. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate lock's process
	if (kl->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Lock belongs to a different process. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate cv index
	if (CVIndex == -1) {
		printf("ERROR: Condition not created properly. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}
	if (CVIndex < 0 || CVIndex > NumCVs) {
		// index out of range
		printf("ERROR: Invalid Condition index passed in. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}
	
	// validate condition
	kernelCV* kc = (kernelCV*) cvtable->Get(CVIndex);
	if (kc == NULL || kc->condition == NULL) {
		// cv does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Condition does not exist. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate condition's process
	if (kc->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Condition belongs to a different process. Wait not called.\n");
		cvtablelock->Release();
		return -1;
	}

    DEBUG('c',"before being called wait!\n");
	
	//**********************************************************************
	//				WAIT
	//**********************************************************************

	kc->counter++;
	cvtablelock->Release();
	kc->condition->Wait(kl->lock);
	cvtablelock->Acquire();
	kc->counter--;

	// if DestroyCV was supposed to destroy this lock but wasn't able to,
	// check if nobody is waiting on the Condition, and destroy it if
	// that condition is met!
	if (kc->isToBeDeleted == true && kc->counter == 0) {
		DEBUG('c', "condition is deleted\n");

		kc = (kernelCV*) cvtable->Remove(CVIndex);
		delete kc->condition;
		delete kc;

		//**********************************************************************
		//				UPDATE PROCESS TABLE
		//**********************************************************************

		// find the current process
		processLock->Acquire();
			int PID = -1;
			kernelProcess* kp;
			for (int i=0; i < NumProcesses; i++) {
				kp = (kernelProcess*) processTable->Get(i);
				if (kp == NULL) {
					continue;
				}
				if (kp->adds == currentThread->space) {
					PID = i;
					break;
				}
			}
			if (PID == -1) {
				printf("Error: invalid process identifier (Wait_Syscall)\n");
				processLock->Release();
				cvtablelock->Release();
				return -1;
			}
			kp->cvs[CVIndex] = false;
		processLock->Release();

	}

	cvtablelock->Release();
    return CVIndex;
}

int Signal_Syscall(int lockIndex, int CVIndex) {
	cvtablelock->Acquire();

      //if you can't call signal properly, then it returns -1 so we know if there is something wrong.
    //Otherwise, it returns CV index number
    DEBUG('c', "lock index %d and CVIndex %d in signal\n", lockIndex, CVIndex);

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	// validate lock index
	if (lockIndex == -1) {
		printf("ERROR: Lock not created properly. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}
	if (lockIndex < 0 || lockIndex > NumLocks) {
		// index out of range
		printf("ERROR: Invalid Lock index passed in. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate lock
	kernelLock* kl = (kernelLock*) locktable->Get(lockIndex);
	if (kl == NULL || kl->lock == NULL) {
		// lock does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Lock does not exist. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate lock's process
	if (kl->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Lock belongs to a different process. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate cv index
	if (CVIndex == -1) {
		printf("ERROR: Condition not created properly. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}
	if (CVIndex < 0 || CVIndex > NumCVs) {
		// index out of range
		printf("ERROR: Invalid Condition index passed in. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}
	
	// validate condition
	kernelCV* kc = (kernelCV*) cvtable->Get(CVIndex);
	if (kc == NULL || kc->condition == NULL) {
		// cv does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Condition does not exist. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate condition's process
	if (kc->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Condition belongs to a different process. Signal not called.\n");
		cvtablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				SIGNAL
	//**********************************************************************

	kc->condition->Signal(kl->lock);

	cvtablelock->Release();
	return CVIndex;
}

int Broadcast_Syscall(int lockIndex, int CVIndex) {
	cvtablelock->Acquire();

    //if you can't call Broadcast properly, then it returns -1 so we know if there is something wrong.
    //Otherwise, it returns CV index number
    DEBUG('c', "lock index %d and CVIndex %d in Broadcasts\n", lockIndex, CVIndex);

	//**********************************************************************
	//				ERROR CHECKING
	//**********************************************************************

	// validate lock index
	if (lockIndex == -1) {
		printf("ERROR: Lock not created properly. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}
	if (lockIndex < 0 || lockIndex > NumLocks) {
		// index out of range
		printf("ERROR: Invalid Lock index passed in. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate lock
	kernelLock* kl = (kernelLock*) locktable->Get(lockIndex);
	if (kl == NULL || kl->lock == NULL) {
		// lock does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Lock does not exist. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate lock's process
	if (kl->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Lock belongs to a different process. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate cv index
	if (CVIndex == -1) {
		printf("ERROR: Condition not created properly. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}
	if (CVIndex < 0 || CVIndex > NumCVs) {
		// index out of range
		printf("ERROR: Invalid Condition index passed in. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}
	
	// validate condition
	kernelCV* kc = (kernelCV*) cvtable->Get(CVIndex);
	if (kc == NULL || kc->condition == NULL) {
		// cv does not exist. Return -1 since it can't be acquired
		printf("ERROR: Target Condition does not exist. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}

	// validate condition's process
	if (kc->adds != currentThread->space) {
		printf("ERROR: Permission denied! Target Condition belongs to a different process. Broadcast not called.\n");
		cvtablelock->Release();
		return -1;
	}

	//**********************************************************************
	//				BROADCAST
	//**********************************************************************

	// it call signal syscall by the number of CV counter so that we can keep track of counter efficiently.
	int count = kc->counter;
	for (int i=0; i < count; i++) {
		Signal_Syscall(lockIndex, CVIndex);
	}

	cvtablelock->Release();
    return CVIndex;
}

void Printf0_Syscall(unsigned int vaddr, int len) {
	// Supposed to work similarly to a standard C printf function
	// First check to see if allocation is possible
	// Second check validity of pointer
	// Printf0 has no additional arguments and is purely a char*
	// Last, print out the statement

	char* buf;

	if (!(buf = new char[len])) {
		printf("Error allocating kernel buffer for Printf0!\n");
		return;
	}
	else {
		if (copyin(vaddr, len, buf) == -1) {
			printf("Bad pointer passed to write: data not written\n");
			delete [] buf;
			return;
		}
	}

	printf(buf);

	delete [] buf;
}

int NumbersInString(char* buffer, int size) {
	int count = 0;
	for (int i=0; i < (size-1); i++) {
		if (buffer[i] == '%' && buffer[i+1] == 'd') {
			count++;
		}
	}
	return count;
}

void Printf1_Syscall(unsigned int vaddr, int len, int num1) {
	// Supposed to work similarly to a standard C printf function
	// First check to see if allocation is possible
	// Second check validity of pointer
	// Printf1 has one additional argument and can support up to 3 numbers
	// Last, print out the statement

	char* buf;

	if (!(buf = new char[len])) {
		printf("Error allocating kernel buffer for Printf1!\n");
		return;
	}
	else {
		if (copyin(vaddr, len, buf) == -1) {
			printf("Bad pointer passed to write: data not written\n");
			delete [] buf;
			return;
		}
	}

	// Find how many numbers are in the char array

	// Printf1 can support up to 3 numbers less than 1000 decimal
	// By using decimal shifts of up to 1000
	int count = NumbersInString(buf, len);
	if (count == 3) {
		printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000));
	}
	else if (count == 2) {
		printf(buf, num1/1000, num1%1000);
	}
	else if (count == 1) {
		printf(buf, num1);
	}
	else {
		printf("Error: Printf1 only supports between 1 to 3 integer arguments\n");
	}

/*	if (num1 / 1000000 > 0) { // 3 numbers
		printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000));
	}
	else if (num1 / 1000 > 0) { // 2 numbers
		printf(buf, num1/1000, num1%1000);
	}
	else { // 1 number
		printf(buf, num1);
	}
*/
	delete [] buf;
}

void Printf2_Syscall(unsigned int vaddr, int len, int num1, int num2) {
  // Supposed to work similarly to a standard C printf function
  // First check to see if allocation is possible
  // Second check validity of pointer
  // Printf2 has two additional arguments and can support up to 6 numbers
  // Last, print out the statement
  
  char* buf;
  
  if (!(buf = new char[len])) {
    printf("Error allocating kernel buffer for Printf2!\n");
    return;
  }
  else {
    if (copyin(vaddr, len, buf) == -1) {
      printf("Bad pointer passed to write: data not written\n");
      delete [] buf;
      return;
    }
  }

  // Printf2 can support up to 6 numbers less than 1000 decimal
  // By using decimal shifts of up to 1000
  // Printf2 assumes you are using more than 3 numbers
	int count = NumbersInString(buf, len);
	if (count == 6) {
    printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000),
          num2/1000000, (num2%1000000)/1000, (num2%1000));
	}
	else if (count == 5) {
    printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000),
          num2/1000, num1%1000);
	}
	else if (count == 4) {
    printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000),
          num2);
	}
	else {
		printf("Error: Printf2 only supports between 4 to 6 integer arguments\n");
	}

/*  if (num2 / 1000000 > 0) { // 6 numbers
    printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000),
          num2/1000000, (num2%1000000)/1000, (num2%1000));
  }
  else if (num2 / 1000 > 0) { // 5 numbers
    printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000),
          num2/1000, num1%1000);
  }
  else { // 4 numbers
    printf(buf, num1/1000000, (num1%1000000)/1000, (num1%1000),
          num2);
  }
*/
  delete [] buf;
}

void DumpTLB() {
	printf("\t******TLB INFO******\n");

	for (int i=0; i < TLBSize; i++) {
		printf("\tTLB page %d: ppn = %d, vpn = %d, valid = %d, dirty = %d\n", i, machine->tlb[i].physicalPage, machine->tlb[i].virtualPage, machine->tlb[i].valid, machine->tlb[i].dirty);
	}

	printf("\t****END TLB INFO****\n");
}

void DumpIPT() {
	printf("\t\t======IPT INFO======\n");

	for (int i=0; i < NumPhysPages; i++) {
		printf("\t\tIPT page %d: ppn = %d, vpn = %d, valid = %d, dirty = %d\n", i, ipt[i].physicalPage, ipt[i].virtualPage, ipt[i].valid, ipt[i].dirty);
	}

	printf("\t\t====END IPT INFO====\n");
}

/*
	Evicts a page from memory (FIFO or RAND)
	And returns evicted page number
*/
int MemFullHandle(int vpn) {
	//******
	// TODO
	//******

    // Evict a page and save it to SWAP if necessary
	int ppn = -1;
	
	//**************
	// FIFO eviction
	//**************
	int* temp = (int*) iptFIFOqueue->Remove();
	ppn = *temp;
	delete temp;

//printf("Evicting ppn = %d, previously vpn = %d to make room for new vpn = %d\n", ppn, ipt[ppn].virtualPage, vpn);
	
	// check if ppn is in TLB
	// if it is, must propogate dirty bit and invalidate it
	for (int i=0; i < TLBSize; i++) {
		if (ppn == machine->tlb[i].physicalPage && machine->tlb[i].valid) {
			ipt[ppn].dirty = machine->tlb[i].dirty;
			machine->tlb[i].valid = FALSE;
			break;
		}
	}

	// check if bit is dirty
	// if it is, then write to SWAP file
	if (ipt[ppn].valid && ipt[ppn].dirty) {

		// find available SWAP bit
		int swapbit = swapMap->Find();
		if (swapbit == -1) {
			printf("Make SwapMap bigger\n");
			return -1;
		}


		// write to SWAP file
//printf("Writing to swap file where swap bit = %d, vpn = %d\n", swapbit, ipt[ppn].virtualPage);
		swapfile->WriteAt(
			&(machine->mainMemory[ppn*PageSize]),
			PageSize,
			swapbit*PageSize);

		// modify pageTable info
		currentThread->space->pageTable[ipt[ppn].virtualPage].byteoffset = swapbit*PageSize;
		currentThread->space->pageTable[ipt[ppn].virtualPage].type = SWAP;
		currentThread->space->pageTable[ipt[ppn].virtualPage].location = swapfile;
		
	}

	currentThread->space->pageTable[ipt[ppn].virtualPage].valid = FALSE;
	

	return ppn;
}

int IPTMissHandle(int vpn) {
//printf("\t\tBefore updating IPT\n");
//DumpIPT();
	// find available physical page number
	// (in step 3, we assume there is always space)
	int ppn = memMap->Find();

	if (ppn == -1) {
		// if physical memory is full, handle it
		// by evicting a page
		ppn = MemFullHandle(vpn);
	}

	// populate IPT
	ipt[ppn].virtualPage = vpn;
	ipt[ppn].physicalPage = ppn;
	ipt[ppn].valid = TRUE;
	ipt[ppn].use = FALSE;
	ipt[ppn].dirty = FALSE;
	ipt[ppn].readOnly = FALSE;
	ipt[ppn].space = currentThread->space;

	// add ppn to IPT FIFO queue
	int* ppntemp = new int;
	*ppntemp = ppn;
	iptFIFOqueue->Append((void*) ppntemp);

	// load physical memory from executable or swap if necessary
	if (currentThread->space->pageTable[vpn].byteoffset != -1) {
		currentThread->space->pageTable[vpn].location->ReadAt(
			&(machine->mainMemory[ppn*PageSize]),
			PageSize,
			currentThread->space->pageTable[vpn].byteoffset);
		if (currentThread->space->pageTable[vpn].type == SWAP) {
//printf("Writing from swap file where swap bit = %d, vpn = %d\n",
//							currentThread->space->pageTable[vpn].byteoffset/PageSize,
//							vpn);
			swapMap->Clear(currentThread->space->pageTable[vpn].byteoffset/PageSize);
            ipt[ppn].dirty = TRUE;
		}
	}
	// load from swap file						***********************

	
	// update pageTable's valid bit
	// (to synchronize with IPT)
	currentThread->space->pageTable[vpn].valid = TRUE;

//printf("\t\tAfter updating IPT\n");
//DumpIPT();
	return ppn;
}

void PFEhandle(unsigned int badvaddr) {
	// disable interrupts
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

//printf("\tBefore updating tlb:\n");
//DumpTLB();

	// calculate pageTable index
	int vpn = badvaddr / PageSize;

	int ppn = -1;

	// find ppn in IPT
	for (int i=0; i < NumPhysPages; i++) {
		// must meet these three conditions:
		// 1) valid bit is true
		// 2) matching virtual page number
		// 3) matching AddrSpace *
		if (ipt[i].valid
				&& ipt[i].virtualPage == vpn
					&& ipt[i].space == currentThread->space) {
			ppn = i;
			break;
		}
	}

	// if ppn is not found in IPT, it's an IPT-miss, so handle it
	// by finding an available physical page
	// and loading physical memory from proper location
	if (ppn == -1) {
		ppn = IPTMissHandle(vpn);
	}

	// propogate dirty bit
	// copy data from pageTable into TLB
	if (machine->tlb[currentTLB].valid) {
		ipt[machine->tlb[currentTLB].physicalPage].dirty = machine->tlb[currentTLB].dirty;
	}
	machine->tlb[currentTLB].virtualPage = ipt[ppn].virtualPage;
	machine->tlb[currentTLB].physicalPage = ipt[ppn].physicalPage;
	machine->tlb[currentTLB].valid = ipt[ppn].valid;
	machine->tlb[currentTLB].use = ipt[ppn].use;
	machine->tlb[currentTLB].dirty = ipt[ppn].dirty;
	machine->tlb[currentTLB].readOnly = ipt[ppn].readOnly;
//printf("\tAfter updating tlb:\n");
//DumpTLB();


	// increment TLB pointer
	currentTLB = (++currentTLB) % TLBSize;

	// restore interrupts
	(void) interrupt->SetLevel(oldLevel);
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0;   // the return value from a syscall

    if ( which == SyscallException ) {
		switch (type) {
			default:
				DEBUG('a', "Unknown syscall - shutting down.\n");
			case SC_Halt:
				DEBUG('a', "Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;
			case SC_Exit:
				DEBUG('a', "Exit Syscall.\n");
				Exit_Syscall(machine->ReadRegister(4));
				break;
			case SC_Create:
				DEBUG('a', "Create syscall.\n");
				Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_Open:
				DEBUG('a', "Open syscall.\n");
				rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_Write:
				DEBUG('a', "Write syscall.\n");
				Write_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
				break;
			case SC_Read:
				DEBUG('a', "Read syscall.\n");
				rv = Read_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
				break;
			case SC_Close:
				DEBUG('a', "Close syscall.\n");
				Close_Syscall(machine->ReadRegister(4));
				break;
			case SC_Fork:
				DEBUG('a', "Fork syscall.\n");
				Fork_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
				break;
			case SC_Yield:
				DEBUG('a', "Yield syscall.\n");
				Yield_Syscall();
				break;
			case SC_Exec:
				DEBUG('a', "Exec syscall.\n");
				Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_CreateLock:
				DEBUG('a', "CreateLock syscall.\n");
				rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_DestroyLock:
				DEBUG('a', "DestroyLock syscall.\n");
				rv = DestroyLock_Syscall(machine->ReadRegister(4));
				break;
			case SC_Acquire:
				DEBUG('a', "Acquire syscall.\n");
				rv = Acquire_Syscall(machine->ReadRegister(4));
				break;
			case SC_Release:
				DEBUG('a', "Release syscall.\n");
				rv = Release_Syscall(machine->ReadRegister(4));
				break;
			case SC_CreateCV:
				DEBUG('a', "CreateCV syscall.\n");
				rv = CreateCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_DestroyCV:
				DEBUG('a', "DestroyCV syscall.\n");
				rv = DestroyCV_Syscall(machine->ReadRegister(4));
				break;
			case SC_Wait:
				DEBUG('a', "Wait syscall.\n");
				rv = Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_Signal:
				DEBUG('a', "Signal syscall.\n");
				rv = Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_Broadcast:
				DEBUG('a', "Broadcast syscall.\n");
				rv = Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_Printf0:
				DEBUG('a', "Printf0 syscall.\n");
				Printf0_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			case SC_Printf1:
				DEBUG('a', "Printf1 syscall.\n");
				Printf1_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
				break;
			case SC_Printf2:
				DEBUG('a', "Printf2 syscall.\n");
				Printf2_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
				break;
		}

		// Put in the return value and increment the PC
		machine->WriteRegister(2,rv);
		machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
		return;
    }
	else if ( which == PageFaultException ) {
		PFEhandle(machine->ReadRegister(39));
		return;
	}
	else {
		cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
		interrupt->Halt();
    }
}

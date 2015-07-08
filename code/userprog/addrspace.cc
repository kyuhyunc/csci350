// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

/*class ProcessTable : Table {
	ProcessTable(int s) : Table(s) { }
public:
	int GetProcessIndex(AddrSpace* as);
	int NumUsed();
}


int ProcessTable::GetProcessIndex(AddrSpace* as) {
	// Finds the process index associated with the addrspace pointer
	// If no match is found, returns -1

	kernelProcess* kp;
	for (int i=0; i < size; i++) {
		kp = (kernelProcess*) Get(i);
		if (kp->adds == as) {
			return i;
		}
	}
	return -1;
}*/

int Table::NumUsed() {
	return ( size - map.NumClear() );
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
//printf("In address constructor...\n");
	this->executable = executable;

    NoffHeader noffH;
    unsigned int i, size;

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);
                                                // we need to increase the size
						// to leave room for the stack
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
	int ppn;
//	pageTable = new TranslationEntry[numPages];
	pageTable = new PTentry[numPages];
	for (i = 0; i < (numPages - 8); i++) {
//		ppn = memMap->Find(); // get available physical page

		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
/*				int* index = new int;
			*index = i;
			currentThread->pages->Append((void*)index);
*/
//		pageTable[i].physicalPage = ppn;
//printf("pageTable[%d]=%d\n", i, pageTable[i].physicalPage);
//		pageTable[i].valid = TRUE;
		pageTable[i].valid = FALSE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
						// a separate page, we could set its 
						// pages to be read-only
		pageTable[i].type = EXECUTABLE;
		pageTable[i].byteoffset = noffH.code.inFileAddr + i*PageSize;
		pageTable[i].location = executable;

		// add ITP info
/*		ipt[ppn].virtualPage = i;
		ipt[ppn].physicalPage = ppn;
		ipt[ppn].valid = TRUE;
		ipt[ppn].use = FALSE;
		ipt[ppn].dirty = FALSE;
		ipt[ppn].readOnly = FALSE;
		ipt[ppn].space = this;*/
	}

	for (i; i < numPages; i++) {
		pageTable[i].virtualPage = i;
		pageTable[i].valid = FALSE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;
		pageTable[i].type = NEITHER;
		pageTable[i].byteoffset = -1;
		pageTable[i].location = NULL;
	}
	
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
//    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory


/*	for (i=0; i < numPages; i++) {
		DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			pageTable[i].physicalPage*PageSize, PageSize);
		executable->ReadAt(
			&(machine->mainMemory[pageTable[i].physicalPage * PageSize]),
			PageSize, noffH.code.inFileAddr + i*PageSize);
	}
*/

/*
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }
*/
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
//printf("%d\n", numPages*PageSize-16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	for (int i=0; i < TLBSize; i++) {
		machine->tlb[i].valid = false;
	}

	(void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
//    machine->pageTable = pageTable;
//    machine->pageTableSize = numPages;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	for (int i=0; i < TLBSize; i++) {
		machine->tlb[i].valid = false;
	}

	(void) interrupt->SetLevel(oldLevel);
}

void AddrSpace::Dump()
{
	DEBUG('b', "Available virtual pages in AddrSpace %x\n", this);
	unsigned int i = 0;
	for (i; i < numPages; i++) {
		if (pageTable[i].valid == true) {
			DEBUG('b', "%d,", i);
		}
		else {
			DEBUG('b', "x,");
		}
		if (i % 8 == 0) {
			DEBUG('b', "\n");
		}
	}
	if (i % 8 != 1) {
		DEBUG('b', "\n");
	}
}

int* AddrSpace::AddStack()
{
//printf("Adding stack...\n");
	memlock->Acquire();
		// keep track of old page table
		// make a new one with 8 extra pages
		// copy all old pages to new pages
		// allocate 8 new pages
		// deallocate old one

		PTentry* oldPT = pageTable;
		pageTable = new PTentry[numPages + 8];
		unsigned int i;

		for (i=0; i < numPages; i++) {
			pageTable[i].virtualPage = oldPT[i].virtualPage;
			pageTable[i].valid = oldPT[i].valid;
			pageTable[i].use = oldPT[i].use;
			pageTable[i].dirty = oldPT[i].dirty;
			pageTable[i].readOnly = oldPT[i].readOnly;
			pageTable[i].byteoffset = oldPT[i].byteoffset;
			pageTable[i].type = oldPT[i].type;
			pageTable[i].location = oldPT[i].location;
		}

		// update page table size (increase it by 8)
		numPages += 8;

		int ppn;
		for (i; i < numPages; i++) {
			ppn = memMap->Find();

			pageTable[i].virtualPage = i;
			pageTable[i].valid = FALSE;
			pageTable[i].use = FALSE;
			pageTable[i].dirty = FALSE;
			pageTable[i].readOnly = FALSE;
			pageTable[i].byteoffset = -1;
			pageTable[i].type = NEITHER;
			pageTable[i].location = NULL;

		}

//		machine->pageTable = pageTable; // using TLB instead of pageTable from now on

		delete oldPT;

		int* retval = new int[2];
		retval[0] = numPages * PageSize - 16;
		retval[1] = numPages - 1;
	memlock->Release();
	return retval; // set stackreg of the new thread
							// to prevent race condition in case of context switching
}

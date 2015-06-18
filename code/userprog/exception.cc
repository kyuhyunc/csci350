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
//printf("In kernel_fork with currentThread=%s\n", currentThread->getName());
  currentThread->space->InitRegisters();
  machine->WriteRegister(PCReg, pc);
  machine->WriteRegister(NextPCReg, pc+4);

  currentThread->space->RestoreState();

  machine->Run();
}

void Fork_Syscall(int pc) {
  // validate data
//printf("In Fork_Syscall with currentThread=%s, pc=%d\n", currentThread->getName(), pc);

  // if null, print error

  // retrieve program counter

  // allocate memory
  // create 8 extra pages for new thread's stack
  // copy over all old data
  currentThread->space->AddStack(); // calls AddrSpace's private function as a friend

  // update process table for multiprogramming part


  // actual implementation
  Thread* t = new Thread("kernel_forker");
  t->space = currentThread->space; // all threads of same process has same AddrSpace
  t->Fork((VoidFunctionPtr)kernel_fork, pc);


}

void Exit_Syscall(int status) {
  currentThread->Finish();
}

void Yield_Syscall() {
  printf("Current thread yielded\n");
  currentThread->Yield();
}

int CreateLock_Syscall(int vaddr, int size) {
  //it returns -1 when user can't create lock for some reason.
  //otherwise, it returns index of table where the lock that user creates is located. 

  char *buf = new char[size+1]; // Kernel buffer to put the name in
   
    if (!buf) {
    printf("%s","Can't allocate kernel buffer in Open\n");
    return -1;
    }

  if(copyin(vaddr, size, buf) == -1) {
    //check if the pointer is valid one. if pointer is not valid, then return.
    printf("error: Pointer is invalid\n");
    return -1;
  }
  
  char * name = currentThread->getName();

  if(name == buf) {
    printf("it already exist in table. You can not create the lock\n");
    return -1;
  }

  //creating lock
  Lock * l = new Lock(name);
  int index = locktable->Put(l);
  //return when you can't put lock in the table.
  if(locktable->Put(l) == -1) {
    return -1;
  }

  return index;
}

int DestroyLock_Syscall(int, int) {
  
}

int Acquire_Syscall(int, int) {
  
}

int Release_Syscall(int, int) {
  
}

int CreateCV_Syscall(int, int) {
  
}

int DestroyCV_Syscall(int, int) {
  
}

int Wait_Syscall(int, int) {
  
}

int Signal_Syscall(int, int) {
  
}

int Broadcast_Syscall(int, int) {
  
}

void Printf0_Syscall(unsigned int vaddr, int len) {
  
  char* buf;
  
  if (!(buf = new char[len])) {
    printf("Error allocating kernel buffer for write!\n");
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

void Printf1_Syscall(unsigned int vaddr, int len, int num1) {
  
}

void Printf2_Syscall(unsigned int vaddr, int len, int num1, int num2) {
  
}

/*void Printf_Syscall(int buffer, int num1, int num2, int num3) {
  char* buf = (char*) buffer;
  printf(buf);
  if (buf == NULL || num1 == NULL || num2 == NULL || num3 == NULL) {
    printf("Illegal operation: cannot pass a NULL pointer into Printf\n");
  }
  if (num1 == -1 && num2 == -1 && num3 == -1) { // no arguments
    printf(buf);
  }
  else if (num2 == -1 && num3 == -1) { // one arg
    printf(buf, num1);
  }
  else if (num3 == -1) { // two args
    printf(buf, num1, num2);
  }
  else { // three args
    printf(buf, num1, num2, num3);
  }
}*/

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
        Write_Syscall(machine->ReadRegister(4),
              machine->ReadRegister(5),
              machine->ReadRegister(6));
        break;
      case SC_Read:
        DEBUG('a', "Read syscall.\n");
        rv = Read_Syscall(machine->ReadRegister(4),
              machine->ReadRegister(5),
              machine->ReadRegister(6));
        break;
      case SC_Close:
        DEBUG('a', "Close syscall.\n");
        Close_Syscall(machine->ReadRegister(4));
        break;
      case SC_Fork:
        DEBUG('a', "Fork syscall.\n");
        Fork_Syscall(machine->ReadRegister(4));
        break;
      case SC_Yield:
        DEBUG('a', "Yield syscall.\n");
        Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_CreateLock:
        DEBUG('a', "CreateLock syscall.\n");
        CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_DestroyLock:
        DEBUG('a', "DestroyLock syscall.\n");
        DestroyLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_Acquire:
        DEBUG('a', "Acquire syscall.\n");
        Acquire_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_Release:
        DEBUG('a', "Release syscall.\n");
        Release_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_CreateCV:
        DEBUG('a', "CreateCV syscall.\n");
        CreateCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_DestroyCV:
        DEBUG('a', "DestroyCV syscall.\n");
        DestroyCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_Wait:
        DEBUG('a', "Wait syscall.\n");
        Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_Signal:
        DEBUG('a', "Signal syscall.\n");
        Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_Broadcast:
        DEBUG('a', "Broadcast syscall.\n");
        Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
      case SC_Printf0:
        DEBUG('a', "Printf0 syscall.\n");
        Printf0_Syscall(machine->ReadRegister(4),
              machine->ReadRegister(5));
      case SC_Printf1:
        DEBUG('a', "Printf1 syscall.\n");
        Printf1_Syscall(machine->ReadRegister(4),
              machine->ReadRegister(5),
              machine->ReadRegister(6));
      case SC_Printf2:
        DEBUG('a', "Printf2 syscall.\n");
        Printf2_Syscall(machine->ReadRegister(4),
              machine->ReadRegister(5),
              machine->ReadRegister(6),
              machine->ReadRegister(7));
        break;
    }

    // Put in the return value and increment the PC
    machine->WriteRegister(2,rv);
    machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
    return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
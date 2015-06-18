// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include <iostream>

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
	name = debugName;
	owner = NULL;
	waitQueue = new List;
}

Lock::~Lock() {
	delete waitQueue;
}

void Lock::Acquire() {
	// disable interrupts
	IntStatus old = interrupt->SetLevel(IntOff);
	// checks if I'm the lock owner
	// if I'm the lock owner, I already "acquired" this lock
	// this can happen if class functions are nested
	// in this case, the lock will never be released without this check
	if (owner == currentThread) {
		// restore interrupts
		(void) interrupt->SetLevel(old);
		return;
	}

	// checks if lock is available
	// if nobody owns the lock, I can have it!
	if (owner == NULL) {
		owner = currentThread;
	}
	
	// checks if lock is not available
	// if somebody else knows the lock, I can't have it :(
	else {
		// add myself to queue and put myself to sleep
    //if (strcmp(currentThread->getName(), "manager"))
		waitQueue->Append((void *)currentThread);
		currentThread->Sleep();
	}

	// restore interrupts
	(void) interrupt->SetLevel(old);
}

void Lock::Release() {
	// disable interrupts
	IntStatus old = interrupt->SetLevel(IntOff);

	// checks if I'm not the lock owner
	// a non-lock owner cannot release a lock they don't own...
	if (!isHeldByCurrentThread()) {
		printf("Error in Lock::Release -- only lock owners can release locks...\n");
		// restore interrupts and return
		(void) interrupt->SetLevel(old);
		return;
	}

	// checks if any thread is waiting for this lock
	// if there are, give it to them based on queue order
	if (!waitQueue->IsEmpty()) {
		Thread* t = (Thread*) waitQueue->Remove();
		scheduler->ReadyToRun(t); // wake up sleepy thread
		owner = t;
	}

	// if there are no threads waiting for this lock,
	// simply free it so other threads can use it when necessary
	else {
		owner = NULL;
	}

	// restore interrupts
	(void) interrupt->SetLevel(old);
}

bool Lock::isHeldByCurrentThread() {
	return (owner == currentThread);
}

Condition::Condition(char* debugName) {
	name = debugName;
	waitQueue = new List;
	waitingLock = NULL;
}

Condition::~Condition() {
	delete waitQueue;
}

void Condition::Wait(Lock* conditionLock) {
	// disable interrupts
	IntStatus old = interrupt->SetLevel(IntOff);

	// if programmer irresponsibly passes in null printer, print error
	if (conditionLock == NULL) {
		printf("Error in Condition::Wait -- parameter conditionLock cannot be NULL...\n");
		// restore interrupts and return
		(void) interrupt->SetLevel(old);
		return;
	}

	// if no lock is attached to this condition yet,
	// attach it!
	// this block will execute if the first thread with a lock calls wait
	// or if no lock is attached to this condition variable
	if (waitingLock == NULL) {
		waitingLock = conditionLock;
	}

	// if conditionLock does not match waitingLock, print error
	if (waitingLock != conditionLock) {
		printf("Error in Condition::Wait -- conditionLock does not match waitingLock...\n");
		// restore interrupts and return
		(void) interrupt->SetLevel(old);
		return;
	}

	conditionLock->Release(); // exit critical section
	waitQueue->Append((void *)currentThread);
	currentThread->Sleep();

	conditionLock->Acquire(); // enter critical section again

	// restore interrupts and return
	(void) interrupt->SetLevel(old);
}
void Condition::Signal(Lock* conditionLock) {
	// disable interrupts
	IntStatus old = interrupt->SetLevel(IntOff);

	// if no waiting threads, then nothing to do
	if (waitQueue->IsEmpty()) {
		// restore interrupts and return
		printf("Error in Condition::Signal: There is no thread to Signal. currentThread: %s", currentThread->getName());
		(void) interrupt->SetLevel(old);
		return;
	}


	// if conditionLock does not match waitingLock, print error
	if (waitingLock != conditionLock) {
		printf("Error in Condition::Signal -- conditionLock does not match waitingLock...\n");
		// restore interrupts and return
		(void) interrupt->SetLevel(old);
		return;
	}

	// Wakeup 1 waiting thread
	Thread* t = (Thread*) waitQueue->Remove();
	std::cout << currentThread->getName() << " properly signaled " << t->getName() << std::endl;
	scheduler->ReadyToRun(t); // wake up sleepy thread

	// if after waking up this thread there are none left,
	// release the lock
	if (waitQueue->IsEmpty()) {
		waitingLock = NULL;
	}

	// restore interrupts and return
	(void) interrupt->SetLevel(old);
}
void Condition::Broadcast(Lock* conditionLock) {
	// if conditionLock does not match waitingLock, print error
	if (waitingLock != conditionLock) {
		printf("Error in Condition::Broadcast -- conditionLock does not match waitingLock...\n");
		return;
	}

	while (!waitQueue->IsEmpty()) {
		Signal(conditionLock);
	}
}

// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}



//	Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);

	TestSuite();
}

//----------------------------------------------------------------------
// Airport
//----------------------------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <time.h>
#include <vector>

#define NUM_PASSENGERS 20
#define NUM_LIASONS 7
#define NUM_AIRLINES 3
#define NUM_CIS_PER_AIRLINE 5
#define NUM_CARGO_HANDLERS 10
#define NUM_SCREENING_OFFICERS 5 // what num?
#define NUM_SECURITY_INSPECTORS 5 // what num?

enum State {
	AVAIL,
	BUSY,
	ONBREAK
};

//-----------------------
// Passenger
//-----------------------

struct Ticket {
	bool _executive;
	int _airline; // choices between 0, 1, and 2
};

struct Baggage {
	int _airline;
	int _weight; // between 30 and 60
	
};

class Passenger : public Thread
{
public:
	Passenger(char* debugName) : Thread(debugName) {
		//------------
		// Initialize
		//------------

		// 2 or 3 baggages 30-60 lbs
		Baggage* mybag;
		int numBaggages = rand() % 2 + 2;
		for (int i=0; i < numBaggages; i++) {
			mybag = new Baggage;
			mybag->_weight = rand() % 31 + 30;
			_baggages.push_back(mybag);
		}

		// Executive or Economy ticket
		_myticket._executive = false;
		if ( (rand() % 2) == 1 ) {
			_myticket._executive = true;
		}

		// Airline number (choose between 0, 1, and 2)
		_myticket._airline = rand() % 3;
	}
	void Start(); // starts the thread

public:
	std::vector<Baggage*> _baggages;
	Ticket _myticket;
};

//-----------------------
// Airport Liaison
//-----------------------

class Liaison : public Thread
{
public:
	Liaison(char* debugName) : Thread(debugName) {
		_lock = new Lock("liaison0_lock");// + "_lock");
		_lineCV = new Condition("liaison0_lineCV");// + "_lineCV");
		_commCV = new Condition("liaison0_commCV");// + "_commCV");
		_lineSize = 0;

		_state = BUSY;
		for (int i=0; i < NUM_AIRLINES; i++) {
			_passCount[0] = 0;
			_bagCount[0] = 0;
		}
	}
	void Start(); // starts the thread

	void updatePassengerInfo(Passenger* pass) {
		_passCount[pass->_myticket._airline]++;
		_passCount[pass->_myticket._airline] += pass->_baggages.size();
		_currentPassenger = pass;
	}

public:
	Lock* _lock;
	Condition* _lineCV;
	Condition* _commCV;
	int _lineSize;

	int _state; // AVAIL or BUSY
	int _passCount[NUM_AIRLINES];
	int _bagCount[NUM_AIRLINES];

	Passenger* _currentPassenger;
	
};

//-----------------------
// Airport Check-In Staff
//-----------------------

class CheckInStaff : public Thread
{
public:
	CheckInStaff(char* debugName) : Thread(debugName) {
		
	}
	void Start(); // starts the thread

public:

};

//-----------------------
// Cargo Handler
//-----------------------

class CargoHandler : public Thread
{
public:
	CargoHandler(char* debugName) : Thread(debugName) {
		
	}
	void Start(); // starts the thread

private:

};

//-----------------------
// Screening Officer
//-----------------------

class ScreeningOfficer : public Thread
{
public:
	ScreeningOfficer(char* debugName) : Thread(debugName) {
		
	}
	void Start(); // starts the thread

private:

};

//-----------------------
// Security Inspector
//-----------------------

class SecurityInspector : public Thread
{
public:
	SecurityInspector(char* debugName) : Thread(debugName) {
		
	}
	void Start(); // starts the thread

private:

};

//-----------------------
// Manager
//-----------------------

class Manager : public Thread
{
public:
	Manager(char* debugName) : Thread(debugName) {
		
	}
	void Start(); // starts the thread

private:

};

//-----------------------
// Data
//-----------------------

Passenger* passengers[NUM_PASSENGERS];
Liaison* liaisons[NUM_LIASONS];
// Cis*
CheckInStaff* checkinstaff[NUM_AIRLINES][NUM_CIS_PER_AIRLINE];
CargoHandler* cargohandlers[NUM_CARGO_HANDLERS];
ScreeningOfficer* screeningofficers[NUM_SCREENING_OFFICERS];
SecurityInspector* securityinspectors[NUM_SECURITY_INSPECTORS];
Manager* manager;

Lock* LiaisonGlobalLineLock;

struct SecureData {
	
} SecurityCloud;

//-----------------------
// Thread Functions
//-----------------------

void PassengerStart(int index)
{
	passengers[index]->Start();
}

void LiaisonStart(int index)
{
	liaisons[index]->Start();
}

void Cis0Start(int index)
{
	// a bit finicky...
	checkinstaff[0][index]->Start();
}

void Cis1Start(int index)
{
	checkinstaff[1][index]->Start();
}

void Cis2Start(int index)
{
	checkinstaff[2][index]->Start();
}

void CargoHandlerStart(int index)
{
	cargohandlers[index]->Start();
}

void ScreeningOfficerStart(int index)
{
	screeningofficers[index]->Start();
}

void SecurityInspectorStart(int index)
{
	securityinspectors[index]->Start();
}

void ManagerStart()
{
	manager->Start();
}

//-----------------------
// Start Functions
//-----------------------

void Passenger::Start()
{
	printf("%s: Made it!\n", this->getName());

	// enter terminal
	// goes to Airport Liaison, choosing shortest line
	int myLine = 0;
	int lineSize = liaisons[0]->_lineSize;
	LiaisonGlobalLineLock->Acquire();

	// find shortest line
	for (int i=0; i < NUM_LIASONS; i++) {
		if (liaisons[i]->_lineSize < lineSize) {
			lineSize = liaisons[i]->_lineSize;
			myLine = i;
		}
	}

	printf("Passenger %s chose Liaison %s with a line length %i\n", getName(), liaisons[myLine]->getName(), lineSize);

	if (liaisons[myLine]->_state == BUSY) {
		liaisons[myLine]->_lineSize++;
		liaisons[myLine]->_lineCV->Wait(LiaisonGlobalLineLock);
		liaisons[myLine]->_lineSize--;
	}

	liaisons[myLine]->_lock->Acquire();
	LiaisonGlobalLineLock->Release();

	// hands ticket to Liaison
	liaisons[myLine]->updatePassengerInfo(this);

	liaisons[myLine]->_commCV->Signal(liaisons[myLine]->_lock);
	liaisons[myLine]->_commCV->Wait(liaisons[myLine]->_lock);

	// receives instruction from Liaison on which terminal to go to
	printf("Passenger %s of Airline %i is directed to the check-in counter\n", getName(), _myticket._airline);

	liaisons[myLine]->_commCV->Signal(liaisons[myLine]->_lock);

	liaisons[myLine]->_lock->Release();




	// if executive passenger
		// go to executive line
		// get helped by cis

	// if economy passenger
		// choose shortest cis line
		// get helped by cis
	
	// go through security...
	// ...
	// ...

	// wait in boarding lounge until boarding announcement
}

void Liaison::Start()
{
	printf("%s: Made it!\n", this->getName());

	// while loop
		// help passenger
		
		// receives ticket from passenger
		// keeps track of passenger count and baggages

	char* statement;
	
	while (true) {
		_lock->Acquire();
		// ISSUE HERE, FIX
		LiaisonGlobalLineLock->Acquire();
		if (_lineSize == 0) {
			LiaisonGlobalLineLock->Release();
			_state = AVAIL;
			_commCV->Wait(_lock);
		}
		else {
			_lineCV->Signal(LiaisonGlobalLineLock);
			LiaisonGlobalLineLock->Release();
			_commCV->Wait(_lock);
		}

		_state = BUSY;

		// guides them to proper airport terminal (based on airline)
		printf("Airport Liaison %s directed passenger %s of airline %i\n", getName(), _currentPassenger->getName(), _currentPassenger->_myticket._airline);

		// print statement

		_commCV->Signal(_lock);
		_commCV->Wait(_lock);

		_lock->Release();

	}

}

void CheckInStaff::Start()
{
	printf("%s: Made it!\n", this->getName());

	// while loop
		// if an executive passenger is waiting, help them

		// if there is no executive passenger, help the first person in current line

		// if there are no passengers, go on break

		// accepts passenger ticket
		// accepts passenger baggage
		// assigns passenger seat number (no overlaps)
			// (make sure there are enough seats! each airplane must have at NUM_PASSENGERS seats to ensure enough)
		// tags baggage with airline code and weight
		// places baggage on conveyor system


		
		// .
		// .
		// .
		// when all passengers checked in, close check-in counter

}

void CargoHandler::Start()
{
	printf("%s: Made it!\n", this->getName());
}

void ScreeningOfficer::Start()
{
	printf("%s: Made it!\n", this->getName());
}

void SecurityInspector::Start()
{
	printf("%s: Made it!\n", this->getName());
}

void Manager::Start()
{
	printf("%s: Made it!\n", this->getName());
}




//-----------------------
// Run Airport Simulation
//-----------------------

void AirportSim()
{
	// Setup
	char* name;
	srand(time(NULL));
	
	// Initialize locks
	LiaisonGlobalLineLock = new Lock("liason_global_line_lock");
	
	// Activating passenger threads
	Passenger* p;
	for (int i=0; i < NUM_PASSENGERS; i++) {
		name = new char[20];
		sprintf(name, "passenger%d", i);

		p = new Passenger(name);
		passengers[i] = p;
	}

	// Activating liaison threads
	Liaison* l;
	for (int i=0; i < NUM_LIASONS; i++) {
		name = new char[20];
		sprintf(name, "liaison%d", i);

		l = new Liaison(name);
		liaisons[i] = l;
	}

	for (int i=0; i < NUM_PASSENGERS; i++) {
		passengers[i]->Fork((VoidFunctionPtr)PassengerStart, i);
	}
	for (int i=0; i < NUM_LIASONS; i++) {
		liaisons[i]->Fork((VoidFunctionPtr)LiaisonStart, i);
	}
/*
	// CIS
	// kinda finicky
	// gotta decide how to do this...
	CheckInStaff* cis;
	for (int i=0; i < NUM_AIRLINES; i++) {
		for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
			name = new char[20];
			sprintf(name, "cis_%d_%d", i, j);

			cis = new CheckInStaff(name);
			checkinstaff[i][j] = cis;
			if (i == 0) {
				cis->Fork((VoidFunctionPtr)Cis0Start, j);
			}
			else if (i == 1) {
				cis->Fork((VoidFunctionPtr)Cis1Start, j);
			}
			else if (i == 2) {
				cis->Fork((VoidFunctionPtr)Cis2Start, j);
			}
		}
	}

	// Activating cargo handler threads
	CargoHandler* ch;
	for (int i=0; i < NUM_CARGO_HANDLERS; i++) {
		name = new char[20];
		sprintf(name, "cargo_handler%d", i);

		ch = new CargoHandler(name);
		cargohandlers[i] = ch;
		ch->Fork((VoidFunctionPtr)CargoHandlerStart, i);
	}

	// Activating screening officer threads
	ScreeningOfficer* so;
	for (int i=0; i < NUM_SCREENING_OFFICERS; i++) {
		name = new char[25];
		sprintf(name, "screening_officer%d", i);

		so = new ScreeningOfficer(name);
		screeningofficers[i] = so;
		so->Fork((VoidFunctionPtr)ScreeningOfficerStart, i);
	}

	// Activating security inspector threads
	SecurityInspector* si;
	for (int i=0; i < NUM_SECURITY_INSPECTORS; i++) {
		name = new char[25];
		sprintf(name, "security_inspectors%d", i);

		si = new SecurityInspector(name);
		securityinspectors[i] = si;
		si->Fork((VoidFunctionPtr)SecurityInspectorStart, i);
	}

	// Activating manager thread
	Manager* m;
	name = new char[20];
	sprintf(name, "manager");

	m = new Manager(name);
	manager = m;
	m->Fork((VoidFunctionPtr)ManagerStart, 0);
	*/
}

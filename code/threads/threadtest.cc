// threadtest.cc 
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield, 
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
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



//  Simple test cases for the threads assignment.
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
Lock t1_l1("t1_l1");          // the lock tested in Test 1

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

    t1_s1.P();  // Wait until t1 has the lock
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

    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock

    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
    printf("%s: Trying to release Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
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
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
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
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
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
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
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
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
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
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
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
//   4.  Show that Broadcast wakes all waiting threads
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
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
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

/*
#define NUM_PASSENGERS 20
#define NUM_LIASONS 7
#define NUM_AIRLINES 3
#define NUM_CIS_PER_AIRLINE 5
#define NUM_CARGO_HANDLERS 10
#define NUM_SCREENING_OFFICERS 5 // what num?
#define NUM_SECURITY_INSPECTORS 5 // what num?
*/

int NUM_PASSENGERS;
int NUM_LIASONS;
int NUM_AIRLINES;
int NUM_CIS_PER_AIRLINE;
int NUM_CARGO_HANDLERS;
int NUM_SCREENING_OFFICERS;
int NUM_SECURITY_INSPECTORS;

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
    int _seat; // uninitialized--done by cis
};

struct Baggage {
    int _airline; // uninitialized--done by cis
    int _weight; // between 30 and 60
    
};

class Passenger : public Thread
{
public:
	Passenger(char* debugName, int id) : Thread(debugName) {
		//------------
		// Initialize
		//------------
		myLine = 0;
        _id = id;
        _furtherQuestioning = false;

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
		if ( (rand() % 4) == 1 ) {
			_myticket._executive = true;
		}

		// Airline number 
		_myticket._airline = rand() % NUM_AIRLINES;
	}
	void Start(); // starts the thread

public:
	std::vector<Baggage*> _baggages;
	Ticket _myticket;
	int myLine;
    int myOfficer;
    int _myInspector;
    int _id;
    bool _furtherQuestioning;
};

//-----------------------
// Airport Liaison
//-----------------------

class Liaison : public Thread
{
public:
    Liaison(char* debugName) : Thread(debugName) {
        char* myname;

        myname = new char[20];
        sprintf(myname, "%s_lock", debugName);
        _lock = new Lock(myname);

        myname = new char[20];
        sprintf(myname, "%s_lineCV", debugName);
        _lineCV = new Condition(myname);

        myname = new char[20];
        sprintf(myname, "%s_commCV", debugName);
        _commCV = new Condition(myname);
        _lineSize = 0;

        _state = BUSY;

        _passCount = new int[NUM_AIRLINES];
        _bagCount = new int[NUM_AIRLINES];

        for (int i=0; i < NUM_AIRLINES; i++) {
            _passCount[0] = 0;
            _bagCount[0] = 0;
        }

        _totalNumber = 0;
    }
    void Start(); // starts the thread

    void updatePassengerInfo(Passenger* pass) {
        _passCount[pass->_myticket._airline]++;
        _bagCount[pass->_myticket._airline] += pass->_baggages.size();
        _currentPassenger = pass;
    }

public:
    Lock* _lock;
    Condition* _lineCV;
    Condition* _commCV;
    int _lineSize;

    int _state; // AVAIL or BUSY
    int* _passCount;
    int* _bagCount;

    Passenger* _currentPassenger;

    int _totalNumber;
    
};

//-----------------------
// Airport Check-In Staff
//-----------------------

class CheckInStaff : public Thread
{
public:
    CheckInStaff(char* debugName, int airline, int cisnum) : Thread(debugName) {
        char* myname;
        _airline = airline;
        _cisNum = cisnum;

        myname = new char[20];
        sprintf(myname, "%s_lock", debugName);
        _lock = new Lock(myname);

        myname = new char[20];
        sprintf(myname, "%s_lineCV", debugName);
        _lineCV = new Condition(myname);

        myname = new char[20];
        sprintf(myname, "%s_commCV", debugName);
        _commCV = new Condition(myname);

        _lineSize = 0;

        _state = BUSY;
        _passCount = 0;
        _bagCount = 0;
        _PassengerNumber = 0; // for test case
       
        _done = false; 
    }
    void Start(); // starts the thread

    void updatePassengerInfo(Passenger* pass) {
        _passCount++;
        _bagCount += pass->_baggages.size();
        _currentPassenger = pass;
    }
  
    friend class Manager;

public:
    Lock* _lock;
    Condition* _lineCV;
    Condition* _commCV;
    int _lineSize;

    int _state; // ONBREAK or BUSY
    int _passCount;
    int _bagCount;

    Passenger* _currentPassenger;

    int _PassengerNumber;

private:
    int _airline;
    int _cisNum;

    bool _done;
};

//-----------------------
// Cargo Handler
//-----------------------

class CargoHandler : public Thread
{
public:
    CargoHandler(char* debugName) : Thread(debugName) {
        _bagCount = new int[NUM_AIRLINES];
        for (int i=0; i < NUM_AIRLINES; i++) {
            _bagCount[i] = 0;
        }
        char* myname;
        myname = new char[20];
        sprintf(myname, "%s_commCV", debugName);
        _commCV = new Condition(myname);

        _state = BUSY;
    }
    void Start(); // starts the thread

public:
    Condition* _commCV;
    int _state;
    int* _bagCount;

};

//-----------------------
// Screening Officer
//-----------------------

class ScreeningOfficer : public Thread
{
public:
    ScreeningOfficer(char* debugName, int num) : Thread(debugName) {
        char* myname;

        myname = new char[20];
        sprintf(myname, "%s_lock", debugName);
        _lock = new Lock(myname);

        myname = new char[20];
        sprintf(myname, "%s_commCV", debugName);
        _commCV = new Condition(myname);

        _state = BUSY;
        _myNum = num;
    }
    void Start(); // starts the thread

public:
    Lock* _lock;
    Condition* _commCV;

    int _state; // ONBREAK or BUSY
    int _passCount;

    Passenger* _currentPassenger;

    int _myNum;

private:
    bool _done;

};

//-----------------------
// Security Inspector
//-----------------------

class SecurityInspector : public Thread
{
public:
    SecurityInspector(char* debugName, int i) : Thread(debugName) {
        _id = i;
        _state = BUSY;
        _lineSize = 0;

        char* myname;
        myname = new char[20];
        sprintf(myname, "%s_lock", debugName);
        _lock = new Lock(myname);

        myname = new char[20];
        sprintf(myname, "%s_commCV", debugName);
        _commCV = new Condition(myname);

        myname = new char[20];
        sprintf(myname, "%s_lineCV", debugName);
        _lineCV = new Condition(myname);
    }
    void Start(); // starts the thread
public:
    int _id;
    int _lineSize;
    int _state;
    Lock* _lock;
    Condition* _commCV;
    Condition* _lineCV;
    Passenger* _currentPassenger;
private:

};

//-----------------------
// Manager
//-----------------------

class Manager : public Thread
{
public:
    Manager(char* debugName) : Thread(debugName) {
        _cisDone = false;
        _cargoDone = false;
        _soDone = false;
        _siDone = false;       
    }
    void Start(); // starts the thread

private:
    bool _cisDone;
    bool _cargoDone;
    bool _soDone;
    bool _siDone;
};

//-----------------------
// Airline
//-----------------------

class Airline
{
public:
    Airline(char* debugName) {
        _name = debugName;

        _cis = new CheckInStaff*[NUM_CIS_PER_AIRLINE];

        char* myname;
        myname = new char[20];
        sprintf(myname, "%s_execlock", debugName);
        _execLineLock = new Lock(myname);

        myname = new char[20];
        sprintf(myname, "%s_execCV", debugName);
        _execLineCV = new Condition(myname);

        _execLineSize = 0;

        _execQueue = new List;

        myname = new char[20];
        sprintf(myname, "%s_globallock", debugName);
        _CisGlobalLineLock = new Lock(myname);

        myname = new char[20];
        sprintf(myname, "%s_airlinelock", debugName);
        _airlineLock = new Lock(myname);        

        _numExpectedPassengers = 0;
        _numCheckedinPassengers = 0;
        _numReadyPassengers = 0;
        _numExpectedBaggages = 0;
        _numLoadedBaggages = 0;

        _numOnBreakCIS = 0;

        _allPassengersCheckedIn = false;
        _CISclosed = false;
    }
    char* getName() { return _name; }
    
    friend class Manager;
    friend class CheckInStaff; 

public:
    CheckInStaff** _cis;

    Lock* _execLineLock;
    Condition* _execLineCV;
    int _execLineSize;
    List* _execQueue;

    Lock* _CisGlobalLineLock;
    
    Lock* _airlineLock;  
  
    int _numExpectedPassengers;
    int _numCheckedinPassengers;
    int _numReadyPassengers;
    int _numExpectedBaggages;
    int _numLoadedBaggages;

    int _numOnBreakCIS;

private:
    char* _name;
    bool _allPassengersCheckedIn;
    bool _CISclosed;

};

//-----------------------
// SecurityData
//-----------------------

class SecurityData 
{
public:
    SecurityData() {
        _officerResults = new int[NUM_PASSENGERS];
    }
private:
    int* _officerResults;

friend class ScreeningOfficer;
friend class SecurityInspector;
};

//-----------------------
// Data
//-----------------------

/*
Passenger* passengers[NUM_PASSENGERS];
Liaison* liaisons[NUM_LIASONS];
// Cis*
CargoHandler* cargohandlers[NUM_CARGO_HANDLERS];
ScreeningOfficer* screeningofficers[NUM_SCREENING_OFFICERS];
SecurityInspector* securityinspectors[NUM_SECURITY_INSPECTORS];
Manager* manager;
*/
Passenger** passengers;
Liaison** liaisons;
CargoHandler** cargohandlers;
ScreeningOfficer** screeningofficers;
SecurityInspector** securityinspectors;
Manager* manager;


Airline** airlines;

Lock* LiaisonGlobalLineLock;
Lock* CisGlobalLineLock;

// Screening Officer
Lock* officersLineLock;
Condition* officersLineCV;
List* officersLine;

// Conveyor Globals
List* ConveyorBelt;
Lock* ConveyorLock;
Condition* ConveyorCV;
int CargoHandlerState = BUSY;

// Security
SecurityData* securityCloud;

// Security Inspector
Lock* inspectorsLineLock;
List* inspectors;

Semaphore t1("t1",0);
Semaphore t4_1("t4_1",0);
Semaphore t4("t4",0);
bool stopSIM1 = false;
bool semaBool = false;
bool semaExe1 = false;
bool semaExe = false; 
bool stopSIM4 = false;

//-----------------------
// Thread Functions
//-----------------------

void PassengerStart(int index) { passengers[index]->Start(); }
void LiaisonStart(int index) { liaisons[index]->Start(); }
void CisStart(int index) { airlines[index / NUM_CIS_PER_AIRLINE]->_cis[index % NUM_CIS_PER_AIRLINE]->Start(); }
void CargoHandlerStart(int index) { cargohandlers[index]->Start(); }
void ScreeningOfficerStart(int index) { screeningofficers[index]->Start(); }
void SecurityInspectorStart(int index) { securityinspectors[index]->Start(); }
void ManagerStart() { manager->Start(); }

//-----------------------
// Start Functions
//-----------------------

void Passenger::Start()
{

    //----------------------------------------------
    // PASSENGER INTERACTS WITH LIAISON
    //----------------------------------------------

    // enter terminal
    // goes to Airport Liaison, choosing shortest line
    myLine = 0;
    LiaisonGlobalLineLock->Acquire();
    int lineSize = liaisons[0]->_lineSize;

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
        liaisons[myLine]->_totalNumber++;
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

    // For Test purposes. it makes wait testing simulation so that we can analyze result and verify outcome.
    if(semaBool == true) {
        t1.V();
    }
    if(semaExe1 == true) {
        t4_1.V();
    }
    
    //----------------------------------------------
    // END PASSENGER INTERACTS WITH LIAISON
    //----------------------------------------------


    //stop here for test 1 and test 2 simulation
    if(stopSIM1 == true) {
        return;
    }
    
    
    //----------------------------------------------
    // PASSENGER INTERACTS WITH CHECK-IN-STAFF
    //----------------------------------------------

#define myairline airlines[_myticket._airline]
#define checkinstaff airlines[_myticket._airline]->_cis
#define myCis airlines[_myticket._airline]->_cis[myLine]
#define ExecLock airlines[_myticket._airline]->_execLineLock
#define GlobalLock airlines[_myticket._airline]->_CisGlobalLineLock
#define CisLock airlines[_myticket._airline]->_cis[myLine]->_lock
	
	  if (_myticket._executive) {
      ExecLock->Acquire();
      myairline->_execQueue->Append((void*) this);
      myairline->_execLineSize++;

      printf("Passenger %s of Airline %i is waiting in the executive class line\n", getName(), _myticket._airline);
      
      myairline->_execLineCV->Wait(ExecLock); // wait for cis to help me out
      
      ExecLock->Release();
    }
    else {
        GlobalLock->Acquire();

        // find shortest line
        lineSize = checkinstaff[0]->_lineSize;
        myLine = 0;
        for (int i=0; i < NUM_CIS_PER_AIRLINE; i++) {
            if (checkinstaff[i]->_lineSize < lineSize) {
                lineSize = airlines[_myticket._airline]->_cis[i]->_lineSize;
                myLine = i;
            }
        }

        printf("Passenger %s of Airline %i chose Airline Check-In staff %s with a line length %i\n", getName(), _myticket._airline, myCis->getName(), lineSize);

        myCis->_lineSize++;
        myCis->_lineCV->Wait(GlobalLock);
        myCis->_lineSize--;

        GlobalLock->Release();
    }
    CisLock->Acquire();

    // give baggage + ticket to cis
    myCis->updatePassengerInfo(this);

    myCis->_commCV->Signal(CisLock);
    myCis->_commCV->Wait(CisLock); // wait for cis for boarding pass

    // receives boarding pass with seat number
    printf("Passenger %s of Airline %i was informed to board at gate %i\n", getName(), _myticket._airline, _myticket._airline);

    myCis->_commCV->Signal(CisLock);
    CisLock->Release();

#undef myairline
#undef checkinstaff
#undef myCis
#undef ExecLock
#undef GlobalLock
#undef CisLock

    //----------------------------------------------
    // END PASSENGER INTERACTS WITH CHECK-IN-STAFF
    //----------------------------------------------


    //----------------------------------------------
    // PASSENGER INTERACTS WITH SCREENING OFFICER/SECURITY INSPECTOR
    //----------------------------------------------

#define officer screeningofficers[myOfficer]
#define inspector securityinspectors[_myInspector]
    
    // Wait in line
    officersLineLock->Acquire();
    officersLine->Append(this);
    officersLineCV->Wait(officersLineLock); // myOfficer gets updated
    printf("Passenger %s gives the hand-luggage to screening officer %s\n", getName(), screeningofficers[myOfficer]->getName());
    officer->_lock->Acquire();
    officersLineLock->Release();
    officer->_commCV->Signal(officer->_lock);
    officer->_commCV->Wait(officer->_lock); // after, moving along to inspector
    //
    // INSPECTOR
    //
    printf("Passenger %s moves to security inspector %s\n", getName(), inspector->getName());
    inspector->_lock->Acquire();
    officer->_lock->Release();
    // 
    inspector->_currentPassenger = (Passenger*)currentThread;
    inspector->_commCV->Signal(inspector->_lock); // initial alert to officer
    inspector->_commCV->Wait(inspector->_lock); // wait for security results
    if (_furtherQuestioning) {
        printf("Passenger %s goes for futher questioning\n", getName());
        inspector->_lock->Release();
        for (int i = 0; i < 10; ++i) {
            currentThread->Yield(); // simulate further questioning
        }
        printf("Passenger %s comes back to security inspector %s after further examination\n", getName(), inspector->getName());
        // Go see the same security inspector
        inspector->_lock->Acquire();
        inspector->_lineSize++;
        if (inspector->_state == AVAIL) {
            inspector->_commCV->Signal(inspector->_lock);        
        }
        inspector->_lineCV->Wait(inspector->_lock); // wait to be released by inspector
        inspector->_currentPassenger = (Passenger*)currentThread;
        inspector->_lineCV->Signal(inspector->_lock);
        inspector->_lineCV->Wait(inspector->_lock);
    }
    inspector->_lock->Release();

#undef officer 
#undef inspector

    //----------------------------------------------
    // END PASSENGER INTERACTS WITH SCREENING OFFICER
    //----------------------------------------------

    printf("Passenger %s of Airline %d reached the gate %s\n", getName(), _myticket._airline, airlines[_myticket._airline]->getName());

    // wait in boarding lounge until boarding announcement
}

void Liaison::Start()
{
//  printf("%s: Made it!\n", this->getName());

    // while loop
        // help passenger
        
        // receives ticket from passenger
        // keeps track of passenger count and baggages

    while (true) {
        _lock->Acquire();
        LiaisonGlobalLineLock->Acquire();
        if (_lineSize == 0) {
            LiaisonGlobalLineLock->Release();
            _state = AVAIL;
            _commCV->Wait(_lock);
            //for testing purposes(TEST2), in order to complete testing simulation first so that we can analyze the result at the end  
            if(semaBool == true) {
                t1.V();
            }
        }
        else {
            _lineCV->Signal(LiaisonGlobalLineLock);
            LiaisonGlobalLineLock->Release();
            _commCV->Wait(_lock);
            //for testing purposes(TEST2), in order to complete testing simulation first so that we can analyze the result at the end  
            if(semaBool == true) {
                t1.V();
            }
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

#define myairline airlines[_airline]
#define ExecLock airlines[_airline]->_execLineLock
#define GlobalLock airlines[_airline]->_CisGlobalLineLock
// CHECK-IN-STAFF
	bool executive = false;
	while (true) {
    GlobalLock->Acquire();
		ExecLock->Acquire();
    _lock->Acquire();
		if (_lineSize == 0 && myairline->_execLineSize == 0) {
			_state = ONBREAK;
		  _currentPassenger = NULL;
      myairline->_airlineLock->Acquire();
      myairline->_numOnBreakCIS++;
			GlobalLock->Release();
			ExecLock->Release();
      myairline->_airlineLock->Release();
			_commCV->Wait(_lock); // wait for manager to wake me up

      myairline->_airlineLock->Acquire();
      myairline->_numOnBreakCIS--;
      myairline->_airlineLock->Release();

      // if there are no more passengers, cis can exit and be done!
      if (_done) {
        _lock->Release();
        printf("Airline check-in staff %s is closing the counter\n", getName());
        return; 
      }

      GlobalLock->Acquire();
      ExecLock->Acquire();
    }
    _state = BUSY;

		// serving an executive passenger
		if (myairline->_execLineSize > 0) {
			executive = true;
			Passenger* p = (Passenger*) myairline->_execQueue->Remove();
			myairline->_execLineSize--;
			p->myLine = _cisNum;

      //incremeting total number of passenger!
      _PassengerNumber++;

			printf("Airline check-in staff %s of airline %i serves an executive class passenger and economy line length = %i\n", getName(), _airline, _lineSize);

			myairline->_execLineCV->Signal(ExecLock);

      //Testing 4. To wait all executive passengers are processes and to see what happends right after that.
      if(semaExe == true) {
          t4.V();
      }
		}
		// serving an economy passenger
		else if (_lineSize > 0) {
      //stop point so that test 4 simulation can stop! and simulate at this point
      if(stopSIM4) {
          return;
      }
			executive = false;
			
      //incremeting total number of passenger!
      _PassengerNumber++;

			printf("Airline check-in staff %s of airline %i serves an economy class passenger and executive class line length = %i\n", getName(), _airline, myairline->_execLineSize);

			_lineCV->Signal(GlobalLock);

        //for testing purposes(TEST2), in order to complete testing simulation first so that we can analyze the result at the end  
      if(semaBool == true) {
          t1.V();
      }
		}
		GlobalLock->Release();
		ExecLock->Release();

		_commCV->Wait(_lock); // wait for passenger to hand over bags and ticket

		// if serving any passenger
		if (_currentPassenger != NULL) {

			// give passenger boarding pass, seat number
      myairline->_airlineLock->Acquire();
      _currentPassenger->_myticket._seat = myairline->_numCheckedinPassengers;
      myairline->_numCheckedinPassengers++;
      myairline->_airlineLock->Release();

  		// weigh bags, tag bags, check ticket
			// put bags on conveyor belt
      ConveyorLock->Acquire();
      for (unsigned int i=0; i < _currentPassenger->_baggages.size(); i++) {
        _currentPassenger->_baggages.at(i)->_airline = _airline; // tags baggage
        ConveyorBelt->Append((void*) _currentPassenger->_baggages.at(i)); // put bags on conveyor belt

				printf("Airline check-in staff %s of airline %i dropped bags to the conveyor system \n", getName(), _airline);
      }
      ConveyorLock->Release();

      // Direct th passenger
			if (executive) {
				printf("Airline check-in staff %s of airline %i informs executive class passenger %s to board at gate %i\n", getName(), _airline, _currentPassenger->getName(), _airline);
			}
			else {
				printf("Airline check-in staff %s of airline %i informs economy class passenger %s to board at gate %i\n", getName(), _airline, _currentPassenger->getName(), _airline);
			}

			_commCV->Signal(_lock);
			_commCV->Wait(_lock);
			_lock->Release();

      _currentPassenger = NULL;
    }
    else {
      // printf("error: (in CIS) SHOULD NOT REACH HERE");
    }   
  }
#undef myairline
#undef ExecLock
#undef GlobalLock

}

void CargoHandler::Start()
{   
    Baggage* b;

    while (true) {
        ConveyorLock->Acquire();
        if (ConveyorBelt->IsEmpty()) {
            printf("Cargo Handler %s is going for a break\n", getName());
//          ConveyorCV->Wait(ConveyorLock);
            _state = ONBREAK;
            _commCV->Wait(ConveyorLock);
            printf("Cargo Handler %s returned from break\n", getName());
        } else {
            b = (Baggage*) ConveyorBelt->Remove();
            printf("Cargo Handler %s picked bag of airline %i with weighing %i lbs\n", getName(), b->_airline, b->_weight);
            airlines[b->_airline]->_numLoadedBaggages++; // load baggage into proper aircraft
            _bagCount[b->_airline]++;
        }
        ConveyorLock->Release();
    }
}

void ScreeningOfficer::Start()
{
    while (true) {
        officersLineLock->Acquire();
        if (officersLine->IsEmpty()) {
            // officersLineLock->Release();
            _state = ONBREAK;
            _commCV->Wait(officersLineLock); // wait for passenger
        } // get to work, lazy butt!
        if (!officersLine->IsEmpty()) {
            _lock->Acquire();
            _state = BUSY;
            _currentPassenger = (Passenger*)officersLine->Remove();
            _currentPassenger->myOfficer = _myNum;
            officersLineCV->Signal(officersLineLock);
            officersLineLock->Release();
            _commCV->Wait(_lock); // signal Passenger to come
            // observe passenger, generate pass/fail
            int result = rand() % 10;
            securityCloud->_officerResults[_currentPassenger->_id] = result > 8;
            // "direct" the passenger to the security inspector
            if (result > 8) {
                printf("Screening officer %s is suspicious of the hand luggage of passenger %s\n", getName(), _currentPassenger->getName());
            } else {
                printf("Screening officer %s is not suspicious of the hand luggage of passenger %s\n", getName(), _currentPassenger->getName());
            }
            // FIND SHORTEST LINE
            // Acquire global line lock
            inspectorsLineLock->Acquire();
            int shortLineIndex = -1;
            while (shortLineIndex == -1) {
                for (int i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
                    #define inspector securityinspectors[i]
                    inspector->_lock->Acquire();
                    if (inspector->_state == AVAIL) {
                        shortLineIndex = inspector->_id;
                        securityinspectors[shortLineIndex]->_state = BUSY;
                    }
                    else {
                        //std::cout << inspector->getName() << " is busy" << std::endl;
                    }
                    inspector->_lock->Release();
                    #undef inspector
                }
                if (shortLineIndex == -1) {
                    currentThread->Yield();
                }
            }
            // update Passenger code
            _currentPassenger->_myInspector = shortLineIndex;
            printf("Screening officer %s directs passenger %s to security inspector %s\n", getName(), _currentPassenger->getName(), securityinspectors[shortLineIndex]->getName());
            _commCV->Signal(_lock); // wake up the passenger
            inspectorsLineLock->Release();
            _lock->Release();
        }
        else {
            officersLineLock->Release();
        }
    }
}

void SecurityInspector::Start()
{
    while (true) {
        _lock->Acquire();
        if (_lineSize == 0) {
            _state = AVAIL;
            //std::cout << "    >>>>    " << getName() << " is waiting" << std::endl;
            _commCV->Wait(_lock);
            //std::cout << "    >>>>    " << getName() << " woke up" << std::endl;
        }
        if (_lineSize == 0) { // nobody returned from further questions
            _state = BUSY;
            // bool result = rand() % 10 > 7;
            bool result = true;
            bool guilty = result || securityCloud->_officerResults[_currentPassenger->_id];
            // securityCloud->_officerResults[_currentPassenger->_id] = result;
            // "direct" the passenger to the security inspector
            if (result) {
                printf("Security Inspector %s is suspicious of the hand luggage of passenger %s\n", getName(), _currentPassenger->getName());
            } else {
                printf("Security Inspector %s is not suspicious of the hand luggage of passenger %s\n", getName(), _currentPassenger->getName());
            }
            if (guilty) {
                _currentPassenger->_furtherQuestioning = true;
                printf("Security inspector %s asks passenger %s to go for further examination\n", getName(), _currentPassenger->getName());
            } else {
                printf("Security inspector %s allows passenger %s to board \n", getName(), _currentPassenger->getName());
            }
            _commCV->Signal(_lock);
        }
        else {
            while (_lineSize > 0) {
                _state = BUSY;
                _lineCV->Signal(_lock);
                _lineCV->Wait(_lock); // currentPassenger will get updated...
                printf("Security inspector %s permits returning passenger %s to board\n", getName(), _currentPassenger->getName());
                std::cout << "There are " << _lineSize << " passengers" << std::endl;
                --_lineSize;
                _lineCV->Signal(_lock);
            }
        }
        _lock->Release();
    }
}

void Manager::Start()
{
//  printf("%s: Made it!\n", this->getName());


    //----------------------------------------------
    // MANAGER CHECKS CIS LINES
    //----------------------------------------------

#define ExecLock airlines[i]->_execLineLock
#define GlobalLock airlines[i]->_CisGlobalLineLock
#define CisLock airlines[i]->_cis[j]->_lock
#define ExecLine airlines[i]->_execLineSize
#define CisLine airlines[i]->_cis[j]->_lineSize
#define Cis airlines[i]->_cis[j]
#define airLock airlines[i]->_airlineLock

    while (true) {
      if (!_cisDone) {
        _cisDone = true;
        for (int i=0; i < NUM_AIRLINES; i++) {
          // If checked in passengers and expected passengers are the same
          // And all CISes are on break
          // airline is done with checking in the passengers
         
          if (airlines[i]->_CISclosed == false) { 
            airLock->Acquire();
            if (airlines[i]->_numExpectedPassengers == airlines[i]->_numCheckedinPassengers) {
              if (airlines[i]->_numOnBreakCIS == NUM_CIS_PER_AIRLINE) 
                airlines[i]->_allPassengersCheckedIn = true;
              else
                airlines[i]->_allPassengersCheckedIn = false;
            }
            airLock->Release();
          }

          // If all passengers have checked in, signal CISes to make them go home
          if (airlines[i]->_allPassengersCheckedIn == true && airlines[i]->_CISclosed == false) {
            for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
              printf("1111111111111111111111111111\n");
              CisLock->Acquire();
              printf("2222222222222222222222222222\n");
              Cis->_done = true;
              Cis->_commCV->Signal(CisLock);
              CisLock->Release();
            }
            airlines[i]->_CISclosed = true;
          }
          else if (airlines[i]->_allPassengersCheckedIn == false && airlines[i]->_CISclosed == false) {
            // Check if all airlines have checked in the passengers
            _cisDone = false;

            // if airline has not been closed yet and all passengers have not checked in, wake up the CISes

            for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
                CisLock->Acquire();
                GlobalLock->Acquire();
                ExecLock->Acquire();
                if ((ExecLine > 0 || CisLine > 0) && Cis->_state == ONBREAK) {
                    Cis->_commCV->Signal(CisLock);
                }
                CisLock->Release();
                GlobalLock->Release();
                ExecLock->Release();
            }
          }
          else if (airlines[i]->_allPassengersCheckedIn == false && airlines[i]->_CISclosed == true) {
            printf("ERROR: should not reach here \n");
            return;
          }
        }        
      }

#undef ExecLock
#undef GlobalLock
#undef CisLock
#undef ExecLine
#undef CisLine
#undef Cis
#undef airLock

	//----------------------------------------------
	// END MANAGER CHECKS CIS LINES
	//----------------------------------------------

        //----------------------------------------------
        // MANAGER CHECKS SCREENING OFFICERS
        //----------------------------------------------

#define officer screeningofficers[i]       
      officersLineLock->Acquire();
      for (int i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
          if (!officersLine->IsEmpty() && officer->_state == ONBREAK) {
              officer->_commCV->Signal(officersLineLock);
          }
      }
      officersLineLock->Release();

#undef officer


        //----------------------------------------------
        // END MANAGER CHECKS SCREENING OFFICERS
        //----------------------------------------------

        //----------------------------------------------
        // MANAGER CHECKS CONVEYOR BELT
        //----------------------------------------------

        if (!_cargoDone) {
            
            ConveyorLock->Acquire();
/*
            _cargoDone = true;
            for (int i=0; i < NUM_AIRLINES; i++) {
                // checks an airline if all baggages have been loaded
                if (airlines[i]->_numExpectedBaggages == airlines[i]->_numLoadedBaggages) {
                    airlines[i]->_allBaggagesCheckedIn = true;
                }
                if (!airlines[i]->_allBaggagesCheckedIn) {
                    _cargoDone = false;
                    break;
                }
            }
*/          
            bool msg_to_cargos = true;
            for (int i=0; i < NUM_CARGO_HANDLERS; i++) {
                if (!ConveyorBelt->IsEmpty() && cargohandlers[i]->_state == ONBREAK) {
                    cargohandlers[i]->_commCV->Signal(ConveyorLock);
                    if (msg_to_cargos) {
                      printf("Airport manager calls back all the cargo handlers from break\n");
                      msg_to_cargos = false;
                    }
                }
             }

            ConveyorLock->Release();

        }

        //----------------------------------------------
        // END MANAGER CHECKS CONVEYOR BELT
        //----------------------------------------------

        // if all manager tasks are done, break!
        if (_cisDone ) { // ADD CASES AS THE PROJECT GOES ALONG
          printf("Manager: SHUTTING DOWN THE AIRPORT\n");  
          return;
        }
        else {
          //printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }
    } // end while(true) for Manager
} // end Manager::Start()



//-----------------------
// Run Airport Simulation
//-----------------------

void AirportSim()
{
    // Setup
    char* name;
    srand(time(NULL));

    // Read in number of airport people
    printf("Number of airport liaisons = ");
    scanf("%i", &NUM_LIASONS);
    printf("Number of airlines = ");
    scanf("%i", &NUM_AIRLINES);
    printf("Number of check-in-staff = ");
    scanf("%i", &NUM_CIS_PER_AIRLINE);
    printf("Number of cargo handlers = ");
    scanf("%i", &NUM_CARGO_HANDLERS);
    printf("Number of screening officers = ");
    scanf("%i", &NUM_SCREENING_OFFICERS);
    printf("Total number of passengers = ");
    scanf("%i", &NUM_PASSENGERS);

    NUM_SECURITY_INSPECTORS = NUM_SCREENING_OFFICERS;

    /*
    *   Initializing global data
    */

    // Airlines
    airlines = new Airline*[NUM_AIRLINES];
    for (int i=0; i < NUM_AIRLINES; i++) {
        name = new char[20];
        sprintf(name, "airline%d", i);
        airlines[i] = new Airline(name);
    }

    // Passenger
    passengers = new Passenger*[NUM_PASSENGERS];

	// Initialize Liaison
    liaisons = new Liaison*[NUM_LIASONS];
	LiaisonGlobalLineLock = new Lock("liason_global_line_lock");

    // Init Security Cloud data
    securityCloud = new SecurityData();

    // Screening Officer
    screeningofficers = new ScreeningOfficer*[NUM_SCREENING_OFFICERS];

    // Init Screening Officer Globals
    officersLineLock = new Lock("screening_officer_line_lock");
    officersLineCV = new Condition("screening_officer_line_cv");
    officersLine = new List();

    // Security Inspector 
    securityinspectors = new SecurityInspector*[NUM_SECURITY_INSPECTORS];
    inspectorsLineLock = new Lock("security_inspectors_line_lock");


    // Initialize Cargo Handler
    cargohandlers = new CargoHandler*[NUM_CARGO_HANDLERS];
    ConveyorBelt = new List();
    ConveyorLock = new Lock("conveyor_belt_lock");
    ConveyorCV = new Condition("conveyor_belt_cv");

    //----------------------------------------------
    // END SETUP
    //----------------------------------------------


    //----------------------------------------------
    // INITIALIZING ALL THREADS
    //----------------------------------------------
    
    // Initializing passenger threads
    Passenger* p;
    for (int i=0; i < NUM_PASSENGERS; i++) {
        name = new char[20];
        sprintf(name, "passenger%d", i);

		p = new Passenger(name, i);
		passengers[i] = p;

        // Track number of passengers expected per airline
        airlines[p->_myticket._airline]->_numExpectedPassengers++;
    }

    // Initializing liaison threads
    Liaison* l;
    for (int i=0; i < NUM_LIASONS; i++) {
        name = new char[20];
        sprintf(name, "liaison%d", i);

        l = new Liaison(name);
        liaisons[i] = l;
    }

    // CIS
    // kinda finicky
    // gotta decide how to do this...
    CheckInStaff* cis;
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            name = new char[20];
            sprintf(name, "cis_%d_%d", i, j);

            cis = new CheckInStaff(name, i, j);
            airlines[i]->_cis[j] = cis;
        }
    }

    // Initializing cargo handler threads
    CargoHandler* ch;
    for (int i=0; i < NUM_CARGO_HANDLERS; i++) {
        name = new char[20];
        sprintf(name, "cargo_handler%d", i);

        ch = new CargoHandler(name);
        cargohandlers[i] = ch;
    }

    // Initializing screening officer threads
    ScreeningOfficer* so;
    for (int i=0; i < NUM_SCREENING_OFFICERS; i++) {
        name = new char[25];
        sprintf(name, "screening_officer%d", i);

		so = new ScreeningOfficer(name, i);
		screeningofficers[i] = so;
	}

    // Initializing security inspector threads
    SecurityInspector* si;
    for (int i=0; i < NUM_SECURITY_INSPECTORS; i++) {
        name = new char[25];
        sprintf(name, "security_inspector%d", i);

        si = new SecurityInspector(name, i);
        securityinspectors[i] = si;
    }

    // Initializing manager thread
    name = new char[20];
    sprintf(name, "manager");
    manager = new Manager(name);


    //----------------------------------------------
    // END INITIALIZING ALL THREADS
    //----------------------------------------------


    //----------------------------------------------
    // PRINT INFORMATION
    //----------------------------------------------


    // Print airline information
    for (int i=0; i < NUM_AIRLINES; i++) {
        printf("Number of passengers for airline %s = %i\n", airlines[i]->getName(), airlines[i]->_numExpectedPassengers);
    }

    // Print passenger information
    for (int i=0; i < NUM_PASSENGERS; i++) {
        printf("Passenger %s belongs to airline %i\n", passengers[i]->getName(), passengers[i]->_myticket._airline);
        printf("Passenger %s : Number of bags = %i\n", passengers[i]->getName(), passengers[i]->_baggages.size());
        printf("Passenger %s : Weight of bags = ", passengers[i]->getName());
        for (unsigned int j=0; j < passengers[i]->_baggages.size(); j++) {
            printf("%i ", passengers[i]->_baggages.at(j)->_weight);
        }
        printf("\n");
    }

    // Print cis information
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            printf("Airline check-in staff %s belongs to airline %s\n", airlines[i]->_cis[j]->getName(), airlines[i]->getName());
        }
    }

    
    //----------------------------------------------
    // END PRINT INFORMATION
    //----------------------------------------------

    //----------------------------------------------
    // ACTIVATING ALL THREADS
    //----------------------------------------------

    // Activating all threads
    for (int i=0; i < NUM_PASSENGERS; i++) {
        passengers[i]->Fork((VoidFunctionPtr)PassengerStart, i);
    }
    for (int i=0; i < NUM_LIASONS; i++) {
        liaisons[i]->Fork((VoidFunctionPtr)LiaisonStart, i);
    }
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            airlines[i]->_cis[j]->Fork((VoidFunctionPtr)CisStart, i*NUM_CIS_PER_AIRLINE+j);
        }
    }
    for (int i=0; i < NUM_CARGO_HANDLERS; i++) {
        cargohandlers[i]->Fork((VoidFunctionPtr)CargoHandlerStart, i);
    }
    for (int i=0; i < NUM_SCREENING_OFFICERS; i++) {
        screeningofficers[i]->Fork((VoidFunctionPtr)ScreeningOfficerStart, i);
    }
    for (int i=0; i < NUM_SECURITY_INSPECTORS; i++) {
        securityinspectors[i]->Fork((VoidFunctionPtr)SecurityInspectorStart, i);
    }
    manager->Fork((VoidFunctionPtr)ManagerStart, 0);

    //----------------------------------------------
    // END ACTIVATING ALL THREADS
    //----------------------------------------------

}
void TEST1() {
    
    NUM_PASSENGERS = 2;
    NUM_LIASONS = 2;
    NUM_AIRLINES = 3;

    // Create 4 Passengers for test

    passengers = new Passenger*[NUM_PASSENGERS];
    liaisons = new Liaison*[NUM_LIASONS];

    Passenger* p1;
    char * testName;
    
    // Initialize locks
    LiaisonGlobalLineLock = new Lock("liason_global_line_lock");

    for (int i = 0; i < 2; i++) {
        testName = new char[20];
        sprintf(testName, "TPassenger%d", i);
        
        p1 = new Passenger(testName, i);
        passengers[i] = p1;
    }

    // Create two Liaisons and assign two passengers to one Liaison
    Liaison* l1;
    
    for (int i = 0; i < 2; i++) {
        testName = new char[20];
        sprintf(testName, "TLiaison%d", i);

        l1 = new Liaison(testName);
        liaisons[i] = l1;
    }

    liaisons[0]->_lineSize = 2;
    passengers[0]->Fork((VoidFunctionPtr)PassengerStart, 0);
    passengers[1]->Fork((VoidFunctionPtr)PassengerStart, 1);
    
    liaisons[0]->Fork((VoidFunctionPtr)LiaisonStart, 0);
    liaisons[1]->Fork((VoidFunctionPtr)LiaisonStart, 1);
    std::cout<<"ending"<<std::endl;
}

void TEST2() {

    //initialize locks
    LiaisonGlobalLineLock = new Lock("liason_global_line_lock");
    
    //use fixed number so that we can test the implementation
    NUM_PASSENGERS = 1;
    NUM_LIASONS = 1;
    NUM_AIRLINES = 3;
    NUM_CIS_PER_AIRLINE = 1;

    passengers = new Passenger*[NUM_PASSENGERS];
    liaisons = new Liaison*[NUM_LIASONS];
    airlines = new Airline*[NUM_AIRLINES];
    
    Passenger* p1;
    char * testName;
    

    for (int i = 0; i < 3; i++) {
        testName = new char[20];
        sprintf(testName, "TPassenger%d", 1);
        
        p1 = new Passenger(testName, i);
        passengers[i] = p1;
    }


    Liaison* l1;
    testName = new char[20];
    sprintf(testName, "TLiaison%d", 0);
    l1 = new Liaison(testName);
    liaisons[0] = l1;
    

    for (int i=0; i < NUM_AIRLINES; i++) {
        testName = new char[20];
        sprintf(testName, "TAirline%d", i);
        airlines[i] = new Airline(testName);
    }

    CheckInStaff* cis;
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            testName = new char[20];
            sprintf(testName, "TCis_%d_%d", i, j);

            cis = new CheckInStaff(testName, i, j);
            airlines[i]->_cis[j] = cis;
        }
    }
    //starting simulation
    for (int i = 0; i < 1; i++) {
        passengers[i]->Fork((VoidFunctionPtr)PassengerStart, i);
    }
        liaisons[0]->Fork((VoidFunctionPtr)LiaisonStart, 0);
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            airlines[i]->_cis[j]->Fork((VoidFunctionPtr)CisStart, i*NUM_CIS_PER_AIRLINE+j);
        }
    }
}
void TEST3() {
    NUM_PASSENGERS = 2;
    NUM_LIASONS = 2;
    NUM_AIRLINES = 3;

    // Create 4 Passengers for test

    passengers = new Passenger*[NUM_PASSENGERS];
    liaisons = new Liaison*[NUM_LIASONS];

    Passenger* p1;
    char * testName;
    
    // Initialize locks
    LiaisonGlobalLineLock = new Lock("liason_global_line_lock");

    for (int i = 0; i < 2; i++) {
        testName = new char[20];
        sprintf(testName, "TPassenger%d", i);
        
        p1 = new Passenger(testName, i);
        passengers[i] = p1;
    }

    // Create two Liaisons and assign two passengers to one Liaison
    Liaison* l1;
    
    for (int i = 0; i < 2; i++) {
        testName = new char[20];
        sprintf(testName, "TLiaison%d", i);

        l1 = new Liaison(testName);
        liaisons[i] = l1;
    }

    liaisons[0]->_lineSize = 2;
    passengers[0]->Fork((VoidFunctionPtr)PassengerStart, 0);
    passengers[1]->Fork((VoidFunctionPtr)PassengerStart, 1);
    
    liaisons[0]->Fork((VoidFunctionPtr)LiaisonStart, 0);
    liaisons[1]->Fork((VoidFunctionPtr)LiaisonStart, 1);
    std::cout<<"ending"<<std::endl;
}
void TEST4() {
    //initialize locks
    LiaisonGlobalLineLock = new Lock("liason_global_line_lock");
    
    //use fixed number so that we can test the implementation
    NUM_PASSENGERS = 6;
    NUM_LIASONS = 1;
    NUM_AIRLINES = 1;
    NUM_CIS_PER_AIRLINE = 5;

    passengers = new Passenger*[NUM_PASSENGERS];
    liaisons = new Liaison*[NUM_LIASONS];
    airlines = new Airline*[NUM_AIRLINES];

    semaExe1 = true;
    //initialize passengers, airport, cis, liaison for testing!
    Passenger* p1;
    char * testName;
    std::cout<<NUM_AIRLINES<<std::endl;
    
    for (int i = 0; i < NUM_PASSENGERS; i++) {
        testName = new char[20];
        sprintf(testName, "TPassenger%d", i);

        //for testing purposes, manually set the number of executive class passengers and economy passesngers
        p1 = new Passenger(testName, i);
        if (i < 5) {
            p1->_myticket._executive = true;
        }else {
            p1->_myticket._executive = false;
        }
        passengers[i] = p1;
    }

    Liaison* l1;
    testName = new char[20];
    sprintf(testName, "TLiaison%d", 0);
    l1 = new Liaison(testName);
    liaisons[0] = l1;
    
    for (int i=0; i < NUM_AIRLINES; i++) {
        testName = new char[20];
        sprintf(testName, "TAirline%d", i);
        airlines[i] = new Airline(testName);
    }
    CheckInStaff* cis;
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            testName = new char[20];
            sprintf(testName, "TCis_%d_%d", i, j);

            cis = new CheckInStaff(testName, i, j);
            airlines[i]->_cis[j] = cis;
        }
    }
    //starting fork every element so we can start test simulation
    for (int i = 0; i < NUM_PASSENGERS; i++) {
        passengers[i]->Fork((VoidFunctionPtr)PassengerStart, i);
    }
        liaisons[0]->Fork((VoidFunctionPtr)LiaisonStart, 0);
    //waiting for passengers so that they can be standing and waiting in CIS line.
    for (int i = 0; i < 6; i++)
            t4_1.P();

    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            airlines[i]->_cis[j]->Fork((VoidFunctionPtr)CisStart, i*NUM_CIS_PER_AIRLINE+j);
        }
    }
}
void TEST5() {

    //initialize locks
    LiaisonGlobalLineLock = new Lock("liason_global_line_lock");
    
    //use fixed number so that we can test the implementation
    NUM_PASSENGERS = 1;
    NUM_LIASONS = 1;
    NUM_AIRLINES = 3;
    NUM_CIS_PER_AIRLINE = 1;

    passengers = new Passenger*[NUM_PASSENGERS];
    liaisons = new Liaison*[NUM_LIASONS];
    airlines = new Airline*[NUM_AIRLINES];
    
    Passenger* p1;
    char * testName;
    

    for (int i = 0; i < 3; i++) {
        testName = new char[20];
        sprintf(testName, "TPassenger%d", 1);
        
        p1 = new Passenger(testName, i);
        passengers[i] = p1;
    }


    Liaison* l1;
    testName = new char[20];
    sprintf(testName, "TLiaison%d", 0);
    l1 = new Liaison(testName);
    liaisons[0] = l1;
    

    for (int i=0; i < NUM_AIRLINES; i++) {
        testName = new char[20];
        sprintf(testName, "TAirline%d", i);
        airlines[i] = new Airline(testName);
    }

    CheckInStaff* cis;
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            testName = new char[20];
            sprintf(testName, "TCis_%d_%d", i, j);

            cis = new CheckInStaff(testName, i, j);
            airlines[i]->_cis[j] = cis;
        }
    }
    //starting simulation
    for (int i = 0; i < 1; i++) {
        passengers[i]->Fork((VoidFunctionPtr)PassengerStart, i);
    }
        liaisons[0]->Fork((VoidFunctionPtr)LiaisonStart, 0);
    for (int i=0; i < NUM_AIRLINES; i++) {
        for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
            airlines[i]->_cis[j]->Fork((VoidFunctionPtr)CisStart, i*NUM_CIS_PER_AIRLINE+j);
        }
    }
}
void AirTest() {

    while(true) {
        int i;
        std::cout<<"Select TESTING Menu"<<std::endl;
        std::cout<<"1.  Test 1  : Passenger selects the shortest line for the aiport liaison"<<std::endl;
        std::cout<<"2.  Test 2  : Passenger is directed by the Liaison to the correct airline counters"<<std::endl;
        std::cout<<"3.  Test 3  : Economy class passengers enter the shortest line while Executive class passengers enter their correct line"<<std::endl;
        std::cout<<"4.  Test 4  : Executive class passengers are given priority over the economy class passengers at the check-in kiosks"<<std::endl;
        std::cout<<"5.  Test 5  : Screening officer chooses an available security inspector each time a passenger comes in."<<std::endl;
        std::cout<<"6.  Test 6  : Cargo handlers choose bags from the conveyor system each time and go on a break if there are no bags."<<std::endl;
        std::cout<<"7.  Test 7  : Handing over of the hand luggage by the passenger to the screening officer."<<std::endl;
        std::cout<<"8.  Test 8  : Passenger returns to the same security inspector after further questioning."<<std::endl;
        std::cout<<"9.  Test 9  : Baggage weights of all the passengers of a particular airline should match the weights of the bags reported by the cargo handlers."<<std::endl;
        std::cout<<"10. Test 10 : Handing over of boarding pass by the passenger to the security inspector"<<std::endl;
        std::cout<<"11. QUIT "<<std::endl;

        std::cin>>i;
        if(i == 1) {
            std::cout<<"TESTING 1"<<std::endl;
            semaBool = true;
            stopSIM1 = true;
            // Passenger selects the shortest line for the airport liaison
            printf("Test1 : Passenger selects the shortest line for the airport liaison\n");

            //executes TEST 1 simulation!
            Thread * t = new Thread("TEST1");
            t->Fork((VoidFunctionPtr)TEST1,0);

            //First two Passengers start going Liaison
            //the for loop is waiting for the testing simulation done so that there is correct result/outcome
            for (int i=0;i<4;i++) {
                t1.P();
            }
                
            if(liaisons[1]->_totalNumber == 2) {
                printf("Test 1 passed!\n");
            }else {
                printf("Test 1 failed\n");
            }
            printf("Test1 over!\n");
            //going back to initial case so that we can start over in initial condition
            semaBool = false;
            stopSIM1 = false;
            NUM_PASSENGERS = 0;
            NUM_LIASONS = 0;
            liaisons[0]->_lineSize = 0;
            Semaphore t1("t1",0);
            continue;
        }else if(i == 2) {
            std::cout<<"TESTING 2"<<std::endl;
            semaBool = true;
            stopSIM1 = true;
            printf("Test2 : Passenger is directed by the Liaison to the correct airline counters\n");

            //executes TEST 2 simulation
            Thread * t = new Thread("TEST2");
            t->Fork((VoidFunctionPtr)TEST2,0);

            //waiting test simulation before getting results and compare the results 
            for (int i = 0;i < 2; i++)
                t1.P();

            int temp2 = 0;
            for (int i=0; i < NUM_AIRLINES; i++) {
                int temp = 0;
                for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
                    temp += airlines[i]->_cis[j]->_passCount;
                }
                if(temp == 1) {
                    temp2 = i;
                }
            }
            if(passengers[0]->_myticket._airline == temp2) {
                printf("Test 2 passed!\n");
            }else {
                printf("Test 2 failed\n");
            }
            //going back to initial case so that we can start over in initial condition
            semaBool = false;
            stopSIM1 = false;
            NUM_PASSENGERS = 0;
            NUM_LIASONS = 0;
            NUM_AIRLINES = 0;
            NUM_CIS_PER_AIRLINE = 0;
            Semaphore t1("t1",0);
            temp2 = 0;
            continue;
        }else if(i == 3) {
          std::cout<<"TESTING 3"<<std::endl;
            semaBool = true;
            stopSIM1 = true;
            // Passenger selects the shortest line for the airport liaison
            printf("Test3 : Economy class passengers enter the shortest line while Executive class passengers enter their correct line\n");

            //executes TEST 1 simulation!
            Thread * t = new Thread("TEST3");
            t->Fork((VoidFunctionPtr)TEST3,0);

            //First two Passengers start going Liaison
            //the for loop is waiting for the testing simulation done so that there is correct result/outcome
            for (int i=0;i<4;i++) {
                t1.P();
            }
                
            if(liaisons[1]->_totalNumber == 2) {
                printf("Test 3 passed!\n");
            }else {
                printf("Test 3 failed\n");
            }
            //going back to initial case so that we can start over in initial condition
            semaBool = false;
            stopSIM1 = false;
            NUM_PASSENGERS = 0;
            NUM_LIASONS = 0;
            liaisons[0]->_lineSize = 0;
            Semaphore t1("t1",0);
            continue;
        }else if(i == 4) {
            std::cout<<"TESTING 4"<<std::endl;
            semaExe = true; 
            printf("Test4 : Executive class passengers are given priority over the economy class passengers at the check-in kiosks\n");

            //executes TEST 1 simulation!
            Thread * t = new Thread("TEST4");
            t->Fork((VoidFunctionPtr)TEST4,0);

            //waiting test simulation before getting results and compare the results
            for (int i = 0; i < 5; i++) {
                t4.P();
            }

            //if there is a passenger waiting in economy line, then test passes
            int total_number_passenger = 0;
            for (int i=0; i < NUM_AIRLINES; i++) {
                for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
                    total_number_passenger += airlines[i]->_cis[j]->_PassengerNumber;
                }
            }
            if(total_number_passenger == 5) {
                printf("Test 4 passes!\n");
            }else{
                printf("Test 4 failed!\n");
            }
            //going back to initial case so that we can start over in initial condition
            NUM_PASSENGERS = 0;
            NUM_LIASONS = 0;
            NUM_AIRLINES = 0;
            NUM_CIS_PER_AIRLINE = 0;
            semaExe = false;
            semaExe1 = false; 
            Semaphore t4("t4",0);
            Semaphore t1("t1",0);
            Semaphore t4_1("t4_1",0);     
            continue;
        }else if(i == 5) {
            std::cout<<"TESTING 5"<<std::endl;
            semaBool = true;
            stopSIM1 = true;
            printf("Test5 : Screening officer chooses an available security inspector each time a passenger comes in.\n");

            //executes TEST 5 simulation
            Thread * t = new Thread("TEST5");
            t->Fork((VoidFunctionPtr)TEST5,0);

            //waiting test simulation before getting results and compare the results 
            for (int i = 0;i < 2; i++)
                t1.P();

            int temp2 = 0;
            for (int i=0; i < NUM_AIRLINES; i++) {
                int temp = 0;
                for (int j=0; j < NUM_CIS_PER_AIRLINE; j++) {
                    temp += airlines[i]->_cis[j]->_passCount;
                }
                if(temp == 1) {
                    temp2 = i;
                }
            }
            if(passengers[0]->_myticket._airline == temp2) {
                printf("Test 5 passed!\n");
            }else {
                printf("Test 5 failed\n");
            }
            //going back to initial case so that we can start over in initial condition
            semaBool = false;
            stopSIM1 = false;
            NUM_PASSENGERS = 0;
            NUM_LIASONS = 0;
            NUM_AIRLINES = 0;
            NUM_CIS_PER_AIRLINE = 0;
            Semaphore t1("t1",0);
            temp2 = 0;
            continue;
        }else if(i == 6) {
            std::cout<<"TESTING 6"<<std::endl;  
            //put function  
            continue;
        }else if(i == 7) {
            std::cout<<"TESTING 7"<<std::endl;  
            //put function  
            continue;
        }else if(i == 8) {
            std::cout<<"TESTING 8"<<std::endl;  
            //put function  
            continue;
        }else if(i == 9) {
            std::cout<<"TESTING 9"<<std::endl;
            //put function      
            continue;
        }else if(i == 10) {
            std::cout<<"TESTING 10"<<std::endl; 
            //put function  
            continue;
        }else if(i == 11) {
            std::cout<<"QUIT"<<std::endl;   
            break;
        }else{
            std::cout<<"You select wrong number try again."<<std::endl;
            continue;
        }
    }   



}

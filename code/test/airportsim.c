/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */
#include "syscall.h"

#define NULL 0

#define NUM_PASSENGERS 10
#define	NUM_LIASONS 5
#define	NUM_AIRLINES 3
#define	NUM_CIS_PER_AIRLINE 7
#define	NUM_CARGO_HANDLERS 9
#define	NUM_SCREENING_OFFICERS 8
#define	NUM_SECURITY_INSPECTORS 8

typedef int bool;
enum bool {false, true};

/* States used by various employees */
enum State {
    AVAIL,
    BUSY,
    ONBREAK
};

/*
	Structs
*/

typedef struct {
    int _array[NUM_PASSENGERS*3];
    int _rear;
    int _front;
} Queue;

typedef struct {
    bool _executive;
    int _airline; 
    int _seat; 
} Ticket;

typedef struct {
    int _airline; 
    int _weight; 
} Baggage;

/* Passenger */
typedef struct {
 	int _id;   
 	int _inspectorID;
 	int _officerID;
 	int _liaisonID;
 	int _cisID;
 	bool _furtherQuestioning;
 	int _numBaggages;
 	Ticket _ticket;
} Passenger;

/* Liaison */
typedef struct {
	int _lock;
    int _lineCV;
    int _commCV;
    int _lineSize;
    int _state; 
    int _passCount[NUM_AIRLINES];
    int _bagCount[NUM_AIRLINES];
    int _currentPassenger;
} Liaison;

/* Check-in Staff */
typedef struct {
    int _lock;
    int _lineCV;
    int _commCV;
    int _lineSize;
    int _state;
    int _passCount;
    int _bagCount;
    int _weightCount;
    int _currentPassenger;
    int _airline;
    bool _done;
} CIS;

/* ScreeningOfficer */
typedef struct {
    int _lock;
    int _commCV;
    int _state; 
    int _passCount;
    int _currentPassenger;
    int _myNum;
    bool _done;
} ScreeningOfficer;

/* Security Inspector */
typedef struct {
	int _id;
    int _rtnPassSize;
    int _state;
    int _lock;
    int _commCV;
    int _rtnPassCV;
    int _newPassCV;
    int _rtnPassenger;
    int _newPassenger;
    int _passCount;
} SecurityInspector;

/* Manager */
typedef struct {
	bool _allCISDone;
    bool _allCargoDone;
    bool _allSODone;
    bool _allSIDone;
} ManagerStruct;

/* Cargo Handler */
typedef struct {
    int _commCV;
    int _state;
    int _bagCount[NUM_AIRLINES];
    int _weightCount[NUM_AIRLINES];
} CargoHandler;

/* Airline */
typedef struct {
    int _lock; 
    int _execLineLock;
    int _execLineCV;
    Queue _execQueue;
    int _boardLoungeCV;
    int _cisLineLock;
    CIS _cis[NUM_CIS_PER_AIRLINE];
  	/* Stats */
    int _numExpectedPassengers;
    int _numCheckedinPassengers;
    int _numReadyPassengers;
    int _numExpectedBaggages;
    int _numLoadedBaggages;
    int _bagCount;
    int _weightCount;
    /* Shutdown */
    int _numOnBreakCIS;
    bool _boarded;
    bool _allPassengersCheckedIn;
    bool _CISclosed;
} Airline;

/*
	Global Data
*/
/* Airport Entities */
Passenger Passengers[NUM_PASSENGERS];
Liaison Liaisons[NUM_LIASONS];
Airline Airlines[NUM_AIRLINES];
ManagerStruct Manager;
Baggage Baggages[NUM_PASSENGERS * 3];
CargoHandler CargoHandlers[NUM_CARGO_HANDLERS];
ScreeningOfficer ScreeningOfficers[NUM_SCREENING_OFFICERS];
SecurityInspector SecurityInspectors[NUM_SECURITY_INSPECTORS]; 

/* Number of currently active entities */
/* Used strictly for initialization */
int NumActivePassengers; 
int NumActiveLiaisons;
int NumActiveCIS;
int NumActiveCargoHandlers;
int NumActiveScreeningOfficers;
int NumActiveSecurityInspectors;

/* Locks */
int GlobalDataLock; /* Used for initializing */
int LiaisonLineLock;
int ConveyorLock;
int OfficersLineLock;
int InspectorLineLock;

/* CV */
int OfficersLineCV;

/* Queue */
Queue OfficersLine;
Queue ConveyorBelt;

/* Data for Security Personel */
bool SecurityFailResults[NUM_PASSENGERS];

/*
	Utilities	
*/
char concatString[100];
char* concatNumToString(char* str, int length, int num) { /* TODO - Not working Properly */
	int i;
	for (i=0; i < length; i++) {
		concatString[i] = str[i];
	}
	concatString[length] = (char)num;
	return concatString;
}

int concat3Num(int i, int j, int k) {
	return 1000000 * i + 1000 * j + k;
} 

int concat2Num(int i, int j) {
	return 1000 * i + j;
}

/*#define front queue->_front
#define rear queue->_rear*/
void queue_insert (Queue* queue, int index) {
    if (queue->_rear == NUM_PASSENGERS*3-1)
        Printf0("ERROR: QUEUE OVERFLOW\n", sizeof("ERROR: QUEUE OVERFLOW\n"));
    else {
        /* If queue is initially empty */
        if (queue->_front == -1)
            queue->_front = 0;
/*        queue->_array[++rear] = index;*/
        queue->_array[++queue->_rear] = index;
    }
}

int queue_pop (Queue* queue) {
    if (queue->_front == -1 || queue->_front > queue->_rear) {
        Printf0("ERROR: QUEUE IS EMPTY\n", sizeof("ERROR: QUEUE IS EMPTY\n"));
        return -1;
    }
/*    return queue->_array[front++];*/
    return queue->_array[queue->_front++];
}

int queue_size (Queue* queue) {
    if (queue->_front == -1 || queue->_front > queue->_rear) return 0;
/*    return rear - front + 1;*/
    return queue->_rear - queue->_front + 1;
}

bool queue_empty (Queue* queue) {
    if (queue_size (queue) == 0) return true;
    else return false;
}
/*#undef front
#undef reqr*/

/*
	Start Functions - functions called by Fork() syscall.
*/
void startPassenger() {    
#define my Passengers[_myIndex]
#define liaison Liaisons[my._liaisonID]
	/* Claim my Passenger */
	int i, j; /* for-loop iterators */
	int _myIndex; /* ID for currentThread */
	int _minLineSize;

    Acquire(GlobalDataLock);
    _myIndex = NumActivePassengers++;
    Release(GlobalDataLock);

    /*
		Liaison Interaction
    */
	Acquire(LiaisonLineLock);
	_minLineSize = Liaisons[0]._lineSize;
	/* Find shortest line */
	for (i = 1; i < NUM_LIASONS; i++) {
		if (Liaisons[i]._lineSize < _minLineSize) {
			_minLineSize = Liaisons[i]._lineSize;
			my._liaisonID = i;
		}
	}

	Printf1("Passenger %d chose Liaison %d with a line length %d\n", 
		sizeof("Passenger %d chose Liaison %d with a line length %d\n"), 
		concat3Num(_myIndex, my._liaisonID, liaison._lineSize));
	/* Get in line? */
	if (liaison._state == BUSY) {
/*		Printf0("Passenger says liaison is BUSY\n", 
			sizeof("Passenger says liaison is BUSY\n"));
		Printf1("Passenger's liaison id: %d\n", 
			sizeof("Passenger's liaison id: %d\n"),
			_myIndex);*/
		liaison._lineSize++;
		Wait(LiaisonLineLock, liaison._lineCV);
		liaison._lineSize--;
	}
	Printf1("Passenger %d is moving along...\n", sizeof("Passenger %d is moving along...\n"), _myIndex);
	/* Go to Liaison */
	Acquire(liaison._lock);
	Release(LiaisonLineLock);
	/* Give Liaison my Passenger info */
	liaison._passCount[my._ticket._airline]++;
	liaison._bagCount[my._ticket._airline] += my._numBaggages;
	liaison._currentPassenger = _myIndex;
	Signal(liaison._lock, liaison._commCV); /* Signal Liaison */
	Wait(liaison._lock, liaison._commCV); /* Wait for Liaison */
	Printf1("Passenger %d of Airline %d is directed to the check-in counter\n", 
		sizeof("Passenger %d of Airline %d is directed to the check-in counter\n"),
		concat2Num(_myIndex, my._ticket._airline));
	Signal(liaison._lock, liaison._commCV);
	Release(liaison._lock);
#undef liaison
	/* end Liaison Interaction */

	/*
		Check-in Staff Interaction
	*/
#define myAirline Airlines[my._ticket._airline]
#define myCIS myAirline._cis[my._cisID]
	if (my._ticket._executive) {
		Acquire(myAirline._execLineLock);
		queue_insert(&myAirline._execQueue, _myIndex);
		Printf1("Passenger %d of Airline %d is waiting in the executive class line\n", 
			sizeof("Passenger %d of Airline %d is waiting in the executive class line\n"),
			concat2Num(_myIndex, my._ticket._airline));
/*		Wait(myAirline._execLineLock, myAirline._execLineLock); *//* Wait on CIS */
		Wait(myAirline._execLineLock, myAirline._execLineCV); /* Wait on CIS */
	} else { /* Economy */
		Acquire(myAirline._cisLineLock);
		/* Find shortest line */
		_minLineSize = myAirline._cis[0]._lineSize; /* declare at top of startPassenger */
		my._cisID = 0;
		for (i = 0; i < NUM_CIS_PER_AIRLINE; ++i) {
			if (myAirline._cis[i]._lineSize < _minLineSize) {
				_minLineSize = myAirline._cis[i]._lineSize;
				my._cisID = i;
			}
		}
		Printf2("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
			sizeof("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n"),
			concat3Num(_myIndex, my._ticket._airline, my._cisID), _minLineSize);
			myCIS._lineSize++;
/*Printf1("Passenger %d of Airline %d is going to sleep and should be woken up by cis %d\n", sizeof("Passenger %d of Airline %d is going to sleep and should be woken up by cis %d\n"), concat3Num(_myIndex, my._ticket._airline, my._cisID));*/
		Wait(myAirline._cisLineLock, myCIS._lineCV);
/*Printf1("Passenger %d of Airline %d is woken up by cis %d\n", sizeof("Passenger %d of Airline %d is woken up by cis %d\n"), concat3Num(_myIndex, my._ticket._airline, my._cisID));*/
	}
	Acquire(myCIS._lock);
	if (my._ticket._executive) {
		Release(myAirline._execLineLock);
	} else {
		myCIS._lineSize--;
		Release(myAirline._cisLineLock);
	}
	/* Give baggage to CIS */
	myCIS._passCount++;
	myCIS._bagCount += my._numBaggages;
	myCIS._currentPassenger = _myIndex;
	Signal(myCIS._lock, myCIS._commCV); /* Signal CIS */
	Wait(myCIS._lock, myCIS._commCV); /* Wait on CIS */
	Printf1("Passenger %d of Airline %d was informed to board at gate %d\n",
		sizeof("Passenger %d of Airline %d was informed to board at gate %d\n"),
		concat3Num(_myIndex, my._ticket._airline, my._ticket._airline));
	Signal(myCIS._lock, myCIS._commCV); /* Signal CIS */
	Release(myCIS._lock);

#undef myCIS
#undef myAirline
	/* end Check-in Staff Interaction */


	/*
		Screening Officer Interaction
	*/
	Acquire(OfficersLineLock);
	queue_insert(&OfficersLine, _myIndex);
	Wait(OfficersLineLock, OfficersLineCV);
	Printf1("Passenger %d gives the hand-luggage to screening officer %d\n",
		sizeof("Passenger %d gives the hand-luggage to screening officer %d\n"),
		concat2Num(_myIndex, my._officerID));
	Acquire(ScreeningOfficers[my._officerID]._lock);
	Release(OfficersLineLock);
	Signal(ScreeningOfficers[my._officerID]._lock, ScreeningOfficers[my._officerID]._commCV);
	Wait(ScreeningOfficers[my._officerID]._lock, ScreeningOfficers[my._officerID]._commCV);
	/* officer lock is released below! */
	/* end Screening Officer Interaction */

	/*
		Security Inspector Interaction
	*/
	#define inspector SecurityInspectors[my._inspectorID]
	Printf1("Passenger %d moves to security inspector %d\n",
		sizeof("Passenger %d moves to security inspector %d\n"),
		concat2Num(_myIndex, my._inspectorID));
	Acquire(inspector._lock);
	Release(ScreeningOfficers[my._officerID]._lock);
	inspector._newPassenger = _myIndex;
	if (inspector._state == AVAIL) { /* Wake up inspector */
		Signal(inspector._lock, inspector._commCV);
	}
	Wait(inspector._lock, inspector._newPassCV); /* Wait for security results */
	if (my._furtherQuestioning) {
		Printf1("Passenger %d goes for futher questioning\n", 
			sizeof("Passenger %d goes for futher questioning\n"),
			_myIndex);
		Release(inspector._lock);
		for (i = 0; i < 10; ++i) {
			Yield(); /* Simulate Further questioning */
		}
		Printf1("Passenger %d comes back to security inspector %d after further examination\n",
			sizeof("Passenger %d comes back to security inspector %d after further examination\n"),
			concat2Num(_myIndex, my._inspectorID));
		Acquire(inspector._lock);
		inspector._rtnPassenger++;
		if (inspector._state == AVAIL) {
			Signal(inspector._lock, inspector._commCV);
		}
		Wait(inspector._lock, inspector._rtnPassCV);
		inspector._rtnPassenger = _myIndex;
		Signal(inspector._lock, inspector._rtnPassCV);
		Wait(inspector._lock, inspector._rtnPassCV);
	}	
	Release(inspector._lock);

	#undef inspector
	/* end Security Inspector Interaction */

	/*
		Reached the Boarding Lounge
	*/
	#define myAirline Airlines[my._ticket._airline]

	Acquire(myAirline._lock);
	myAirline._numReadyPassengers++;
	Printf1("Passenger %d of Airline %d reached the gate %d\n",
		sizeof("Passenger %d of Airline %d reached the gate %d\n"),
		concat3Num(_myIndex, my._ticket._airline, my._ticket._airline));
	Wait(myAirline._lock, myAirline._boardLoungeCV); /* Wait for boarding call by manager */
	Printf1("Passenger %d of Airline %d boarded airline %d\n",
		sizeof("Passenger %d of Airline %d boarded airline %d\n"),
		concat3Num(_myIndex, my._ticket._airline, my._ticket._airline));
	Release(myAirline._lock);

	#undef myAirline
	/* End Boarding Lounge */

	Exit(0);
#undef my
}

void startLiaison() {
#define l Liaisons[_myIndex]
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
    Acquire(GlobalDataLock);
    _myIndex = NumActiveLiaisons++;
    Release(GlobalDataLock);

	while(true) {
		/*
			Passenger Interaction
    	*/
	    Acquire(LiaisonLineLock);

	    Acquire(l._lock);
	    if (l._lineSize == 0) {
	    	Release(LiaisonLineLock);
	    	l._state = AVAIL;
	    	Wait(l._lock, l._commCV); /* Wait for new Passenger */
	    } else {
	    	Signal(LiaisonLineLock, l._lineCV); /* Signal Passenger */
	    	Release(LiaisonLineLock); 
	    	Wait(l._lock, l._commCV); /* Wait on Passenger */
	    }
	    l._state = BUSY;
	    Printf1("Airport Liaison %d directed passenger %d of airline %d\n",
	    	sizeof("Airport Liaison %d directed passenger %d of airline %d\n"),
	    	concat3Num(_myIndex, l._currentPassenger, Passengers[l._currentPassenger]._ticket._airline));
	    Signal(l._lock, l._commCV); /* Signal Passenger */
	    Wait(l._lock, l._commCV); /* Wait for Passenger to say goodbye */
	    Release(l._lock);
	    /* end Passenger Interaction */
	}

	Exit(0);
#undef l
}

void startCheckInStaff() {
#define myAirline Airlines[_myAirline]
#define my myAirline._cis[_myIndex]
#define passenger Passengers[my._currentPassenger]
	/* Claim my CIS */
	int _myAirline;
	int _myIndex; /* ID for currentThread */
    Acquire(GlobalDataLock);
	_myIndex = NumActiveCIS % NUM_CIS_PER_AIRLINE;
	_myAirline = NumActiveCIS / NUM_CIS_PER_AIRLINE;
    NumActiveCIS++;
    Release(GlobalDataLock);

    while (true) {
		/* Check lines */
		Acquire(myAirline._lock);
		if (my._lineSize == 0 && queue_empty(&myAirline._execQueue)) {
			/*Printf0("1\n", sizeof("1\n"));*/
			my._state = ONBREAK;
			/* 'Clock Out' for Break */
			myAirline._numOnBreakCIS++;
Printf1("Cis %d going to sleep\n", sizeof("Cis %d going to sleep\n"), _myIndex);
			Wait(myAirline._lock, my._commCV); /* Wait on Manager */ /* TODO - make sure okay to wait on aiport lock... maybe better? */
Printf1("Cis %d woke up by manager\n", sizeof("Cis %d woke up by manager\n"), _myIndex);
			/* Time to go home! TGIF! */
			if (my._done) {
				Printf1("Airline check-in staff %d of airline %d is closing the counter\n",
					sizeof("Airline check-in staff %d of airline %d is closing the counter\n"),
					concat2Num(_myIndex, _myAirline));
/*				Acquire(my._lock);*/
				Release(myAirline._lock);
/*				Wait(my._lock, my._commCV); *//* Wait forever, basically */
/*				Release(my._lock); *//* Never reaches here, but whatever... */
				Exit(0);
			}
			myAirline._numOnBreakCIS--;
		}
		/* Start helping a passenger */
		my._state = BUSY;
		Acquire(myAirline._cisLineLock);
		Acquire(myAirline._execLineLock);
		if (queue_size(&myAirline._execQueue) > 0) {
			my._currentPassenger = queue_pop(&myAirline._execQueue);
			passenger._cisID = _myIndex;
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concat3Num(_myIndex, _myAirline, my._lineSize));
			Signal(myAirline._execLineLock, myAirline._execLineCV); /* Signal Passenger */
		} else if (my._lineSize > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concat3Num(_myIndex, _myAirline, queue_size(&myAirline._execQueue)));
			Signal(myAirline._cisLineLock, my._lineCV);
/*Printf1("Cis %d of airline %d wakes up passenger\n", sizeof("Cis %d of airline %d wakes up passenger\n"), _myIndex*1000+_myAirline);*/
		}
		/* Interact with Passenger */
		Acquire(my._lock);
		Release(myAirline._cisLineLock);
		Release(myAirline._execLineLock);
		Release(myAirline._lock);
		if (my._lineSize > 0 || my._currentPassenger != -1) {
/*Printf1("Cis %d of airline %d goes to sleep\n", sizeof("Cis %d of airline %d goes to sleep\n"), _myIndex*1000+_myAirline);*/
			Wait(my._lock, my._commCV); 
		} /* Otherwise, Manager woke you up for no reason */
		if (my._currentPassenger != -1) {
			int i;
			/* Assign seat number */
			Acquire(myAirline._lock);
			passenger._ticket._seat = myAirline._numCheckedinPassengers;
			myAirline._numCheckedinPassengers++;
			Release(myAirline._lock);
			/* Deal with baggage */
			Acquire(ConveyorLock);
			for (i = 0; i < passenger._numBaggages; ++i) {
				/*Printf0("3\n", sizeof("1\n"));*/
/*				#define bIndex (passenger._id * 3) + i
				#define bag Baggages[bIndex]*/
				#define bag Baggages[(passenger._id*3)+i]
				bag._airline = _myAirline; /* Tag the bag */
/*				queue_insert(&ConveyorBelt, bIndex);*/
				queue_insert(&ConveyorBelt, passenger._id*3+i);
				Printf1("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n",
					sizeof("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n"),
					concat2Num(_myIndex, _myAirline));
				myAirline._numExpectedBaggages++;
				my._weightCount += bag._weight;
				#undef bag
/*				#undef bIndex*/
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (passenger._ticket._executive) {
				Printf2("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, my._currentPassenger, _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, my._currentPassenger), _myAirline);
			} else {
				Printf2("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, my._currentPassenger, _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, my._currentPassenger), _myAirline);
			}
			Signal(my._lock, my._commCV); 
			Wait(my._lock, my._commCV); 
		}
		my._currentPassenger = -1;
		Release(my._lock);
	} /* end while */
	Exit(0);
#undef passenger
#undef my
#undef myAirline
}

void startCargoHandler() {
#define my CargoHandlers[_myIndex]
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
	int bIndex;
    Acquire(GlobalDataLock);
    _myIndex = NumActiveCargoHandlers++;
    Release(GlobalDataLock);
	
	while (true) {
		#define bag Baggages[bIndex]
		Acquire(ConveyorLock);
		if (queue_empty(&ConveyorBelt)) {
			my._state = ONBREAK;
Printf1("CargoHandler %d going to sleep\n", sizeof("CargoHandler %d going to sleep\n"), _myIndex);
			Wait(ConveyorLock, my._commCV);
Printf1("CargoHandler %d woke up by manager\n", sizeof("CargoHandler %d woke up by manager\n"), _myIndex);
			Printf1("Cargo Handler %d returned from break\n",
				sizeof("Cargo Handler %d returned from break\n"),
				_myIndex);
		} else {
			bIndex = queue_pop(&ConveyorBelt);
			Printf1("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n",
				sizeof("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n"),
				concat3Num(_myIndex, bag._airline, bag._weight));
			Airlines[bag._airline]._numLoadedBaggages++;
			my._bagCount[bag._airline]++;
			my._weightCount[bag._airline]++;
		}
		Release(ConveyorLock);	
		Yield();
		#undef bag	
	}
	Exit(0);
#undef my
}

void startScreeningOfficer() {
	#define mySO ScreeningOfficers[_myIndex]
	int _myIndex;
	/* Claim my screening officer */
	Acquire(GlobalDataLock);
    _myIndex = NumActiveScreeningOfficers++;
    Release(GlobalDataLock);

    /* Get to work! */
	while (true) {
		int suspicionLevel = false;
		int shortestLineIndex = -1; /* impossible value */
		int i; /* iterator */
		Acquire(OfficersLineLock);
		if (queue_empty(&OfficersLine)) {
			mySO._state = ONBREAK;
			Wait(OfficersLineLock, mySO._commCV); /* Wait for a passenger */
		}
		if (!queue_empty(&OfficersLine)) {
			Acquire(mySO._lock);
			mySO._state = BUSY;
			mySO._currentPassenger = queue_pop(&OfficersLine);
			Passengers[mySO._currentPassenger]._officerID = _myIndex;
			Signal(OfficersLineLock, OfficersLineCV); /* Wake up passenger */
			Release(OfficersLineLock);
			Wait(mySO._lock, mySO._commCV); /* Wait for passenger to approach */
			/* Generate PASS/FAIL Results */
			suspicionLevel = (17 * mySO._currentPassenger) % 10; /* PSUEDO rand() */
			SecurityFailResults[mySO._currentPassenger] = suspicionLevel > 8;
			if (SecurityFailResults[mySO._currentPassenger]) { /* FAIL */
				Printf1("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
					sizeof("Screening officer %d is suspicious of the hand luggage of passenger %d\n"),
					concat2Num(_myIndex, mySO._currentPassenger));
			} else { /* PASS */
				Printf1("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
					sizeof("Screening officer %d is not suspicious of the hand luggage of passenger %d\n"),
					concat2Num(_myIndex, mySO._currentPassenger));
			}
			/* Find an Available Security Inspector */
			while (shortestLineIndex == -1) {
				#define inspector SecurityInspectors[i]
				Acquire(InspectorLineLock);	
				for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
					Acquire(inspector._lock);
					if (inspector._state == AVAIL && inspector._newPassenger == -1) {
						/* Found an inspector! */
						shortestLineIndex = inspector._id;
						inspector._newPassenger = mySO._currentPassenger;
						Release(inspector._lock);
						break;
					}
					Release(inspector._lock);
				}
				if (shortestLineIndex == -1) {
					Yield();
				}
				Release(InspectorLineLock);	
				#undef inspector
				Passengers[mySO._currentPassenger]._inspectorID = shortestLineIndex;
				Printf1("Screening officer %d directs passenger %d to security inspector %d\n",
					sizeof("Screening officer %d directs passenger %d to security inspector %d\n"),
					concat3Num(_myIndex, mySO._currentPassenger, shortestLineIndex));
				Signal(mySO._lock, mySO._commCV);
				Wait(mySO._lock, mySO._commCV);
			}
			/* Found an inspetor for the passenger */
		} else {
			Release(OfficersLineLock);
		}
	}
	Exit(0);
}

void startSecurityInspector() {
	#define mySI SecurityInspectors[_myIndex]
	int _myIndex;
	/* Claim my security insepctor */
	Acquire(GlobalDataLock);
    _myIndex = NumActiveScreeningOfficers++;
    Release(GlobalDataLock);

    /* Start work */
    while (true) {
    	bool suspicious, guilty;
    	Acquire(mySI._lock);
    	if (mySI._rtnPassSize == 0 && mySI._newPassenger == -1) {
    		mySI._state = AVAIL;
    		Wait(mySI._lock, mySI._commCV); /* Wait for Passenger to come */
    		mySI._state = BUSY;
    	}
    	if (mySI._rtnPassSize > 0) { /* priority to returning passengers */
    		while (mySI._rtnPassSize > 0) {
    			Signal(mySI._lock, mySI._rtnPassCV); /* Wake up passener */
    			Wait(mySI._lock, mySI._rtnPassCV); /* Wait on passenger */
    			Printf1("Security inspector %d permits returning passenger %d to board\n",
    				sizeof("Security inspector %d permits returning passenger %d to board\n"),
    				concat2Num(_myIndex, mySI._rtnPassenger));
    			mySI._passCount++;
    			mySI._rtnPassenger--;
    			Signal(mySI._lock, mySI._rtnPassCV);
    		}
    		mySI._rtnPassenger = -1;
    	}
    	if (mySI._newPassenger != -1) { /* I have a new passenger to help */ 
    		suspicious = (mySI._newPassenger * 23) % 10 > 7; /* Psuedo Random */
    		guilty = suspicious || SecurityFailResults[mySI._newPassenger];
    		if (suspicious) {
    			Printf1("Security Inspector %d is suspicious of the hand luggage of passenger %d\n",
    				sizeof("Security Inspector %d is suspicious of the hand luggage of passenger %d\n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    		} else {
    			Printf1("Security Inspector %d is not suspicious of the hand luggage of passenger %d\n",
    				sizeof("Security Inspector %d is not suspicious of the hand luggage of passenger %d\n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    		}
    		if (guilty) {
    			Printf1("Security inspector %d asks passenger %d to go for further examination\n", 
    				sizeof("Security inspector %d asks passenger %d to go for further examination\n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    		} else {
    			Printf1("Security inspector %d allows passenger %d to board \n",
    				sizeof("Security inspector %d allows passenger %d to board \n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    			mySI._passCount++;
    		}
    		mySI._newPassenger = -1;
    		Signal(mySI._lock, mySI._newPassCV);
    	}
    	Release(mySI._lock);
    }
    #undef mySI
	Exit(0);
}

void startManager() {
#define cis Airlines[i]._cis[j]
	int i, j; /* for-loop iterator */
	while (true) {
		bool allFlightsBoarded = false;
		int numReadyAirlines = 0;
		/*
			Check-in Staff 
		*/
		if (!Manager._allCISDone) {
			int numDoneAirline = 0;
			for (i = 0; i < NUM_AIRLINES; ++i) {
				if (!Airlines[i]._CISclosed) {
					Acquire(Airlines[i]._lock);
					if (Airlines[i]._numCheckedinPassengers == Airlines[i]._numExpectedPassengers){
						if (Airlines[i]._numOnBreakCIS == NUM_CIS_PER_AIRLINE) {
							/* All Passenger have JUST went through, send CIS home */
							Release(Airlines[i]._lock); /* TODO is this necessary? */
							numDoneAirline++;
							for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
								/*Acquire(cis._lock);*/
								cis._done = true;
								Signal(Airlines[i]._lock, cis._commCV);
								/*Release(cis._lock);*/
							}
							Airlines[i]._CISclosed = true;
						} else {
							Release(Airlines[i]._lock); /* TODO is this necessary? */
						}
					} else {
						/* There are still passengers to serve */
						Release(Airlines[i]._lock);/* TODO is this necessary? */
						for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
							Acquire(Airlines[i]._cisLineLock);
							Acquire(Airlines[i]._execLineLock);
							Acquire(cis._lock);
							if ((!queue_empty(&Airlines[i]._execQueue) || cis._lineSize) && cis._state == ONBREAK) {
/*								Signal(cis._lock, cis._commCV);*/
								Signal(Airlines[i]._lock, cis._commCV);
							}
							Release(cis._lock);
							Release(Airlines[i]._execLineLock);
							Release(Airlines[i]._cisLineLock);
						}
					}
				} else {
					numDoneAirline++;
				}	
			}
			if (numDoneAirline == NUM_AIRLINES) {
				Manager._allCISDone = true;
Printf0("All Cis is done!\n", sizeof("All Cis is done!\n"));
			} else {
				Manager._allCISDone = false;
			}
		} /* end if(!_allCISDone) */
#undef cis
		/* end CIS */

		/*
			Check Conveyor Belt - Cargo Handlers
		*/
		if (!Manager._allCargoDone) {
			int numDone = 0;
			bool msgToCargo = true;

			Acquire(ConveyorLock);
			for (i = 0; i < NUM_AIRLINES; ++i) {
Printf1("Number loaded: %d, number expected: %d\n", sizeof("Number loaded: %d, number expected: %d\n"), concat2Num(Airlines[i]._numExpectedBaggages, Airlines[i]._numLoadedBaggages));
				if (Airlines[i]._numExpectedBaggages == Airlines[i]._numLoadedBaggages) {
					numDone++;
				}
			}
			for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
				if (!queue_empty(&ConveyorBelt) && CargoHandlers[i]._state == ONBREAK) {
					Signal(ConveyorLock, CargoHandlers[i]._commCV);

					if (msgToCargo) {
						Printf0("Airport manager calls back all the cargo handlers from break\n",
							sizeof("Airport manager calls back all the cargo handlers from break\n"));
						msgToCargo = false;
					}
				}
			}
			if (numDone == NUM_AIRLINES) {
Printf0("All Cargo Handlers done!\n", sizeof("All Cargo Handlers done!\n"));
				Manager._allCargoDone = true;
			}
			Release(ConveyorLock);
		}
		/* end Conveyor Belt / Cargo Handlers */

		if (Manager._allCISDone && Manager._allCargoDone) {
			break;
		}

		/*
			Check Boarding Lounge
		*/
		for (i = 0; i < NUM_AIRLINES; ++i) {
			if (Airlines[i]._boarded) {
				numReadyAirlines++;
			} else if(Airlines[i]._numExpectedBaggages == Airlines[i]._numLoadedBaggages
				&& Airlines[i]._numExpectedPassengers == Airlines[i]._numCheckedinPassengers) {
				Printf1("Airport manager gives a boarding call to airline %d\n",
					sizeof("Airport manager gives a boarding call to airline %d\n"),
					i);
				for (j = 0; j < Airlines[i]._numReadyPassengers; ++j) {
					Acquire(Airlines[j]._lock);
					Signal(Airlines[j]._lock, Airlines[j]._boardLoungeCV);
					Release(Airlines[j]._lock);
				}
				numReadyAirlines++;
				Airlines[i]._boarded;
			}
		}

		/*
			Make sure Manager doesn't hog CPU
		*/
		for (i = 0; i < 20; ++i) {
			Yield();
		}
	}
	Exit(0);
}

/*
	Init
*/
void initGlobalData() {
    int i;
	GlobalDataLock = CreateLock("GlobalDataLock", sizeof("GlobalDataLock"));
	LiaisonLineLock = CreateLock("LiaisonLineLock", sizeof("LiaisonLineLock"));
	ConveyorLock = CreateLock("ConveyorLock", sizeof("ConveyorLock"));
    for (i = 0; i < NUM_PASSENGERS*3; i++) {
        OfficersLine._array[i] = -1;
        ConveyorBelt._array[i] = -1;
    }
    OfficersLine._front = -1;
    OfficersLine._rear = -1;
    ConveyorBelt._front = -1;
    ConveyorBelt._rear= -1;
}

void initPassengers() {
	int i;
	int j;
	NumActivePassengers = 0;
	Airlines[Passengers[i]._ticket._airline]._numExpectedPassengers = 0;
	for (i = 0; i < NUM_PASSENGERS; i++) {
		Passengers[i]._id = i;
		Passengers[i]._inspectorID = -1;
 		Passengers[i]._officerID = -1;
 		Passengers[i]._liaisonID = 0;
 		Passengers[i]._cisID = 0;
 		Passengers[i]._furtherQuestioning = false;
 		/* Baggages */
 		Passengers[i]._numBaggages = i % 2 + 2;
 		for (j = 0; j < Passengers[i]._numBaggages; j++) {
			Baggages[(i * 3) + j]._weight = (i * 13) % 31 + 30; 
			Airlines[Passengers[i]._ticket._airline]._numExpectedBaggages++;
		}
 		/* Ticket */
 		Passengers[i]._ticket._airline = (i*17) % NUM_AIRLINES;
 		Airlines[Passengers[i]._ticket._airline]._numExpectedPassengers++;
		Passengers[i]._ticket._executive = false;
		if ( (i % 4) == 1 ) {
			Passengers[i]._ticket._executive = true;
		}
	}
}

void initLiaisons() {
/*Printf0("got into initLiaisons\n", sizeof("got into initLiaisons\n"));*/
	int i;
	int j;
	NumActiveLiaisons = 0;
	for (i = 0; i < NUM_LIASONS; i++) { 
		Liaisons[i]._lock = CreateLock(concatNumToString("liaison_lock_", sizeof("liaison_lock_"), i), 14);
		Liaisons[i]._lineCV = CreateCV(concatNumToString("liaison_lineCV_", sizeof("liaison_lineCV_"), i), 16);
		Liaisons[i]._commCV = CreateCV(concatNumToString("liaison_commCV_", sizeof("liaison_commCV_"), i), 16);
/*		Liaisons[i]._lock = CreateLock("llinelock", sizeof("llinelock"));
		Liaisons[i]._lineCV = CreateCV("llinecv", sizeof("llinecv"));
		Liaisons[i]._commCV = CreateCV("lcommcv", sizeof("lcommcv"));*/
		for (j = 0; j < NUM_AIRLINES; j++) {
			Liaisons[i]._passCount[j] = 0;
			Liaisons[i]._bagCount[j] = 0;
		}
		Liaisons[i]._currentPassenger = -1;
		Liaisons[i]._state = BUSY;
		Liaisons[i]._lineSize = 0;
	}
}

/*
	Initializes all CIS within an airline
*/
void initCIS(int airline) { 
#define cis Airlines[airline]._cis[i]
	int i;
    int j;
	NumActiveCIS = 0;
	for (i = 0; i < NUM_CIS_PER_AIRLINE; i++) {
		cis._lock = CreateLock(concatNumToString("CIS_lock_", sizeof("CIS_lock_"), i), 10);
    	cis._lineCV = CreateCV(concatNumToString("CIS_lineCV_", sizeof("CIS_lineCV_"), i), 12);
    	cis._commCV = CreateCV(concatNumToString("CIS_comCV_", sizeof("CIS_comCV_"), i), 11);;
    	cis._lineSize = 0;
    	cis._state = BUSY;
    	cis._passCount = 0;
    	cis._bagCount = 0;
    	cis._weightCount = 0;
    	cis._currentPassenger = -1;
    	cis._airline = airline;
    	cis._done = false;
	}
#undef cis
}

void initManager() {
	Manager._allCISDone = false;
    Manager._allCargoDone = false;
    Manager._allSODone = false;
    Manager._allSIDone = false;
}

void initCargoHandlers() {
#define ch CargoHandlers[i]	
	int i, j;
	NumActiveCargoHandlers = 0;
	for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
    	ch._commCV = CreateCV(concatNumToString("cargo_CV_", sizeof("cargo_CV_"), i), 10);
    	ch._state = BUSY;
    	for (j = 0; j < NUM_AIRLINES; ++j) {	
    		ch._bagCount[NUM_AIRLINES] = 0;
    		ch._weightCount[NUM_AIRLINES] = 0;
    	}
	}
#undef ch
}

void initScreeningOfficers() {
	int i;
	NumActiveScreeningOfficers = 0;
	#define officer ScreeningOfficers[i]
	for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		officer._lock = CreateLock(concatNumToString("officer_lock_", sizeof("officer_lock_"), i), 14);
    	officer._commCV = CreateCV(concatNumToString("officer_commCV_", sizeof("officer_commCV_"), i), 20);
    	officer._state = BUSY; 
    	officer._passCount = 0;
    	officer._currentPassenger = -1;
    	officer._myNum = i;
    	officer._done = false;
	}
	#undef officer
}

void initSecurityInspectors() {
	int i;
	NumActiveSecurityInspectors = 0;
	#define inspector SecurityInspectors[i]
	for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		inspector._id = i;
	    inspector._rtnPassSize = 0;
	    inspector._state = BUSY;
	    inspector._lock = CreateLock(concatNumToString("inspector_lock_", sizeof("inspector_lock_"), i), 14);
	    inspector._commCV = CreateCV(concatNumToString("_commCV", sizeof("_commCV"), i), 20);
	    inspector._rtnPassCV = CreateCV(concatNumToString("_rtnPassCV", sizeof("_rtnPassCV"), i), 20);
	    inspector._newPassCV = CreateCV(concatNumToString("_newPassCV", sizeof("_newPassCV"), i), 20);
	    inspector._rtnPassenger = -1;
	    inspector._newPassenger = -1;
	    inspector._passCount = 0;
	}
	#undef inspector
}

void initAirlines() {
#define a Airlines[i]
	int i, j;
	for (i = 0; i < NUM_AIRLINES; ++i) {
		a._lock = CreateLock(concatNumToString("airline_lock_", sizeof("airline_lock_"), i), 14);
    	a._execLineLock = CreateLock(concatNumToString("airline_exec_lock_", sizeof("airline_exec_lock_"), i), 19);
    	a._execLineCV = CreateCV(concatNumToString("airline_execLineCV_", sizeof("airline_execLineCV_"), i), 20);
    	a._boardLoungeCV = CreateCV(concatNumToString("airline_boardLoungeCV_", sizeof("airline_boardLoungeCV_"), i), 23);
    	a._cisLineLock = CreateLock(concatNumToString("airline_CIS_lock_", sizeof("airline_CIS_lock_"), i), 18);
/*    	for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {*/
    		initCIS(i);
/*    	}*/
        for (j = 0; j < NUM_PASSENGERS; j++) {
            a._execQueue._array[j] = -1;
        }
        a._execQueue._front = -1;
        a._execQueue._rear = -1;

  		/* Stats */
/*    	a._numExpectedPassengers = 0;*/
    	a._numCheckedinPassengers = 0;
    	a._numReadyPassengers = 0;
    	a._numExpectedBaggages = 0;
    	a._numLoadedBaggages = 0;
    	a._bagCount = 0;
    	a._weightCount = 0;
    	/* Shutdown */
    	a._numOnBreakCIS = 0;
    	a._boarded = false;
    	a._allPassengersCheckedIn = false;
    	a._CISclosed = false;
	}
#undef a
}

void init() {
	initGlobalData();
	initPassengers();
	initLiaisons();
	initManager();
	initCargoHandlers();
	initScreeningOfficers();
	initSecurityInspectors();
	initAirlines();

}

void forkThreads() {
	int i, j;
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		Fork(startPassenger, "passenger", sizeof("passenger")); /* params: ftnptr, name, size */
	}
	for (i = 0; i < NUM_LIASONS; ++i) {
		Fork(startLiaison, "liaison", sizeof("liaison"));
	}
	for (i = 0; i < NUM_AIRLINES; ++i) {
		for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
			Fork(startCheckInStaff, "cis", sizeof("cis"));
		}
	}
	for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
		Fork(startCargoHandler, "ch", sizeof("ch"));
	}
	/*for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		Fork(startScreeningOfficer);
	}
	for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
		Fork(startSecurityInspector);
	}*/
	Fork(startManager, "manager", sizeof("manager"));
}

/*
	main - start the airport sim
*/
int main() {
    init();
	forkThreads();
}

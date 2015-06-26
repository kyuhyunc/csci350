/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */
#include "syscall.h"

#define NULL 0

#define NUM_PASSENGERS 10
#define	NUM_LIASONS 5
#define	NUM_AIRLINES 5
#define	NUM_CIS_PER_AIRLINE 3
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
    int _array[NUM_PASSENGERS];
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

/* Number of currently active entities */
int NumActivePassengers; 
int NumActiveLiaisons;
int NumActiveCIS;
int NumActiveCargoHandlers;

/* Locks */
int GlobalDataLock; /* Used for initializing */
int LiaisonLineLock;
int ConveyorLock;

/* Queue */
Queue OfficersLine;
Queue ConveyorBelt;

/*
	Utilities	
*/
char* concatNumToString(char* str, int num) { /* TODO - Not working Properly */
	char cnum = (char)(((int)'0') + num);
	int len = sizeof(str);
	str[len] = cnum;
	return str;
}

int concatNum(int i, int j, int k) {
	return 1000000 * i + 1000 * j + k;
} 

#define front queue->_front
#define rear queue->_rear
void queue_insert (Queue* queue, int index) {
    if (queue->_rear == NUM_PASSENGERS-1)
        Printf0("ERROR: QUEUE OVERFLOW\n", sizeof("ERROR: QUEUE OVERFLOW\n"));
    else {
        /* If queue is initially empty */
        if (queue->_front == -1)
            queue->_front = 0;
        queue->_array[++rear] = index;
    }
}

int queue_pop (Queue* queue) {
    if (queue->_front == -1 || queue->_front > queue->_rear) {
        Printf0("ERROR: QUEUE IS EMPTY\n", sizeof("ERROR: QUEUE IS EMPTY\n"));
        return -1;
    }
    return queue->_array[front++];    
}

int queue_size (Queue* queue) {
    if (queue->_front == -1 || queue->_front > queue->_rear) return 0;
    return rear - front + 1;
}

bool queue_empty (Queue* queue) {
    if (queue_size (queue) == 0) return true;
    else return false;
}
#undef front
#undef reqr

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
		concatNum(_myIndex, my._liaisonID, liaison._lineSize));
	/* Get in line? */
	if (liaison._state == BUSY) {
		Printf0("Passenger says liaison is BUSY\n", 
			sizeof("Passenger says liaison is BUSY\n"));
		Printf1("Passenger's liaison id: %d\n", 
			sizeof("Passenger's liaison id: %d\n"),
			LiaisonLineLock);
		liaison._lineSize++;
		Wait(LiaisonLineLock, liaison._lineCV);
		liaison._lineSize--;
	}
	Printf0("Passenger is moving along...\n", sizeof("Passenger is moving along...\n"));
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
		concatNum(0, _myIndex, my._liaisonID));
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
			concatNum(0, _myIndex, my._ticket._airline));
		Wait(myAirline._execLineLock, myAirline._execLineLock); /* Wait on CIS */
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
			_myIndex,
			concatNum(my._ticket._airline, my._cisID, _minLineSize));
		myCIS._lineSize++;
		Wait(myAirline._cisLineLock, myCIS._lineCV);
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
		concatNum(_myIndex, my._ticket._airline, my._ticket._airline));
	Signal(myCIS._lock, myCIS._commCV); /* Signal CIS */
	Release(myCIS._lock);

#undef myCIS
#undef myAirline
	/* end Check-in Staff Interaction */		
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
		Printf0("1\n", sizeof("1\n"));
	    Acquire(LiaisonLineLock);
	    Acquire(l._lock);
		Printf0("2\n", sizeof("1\n"));
	    if (l._lineSize == 0) {
	    	Printf0("3.0\n", sizeof("3.3\n"));
	    	Release(LiaisonLineLock);
	    	l._state = AVAIL;
	    	Printf0("3.3\n", sizeof("3.3\n"));
	    	Wait(l._lock, l._commCV); /* Wait for new Passenger */
	    	Printf0("3.5\n", sizeof("3.3\n"));
	    } else {
	    	Printf0("4\n", sizeof("1\n"));
	    	Signal(LiaisonLineLock, l._lineCV); /* Signal Passenger */
	    	Release(LiaisonLineLock); 
	    	Printf0("5\n", sizeof("1\n"));
			Printf1("Liaison say his id is: %d\n", 
				sizeof("Liaison say his id is: %d\n"),
				LiaisonLineLock);
	    	Wait(l._lock, l._commCV); /* Wait on Passenger */
	    }
	    l._state = BUSY;
	    Printf1("Airport Liaison %d directed passenger %d of airline %d\n",
	    	sizeof("Airport Liaison %d directed passenger %d of airline %d\n"),
	    	concatNum(_myIndex, l._currentPassenger, Passengers[l._currentPassenger]._ticket._airline));
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
    _myIndex = (NumActiveCIS % NUM_AIRLINES);
    _myAirline = (int)(NumActiveCIS / NUM_AIRLINES); 
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
			Wait(myAirline._lock, my._commCV); /* Wait on Manager */ /* TODO - make sure okay to wait on aiport lock... maybe better? */
			/* Time to go home! TGIF! */
			if (my._done) {
				Printf1("Airline check-in staff %d is closing the counter\n",
					sizeof("Airline check-in staff %d is closing the counter\n"),
					_myIndex);
				Acquire(my._lock);
				Release(myAirline._lock);
				Wait(my._lock, my._commCV); /* Wait forever, basically */
				Release(my._lock); /* Never reaches here, but whatever... */
			}
			myAirline._numOnBreakCIS--;
		}
		/*Printf0("2\n", sizeof("1\n"));*/
		/* Start helping a passenger */
		my._state = BUSY;
		Acquire(myAirline._cisLineLock);
		Acquire(myAirline._execLineLock);
		if (queue_size(&myAirline._execQueue) > 0) {
			my._currentPassenger = queue_pop(&myAirline._execQueue);
			passenger._cisID = _myIndex;
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concatNum(_myIndex, _myAirline, my._lineSize));
			Signal(myAirline._execLineLock, myAirline._execLineCV); /* Signal Passenger */
		} else if (my._lineSize > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concatNum(_myIndex, _myAirline, queue_size(&myAirline._execQueue)));
			Signal(myAirline._execLineLock, myAirline._execLineCV); /* Signal Passenger */
		}
		/* Interact with Passenger */
		Acquire(my._lock);
		Release(myAirline._cisLineLock);
		Release(myAirline._execLineLock);
		Release(myAirline._lock);
		if (my._lineSize > 0 || passenger._id != -1) {
			Wait(my._lock, my._commCV); 
		} /* Otherwise, Manager woke you up for no reason */
		if (passenger._id != -1) {
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
				#define bagIndex (passenger._id * 3) + i
				#define bag Baggages[bagIndex] 
				bag._airline = _myAirline; /* Tag the bag */
				queue_insert(&ConveyorBelt, bagIndex);
				Printf1("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n",
					sizeof("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n"),
					concatNum(0, _myIndex, _myAirline));
				myAirline._numExpectedBaggages++;
				my._weightCount += bag._weight;
				#undef bag
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (passenger._ticket._executive) {
				Printf2("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n"),
					_myIndex, concatNum(_myAirline, my._currentPassenger, _myAirline));
			} else {
				Printf2("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n"),
					_myIndex, concatNum(_myAirline, my._currentPassenger, _myAirline));
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
	Printf0("startCargoHandler\n", sizeof("startCargoHandler\n"));
	Exit(0);
}

void startScreeningOfficer() {
	Printf0("startScreeningOfficer\n", sizeof("startScreeningOfficer\n"));
	Exit(0);
}

void startSecurityInspector() {
	Printf0("startSecurityInspector\n", sizeof("startSecurityInspector\n"));
	Exit(0);
}

void startManager() {
#define cis Airlines[i]._cis[j]
	int i, j; /* for-loop iterator */
	while (true) {
		/*
			Check-in Staff 
		*/
		if (!Manager._allCISDone) {
			int numDoneCIS = 0;
			for (i = 0; i < NUM_AIRLINES; ++i) {
				if (!Airlines[i]._CISclosed) {
					Acquire(Airlines[i]._lock);
					if (Airlines[i]._numCheckedinPassengers == Airlines[i]._numExpectedPassengers){
						if (Airlines[i]._numOnBreakCIS == NUM_CIS_PER_AIRLINE) {
							/* All Passenger have JUST went through, send CIS home */
							Release(Airlines[i]._lock); /* TODO is this necessary? */
							numDoneCIS++;
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
							Acquire(cis._lock);

							Release(cis._lock);
						}
					}
				}	
			}
		} /* end if(!_allCISDone) */
#undef cis
		/* end CIS */

		/*
			Make sure Manager doesn't hog CPU
		*/
		for (i = 0; i < 2000; ++i) {
			/*Yield();*/
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
    for (i = 0; i < NUM_PASSENGERS; i++) {
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
	int i;
	int j;
	NumActiveLiaisons = 0;
	for (i = 0; i < NUM_LIASONS; i++) { 
		Liaisons[i]._lock = CreateLock(concatNumToString("liaison_lock_", i), 14);
		Liaisons[i]._lineCV = CreateCV(concatNumToString("liaison_lineCV_", i), 16);
		Liaisons[i]._commCV = CreateCV(concatNumToString("liaison_commCV_", i), 16);
		for (j = 0; j < NUM_AIRLINES; j++) {
			Liaisons[i]._passCount[j] = 0;
			Liaisons[i]._bagCount[j] = 0;
		}
		Liaisons[i]._currentPassenger = -1;
		Liaisons[i]._state = BUSY;
		Liaisons[i]._lineSize = 0;
	}
}

void initCIS(int airline) { 
#define c Airlines[airline]._cis[i]
	int i;
    int j;
	NumActiveCIS = 0;
	for (i = 0; i < NUM_CIS_PER_AIRLINE; i++) {
		c._lock = CreateLock(concatNumToString("CIS_lock_", i), 10);
    	c._lineCV = CreateCV(concatNumToString("CIS_lineCV_", i), 12);
    	c._commCV = CreateCV(concatNumToString("CIS_comCV_", i), 11);;
    	c._lineSize = 0;
    	c._state = BUSY;
    	c._passCount = 0;
    	c._bagCount = 0;
    	c._weightCount = 0;
    	c._currentPassenger = -1;
    	c._airline = airline;
    	c._done = false;
	}
#undef c
}

void initManager() {
	Manager._allCISDone = false;
    Manager._allCargoDone = false;
    Manager._allSODone = false;
    Manager._allSIDone = false;
}

void initCargoHandlers() {
#define ch CargoHandlers[i]	
	int i;
	for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
    	ch._commCV = CreateCV(concatNumToString("cargo_CV_", i), 10);
    	ch._state = BUSY;
    	for (i = 0; i < NUM_AIRLINES; ++i) {	
    		ch._bagCount[NUM_AIRLINES] = 0;
    		ch._weightCount[NUM_AIRLINES] = 0;
    	}
	}
#undef ch
}

void initAirlines() {
#define a Airlines[i]
	int i, j;
	for (i = 0; i < NUM_AIRLINES; ++i) {
		a._lock = CreateLock(concatNumToString("airline_lock_", i), 14);
    	a._execLineLock = CreateLock(concatNumToString("airline_exec_lock_", i), 19);
    	a._execLineCV = CreateCV(concatNumToString("airline_execLineCV_", i), 20);
    	a._boardLoungeCV = CreateCV(concatNumToString("airline_boardLoungeCV_", i), 23);
    	a._cisLineLock = CreateLock(concatNumToString("airline_CIS_lock_", i), 18);
    	for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
    		initCIS(j);
    	}
        for (j = 0; j < NUM_PASSENGERS; j++) {
            a._execQueue._array[j] = -1;
        }
        a._execQueue._front = -1;
        a._execQueue._rear = -1;

  		/* Stats */
    	a._numExpectedPassengers = 0;
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
	/*initManager();*/
	initCargoHandlers();
	initAirlines();
}

void forkThreads() {
	int i, j;
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		Fork(startPassenger); /* params: ftnptr, name, size */
	}
	for (i = 0; i < NUM_LIASONS; ++i) {
		Fork(startLiaison);
	}
	for (i = 0; i < NUM_AIRLINES; ++i) {
		for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
			/*Fork(startCheckInStaff);*/
		}
	}
	/*for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
		Fork(startCargoHandler);
	}
	for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		Fork(startScreeningOfficer);
	}
	for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
		Fork(startSecurityInspector);
	}*/
	Fork(startManager);
}

/*
	main - start the airport sim
*/
int main() {
    init();
	forkThreads();
}

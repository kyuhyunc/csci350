/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */
#include "syscall.h"

#define NULL 0

#define NUM_PASSENGERS 2
#define	NUM_LIASONS 2
#define	NUM_AIRLINES 2
#define	NUM_CIS_PER_AIRLINE 2
#define	NUM_CARGO_HANDLERS 2
#define	NUM_SCREENING_OFFICERS 2
#define	NUM_SECURITY_INSPECTORS 2

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

/* Passenger */
typedef struct {
    bool _executive;
    int _airline; 
    int _seat; 
} Ticket;

typedef struct {
    int _airline; 
    int _weight; 
} Baggage;

typedef struct {
 	int _id;   
 	int _myInspector;
 	int _myOfficer;
 	int _myLiaison;
 	int _myCIS;
 	bool _furtherQuestioning;
 	Baggage _baggages[3];
 	int _numBaggages;
 	Ticket _myTicket;
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

/* Airline */
typedef struct {
    int _lock; 
    int _execLineLock;
    int _execLineCV;
    int _execQueue[NUM_PASSENGERS];
    int _execLineSize;
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

/* Number of currently active entities */
int NumActivePassengers; 
int NumActiveLiaisons;
int NumActiveCIS;

/* Locks */
int GlobalDataLock; /* Used for  */
int LiaisonLineLock;
int ConveyorLock;

/* CVs */

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

/*
	Start Functions - functions called by Fork() syscall.
*/
void startPassenger() {    
#define p Passengers[_myIndex]
#define liaison Liaisons[p._myLiaison]
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
			p._myLiaison = i;
		}
	}

	Printf1("Passenger %d chose Liaison %d with a line length %d\n", 
		sizeof("Passenger %d chose Liaison %d with a line length %d\n"), 
		concatNum(_myIndex, p._myLiaison, liaison._lineSize));
	/* Get in line? */
	if (liaison._state == BUSY) {
		liaison._lineSize++;
		Wait(LiaisonLineLock, liaison._lineCV);
		liaison._lineSize--;
	}
	/* Go to Liaison */
	Acquire(liaison._lock);
	Release(LiaisonLineLock);
	/* Give Liaison my Passenger info */
	liaison._passCount[p._myTicket._airline]++;
	liaison._bagCount[p._myTicket._airline] += p._numBaggages;
	liaison._currentPassenger = _myIndex;
	Signal(liaison._lock, liaison._commCV); /* Signal Liaison */
	Wait(liaison._lock, liaison._commCV); /* Wait for Liaison */
	Printf1("Passenger %d of Airline %d is directed to the check-in counter\n", 
		sizeof("Passenger %d of Airline %d is directed to the check-in counter\n"),
		concatNum(0, _myIndex, p._myLiaison));
	Signal(liaison._lock, liaison._commCV);
	Release(liaison._lock);
	/* end Liaison Interaction */

	/*
		Check-in Staff Interaction
	*/
	
	/* end Check-in Staff Interaction */		
	Exit(0);
#undef p
#undef liaison
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
	    	concatNum(_myIndex, l._currentPassenger, Passengers[l._currentPassenger]._myTicket._airline));
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
    _myAirline = (int)(NumActiveCIS / NUM_AIRLINES); /* TODO - Get second opinion */
    NumActiveCIS++;
    Release(GlobalDataLock);

    while (true) {
		/* Check lines */
		Acquire(myAirline._lock);
		if (my._lineSize == 0 && myAirline._execLineSize == 0) {
			my._state = ONBREAK;
			my._currentPassenger = NULL;
			/* 'Clock Out' for Break */
			myAirline._numOnBreakCIS++;
			Wait(myAirline._lock, my._commCV); /* Wait on Manager */ /* TODO - make sure okay... maybe better? */
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
		/* Start helping a passenger */
		my._state = BUSY;
		Acquire(myAirline._cisLineLock);
		Acquire(myAirline._execLineLock);
		if (myAirline._execLineSize > 0) {
			/*_currentPassenger = */ /* TODO - Grab executive passenger off execQueue */
			passenger._myCIS = _myIndex;
			myAirline._execLineSize--;
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concatNum(_myIndex, _myAirline, my._lineSize));
			Signal(myAirline._execLineLock, myAirline._execLineCV); /* Signal Passenger */
		} else if (my._lineSize > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concatNum(_myIndex, _myAirline, myAirline._execLineSize));
			Signal(myAirline._execLineLock, myAirline._execLineCV); /* Signal Passenger */
		}
		/* Interact with Passenger */
		Acquire(my._lock);
		Release(myAirline._cisLineLock);
		Release(myAirline._execLineLock);
		Release(myAirline._lock);
		if (my._lineSize > 0 || my._currentPassenger != -1) {
			Wait(my._lock, my._commCV); 
		} /* Otherwise, manager woke you up for no reason */
		if (my._currentPassenger != -1) {
			int i;
			/* Assign seat number */
			Acquire(myAirline._lock);
			passenger._myTicket._seat = myAirline._numCheckedinPassengers;
			myAirline._numCheckedinPassengers++;
			Release(myAirline._lock);
			/* Deal with baggage */
			Acquire(ConveyorLock);
			for (i = 0; i < passenger._numBaggages; ++i) {
				#define bag passenger._baggages[i] 
				bag._airline = _myAirline; /* Tag the bag */
				/* TODO - figure out how to put bags on conveyor belt */
				Printf1("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n",
					sizeof("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n"),
					concatNum(0, _myIndex, _myAirline));
				myAirline._numExpectedBaggages++;
				my._weightCount += bag._weight;
				#undef bag
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (passenger._myTicket._executive) {
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

/*
	Init
*/
void initGlobalData() {
	GlobalDataLock = CreateLock("GlobalDataLock", sizeof("GlobalDataLock"));
	LiaisonLineLock = CreateLock("LiaisonLineLock", sizeof("LiaisonLineLock"));
	ConveyorLock = CreateLock("ConveyorLock", sizeof("ConveyorLock"));
}

void initPassengers() {
	int i;
	int j;
	NumActivePassengers = 0;
	for (i = 0; i < NUM_PASSENGERS; i++) {
		Passengers[i]._id = i;
		Passengers[i]._myInspector = -1;
 		Passengers[i]._myOfficer = -1;
 		Passengers[i]._myLiaison = 0;
 		Passengers[i]._myCIS = 0;
 		Passengers[i]._furtherQuestioning = false;
 		/* Baggages */
 		Passengers[i]._numBaggages = i % 2 + 2;
 		for (j = 0; j < Passengers[i]._numBaggages; j++) {
			Passengers[i]._baggages[j]._weight = (i * 13) % 31 + 30; 
		}
 		/* Ticket */
 		Passengers[i]._myTicket._airline = (i*17) % NUM_AIRLINES;
		Passengers[i]._myTicket._executive = false;
		if ( (i % 4) == 1 ) {
			Passengers[i]._myTicket._executive = true;
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
}

void initAirlines() {
#define a Airlines[i]
	int i, j;
	for (i = 0; i < NUM_AIRLINES; ++i) {
		a._lock = CreateLock(concatNumToString("airline_lock_", i), 14);
    	a._execLineLock = CreateLock(concatNumToString("airline_exec_lock_", i), 19);
    	a._execLineCV = CreateCV(concatNumToString("airline_execLineCV_", i), 20);
    	a._execLineSize = 0;
    	a._boardLoungeCV = CreateCV(concatNumToString("airline_boardLoungeCV_", i), 23);
    	a._cisLineLock = CreateLock(concatNumToString("airline_CIS_lock_", i), 18);
    	for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
    		initCIS(j);
    	}
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
			Fork(startCheckInStaff);
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
}

/*
	main - start the airport sim
*/
int main() {
    init();
	forkThreads();
}

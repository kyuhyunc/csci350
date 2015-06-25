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

/* Passenger Structures */
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
	/*char *_name;*/
 	int _id;   
 	int _myInspector;
 	int _myOfficer;
 	int _myLiaison;
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

/*
	Global Data
*/

/* Airport Entities */
Passenger passengers[NUM_PASSENGERS];
Liaison Liaisons[NUM_LIASONS];

/* Number of currently active entities */
int numActivePassengers; 
int numActiveLiaisons;

/* Locks */
int CountLock;
int LiaisonLineLock;

/* CVs */

/*
	Utility Functions	
*/
char* concatNumToString(char* str, int num) {
	char cnum = (char)(((int)'0') + num);
	int len = sizeof(str);
	str[len] = cnum;
	return str;
}

unsigned int concatNum(int i, int j, int k) {
	return 1000000 * i + 1000 * j + k;
}

/*
	Start Functions - functions called by Fork() syscall.
	One per Type of Thread
*/
#define p passengers[_myIndex]
#define liaison Liaisons[p._myLiaison]
void startPassenger() {    
	int i, j; /* for-loop iterators */
	int _myIndex;
	int _minLineSize;

    Acquire(CountLock);
    _myIndex = numActivePassengers++;
    Release(CountLock);

    /*
		Liaison Interaction
    */
	Acquire(LiaisonLineLock);
	_minLineSize = Liaisons[0]._lineSize;
	/*Printf1("!!!!: %d\n", sizeof("!!!!: %d\n"), _minLineSize);*/
	/* Find shortest line */
	for (i = 1; i < NUM_LIASONS; i++) {
		/*Printf1("!!!!: %d\n", sizeof("!!!!: %d\n"), Liaisons[i]._lineSize);*/
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
	Signal(liaison._lock, liaison._commCV);
	Wait(liaison._lock, liaison._commCV);

	Printf1("Passenger %d of Airline %d is directed to the check-in counter\n", 
		sizeof("Passenger %d of Airline %d is directed to the check-in counter\n"),
		concatNum(0, _myIndex, p._myLiaison));

	Signal(liaison._lock, liaison._commCV);
	Release(liaison._lock);
	/* end Liaison Interaction */
	Exit(0);
}
#undef p

#define l Liaisons[_myIndex]
void startLiaison() {
	int _myIndex;
    Acquire(CountLock);
    _myIndex = numActiveLiaisons++;
    Release(CountLock);

    Acquire(LiaisonLineLock);
    Acquire(l._lock);
    if (l._lineSize == 0) {
    	Release(LiaisonLineLock);
    	l._state = AVAIL;
    	Wait(l._lock, l._commCV);
    } else {
    	Signal(LiaisonLineLock, l._lineCV);
    	Release(LiaisonLineLock);
    	Wait(l._lock, l._commCV);
    }
    l._state = BUSY;
    Printf1("Airport Liaison %d directed passenger %d of airline %d\n",
    	sizeof("Airport Liaison %d directed passenger %d of airline %d\n"),
    	concatNum(_myIndex, l._currentPassenger, passengers[l._currentPassenger]._myTicket._airline));
    Signal(l._lock, l._commCV);
    Wait(l._lock, l._commCV);
    Release(l._lock);

	Exit(0);
}
#undef l

void startCheckInStaff() {
	Printf0("startCheckInStaff\n", sizeof("startCheckInStaff\n"));
	Exit(0);
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
void initPassengers() {
	int i;
	int j;
	numActivePassengers = 0;
	for (i = 0; i < NUM_PASSENGERS; i++) {
		passengers[i]._id = i;
		passengers[i]._myInspector = -1;
 		passengers[i]._myOfficer = -1;
 		passengers[i]._myLiaison = 0;
 		passengers[i]._furtherQuestioning = false;
 		/* Baggages */
 		passengers[i]._numBaggages = i % 2 + 2;
 		for (j = 0; j < passengers[i]._numBaggages; j++) {
			passengers[i]._baggages[j]._weight = (i * 13) % 31 + 30; 
		}
 		/* Ticket */
 		passengers[i]._myTicket._airline = (i*17) % NUM_AIRLINES;
		passengers[i]._myTicket._executive = false;
		if ( (i % 4) == 1 ) {
			passengers[i]._myTicket._executive = true;
		}
	}
}

void initLiaisons() {
	int i;
	int j;
	numActiveLiaisons = 0;
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

void forkThreads() {
	int i;
	for (i = 0; i < NUM_PASSENGERS; i++) {
		Fork(startPassenger);
	}
	for (i = 0; i < NUM_LIASONS; ++i) {
		Fork(startLiaison);
	}
	/*for (i = 0; i < NUM_CIS_PER_AIRLINE; ++i) {
		Fork(startCheckInStaff);
	}
	for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
		Fork(startCargoHandler);
	}
	for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		Fork(startScreeningOfficer);
	}
	for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
		Fork(startSecurityInspector);
	}*/
}

void init() {
	CountLock = CreateLock("CountLock", sizeof("CountLock"));
	LiaisonLineLock = CreateLock("LiaisonLineLock", sizeof("LiaisonLineLock"));
	/* Inits */
	initPassengers();
	initLiaisons();
	/* Fork */
	forkThreads();
}

/*
	main - start the airport sim
*/
int main() {
    init();
}

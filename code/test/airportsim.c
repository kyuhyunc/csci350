/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */
#include "syscall.h"

#define NULL 0
#define NUM_PASSENGERS 10
#define	NUM_LIASONS 2
#define	NUM_AIRLINES 2
#define	NUM_CIS_PER_AIRLINE 2
#define	NUM_CARGO_HANDLERS 2
#define	NUM_SCREENING_OFFICERS 2
#define	NUM_SECURITY_INSPECTORS 2

typedef int bool;
enum bool {false, true};

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
 	int _myLine;
 	bool _furtherQuestioning;
 	Baggage _baggages[3];
 	int _numBaggages;
 	Ticket _myTicket;
} Passenger;

/*
	Global Data
*/
int countLock;

/* Airport Entities */
Passenger passengers[NUM_PASSENGERS];

/* Number of currently active entities */
int numActivePassengers; 

/* States used by various employees */
enum State {
    AVAIL,
    BUSY,
    ONBREAK
};

/*
	Start Functions - functions called by Fork() syscall.
	One per Type of Thread
*/
#define p passengers[_myIndex]
void startPassenger() {    
	int _myIndex;
    Acquire(countLock);
    _myIndex = numActivePassengers++;
    Release(countLock);

	Printf1("startPassenger %d\n", sizeof("startPassenger %d\n"), p._id);
	Exit(0);
}
#undef p

void startLiaison() {
	Printf0("startLiaison\n", sizeof("startLiaison\n"));
	Exit(0);
}

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
	for (i = 0; i < NUM_PASSENGERS; i++) {
		passengers[i]._id = i;
		passengers[i]._myInspector = -1;
 		passengers[i]._myOfficer = -1;
 		passengers[i]._myLine = -1;
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

void forkThreads() {
	int i;
	for (i = 0; i < NUM_PASSENGERS; i++) {
		Fork(startPassenger);
	}
	/*for (i = 0; i < NUM_LIASONS; ++i) {
		Fork(startLiaison);
	}
	for (i = 0; i < NUM_CIS_PER_AIRLINE; ++i) {
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
	countLock = CreateLock("countLock", sizeof("countLock"));
	/* Inits */
	initPassengers();
	/* Fork */
	forkThreads();
}

/*
	main - start the airport sim
*/
int main() {
    init();
}

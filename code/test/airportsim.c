/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */

#include "syscall.h"

/*
	Structs
*/

/* Passenger Structures */
struct Ticket {
    bool _executive;
    int _airline; // choices between 0, 1, and 2
    int _seat; // uninitialized--done by cis
};

struct Baggage {
    int _airline; // uninitialized--done by cis
    int _weight; // between 30 and 60
};

struct Passenger {
	char *_name;
 	int _id;   
 	int _myInspector;
 	int _myOfficer;
 	int _myLine;
 	bool _furtherQuestioning;
 	struct Baggages _baggages[3];

    Passenger(char *name, int id) {    
        strcpy(_name, name);
        _id = id;
    }
};

/*
	Global Data
*/
/*
int NUM_PASSENGERS;
int NUM_LIASONS;
int NUM_AIRLINES;
int NUM_CIS_PER_AIRLINE;
int NUM_CARGO_HANDLERS;
int NUM_SCREENING_OFFICERS;
int NUM_SECURITY_INSPECTORS;
*/
int	NUM_PASSENGERS = 2;
int	NUM_LIASONS = 2;
int	NUM_AIRLINES = 2;
int	NUM_CIS_PER_AIRLINE = 2;
int	NUM_CARGO_HANDLERS = 2;
int	NUM_SCREENING_OFFICERS = 2;
int	NUM_SECURITY_INSPECTORS = 2;

/*
    Statical values
*/
static struct Passenger passengers[NUM_PASSENGERS];

/* Pointers to Entities */
//struct Passenger** passengers;

/* Number of currently active entities */
int numInitPassengers; 

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
void startPassenger() {
	Printf0("startPassenger ", sizeof("startPassenger "));
    Printf1("%d \n", sizeof("%d \n"), numInitPassengers);
/*	struct Passenger p;
	p.id = 69;
	Printf1("startPassenger %d\n", sizeof("startPassenger %d\n"), p.id);*/
	Exit(0);
}

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
	main - start the airport sim
*/
int main() {
	/*
		Init Local Data
	*/
    int i;  /* For loop iterator */

	/*
		Init Global Data
	*/
    numInitPassengers = 0; 
/*
	NUM_PASSENGERS = 2;
	NUM_LIASONS = 2;
	NUM_AIRLINES = 2;
	NUM_CIS_PER_AIRLINE = 2;
	NUM_CARGO_HANDLERS = 2;
	NUM_SCREENING_OFFICERS = 2;
	NUM_SECURITY_INSPECTORS = 2;
*/

	/*passengers = (struct Passenger**)malloc( 7 );*/
/*	liaisons;
	cis;
	cargoHandlers;
	screeningOfficers;
	securityInspector;*/

	/*
		Initialize Threads
	*/
	for (i = 0; i < NUM_PASSENGERS; i++) {
		/*passengers[i] = (struct Passenger*)malloc(sizeof(struct Passenger));*/
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

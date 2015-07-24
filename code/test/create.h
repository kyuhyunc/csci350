/*
*
*/

#include "syscall.h"

#define NUM_PASSENGERS 10
#define	NUM_LIASONS 5
#define	NUM_AIRLINES 2
#define	NUM_CIS_PER_AIRLINE 3
#define	NUM_CARGO_HANDLERS 9
#define	NUM_SCREENING_OFFICERS 8
#define	NUM_SECURITY_INSPECTORS 8

/* Airline Lock */
#define AirlineLock 0 
#define AirlineExecLineLock 1
#define AirlineExecLineCV 2
#define AirlineExecQueue 3
#define AirlineBoardLoungeCV 4
#define AirlineCISLineLock 5
#define AirlineCIS 6
#define AirlineNumExpectedPassenger 7
#define AirlineNumCheckedinPassengers 8
#define AirlineNumReadyPassengers 9
#define AirlineNumExpectedBaggages 10
#define AirlineNumLoadedBaggages 11
#define AirlineBagCount 12
#define AirlineWeightCount 13
#define AirlineNumOnBreakCIS 14
#define AirlineBoarded 15
#define AirlineAllPassengersCheckedIn 16
#define AirlineCISClosed 17

/* CIS */
#define CISLock 0
#define CISLineCV 1
#define CISCommCV 2 
#define CISLineSize 3
#define CISState 4
#define CISPassCount 5
#define CISBagCount 6
#define CISWeightCount 7
#define CISCurrentPassenger 8
#define CISAirline 9
#define CISDone 10

/* CargoHandler */
#define CHCommCV 0
#define CHState 1
#define CHBagCount 2
#define CHWeightCount 3

/* Manager */
#define ManAllLiaisonDone 0
#define ManAllCISDone 1
#define ManAllCargoDone 2
#define ManAllSODone 3
#define ManAllSIDone 4

/* Security Inspectors */
#define SIID 0
#define SIRtnPassSize 1
#define SIState 2
#define SILock 3
#define SICommCV 4
#define SIRtnPassCV 5
#define SINewPassCV 6
#define SIRtnPassenger 7
#define SINewPassenger 8
#define SIPassCount 9

/* Screening Officer */
#define SOLock 0
#define SOCommCV 1
#define SOState 2
#define SOPassCount 3
#define SOCurrentPassenger 4
#define SOMyNum 5
#define SODone 6

/* Liaison */
#define LiaisonLock 0
#define LiaisonLineCV 1
#define LiaisonCommCV 2
#define LiaisonLineSize 3
#define LiaisonState 4
#define LiaisonPassCount 5
#define LiaisonBagCount 6
#define LiaisonCurrentPassenger 7

/* Passenger */
#define PassID 0
#define PassInspectorID 1
#define PassOfficerID 2
#define PassLiaisonID 3
#define PassCISID 4
#define PassFurtherQuestioning 5
#define PassNumBaggages 6
#define PassTicketExecutive 7
#define PassTicketAirline 8
#define PassTicketSeat 9

/* Baggage */
#define BaggageAirline 0
#define BaggageWeight 1

/* Queue */
#define QueueData 0
#define QueueRear 1
#define QueueFront 2

/* States */
#define AVAIL 0
#define BUSY 1
#define ONBREAK 2

typedef int bool;
enum bool {false, true};

/* Global People */
int passengers;
int airlines;
int cargoHandlers;
int manager;
int securityInspectors;
int screeningOfficers;
int liaisons;
int baggages;
int officersLine;
int conveyorBelt;

/* Initializing Data */
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

/* Data for Security Personel */
/*int SecurityFailResults;*/

/* Utility */
char concatString[100];

/* Function Declarations */
void doCreates();
void createGlobalData();
void createAirlines();
void createCIS(int airline);
void createCargoHandlers();
void createManager();
void createSecurityInspectors();
void createScreeningOfficers();
void createLiaisons();
void createBaggages();
void createPassengers();
char* concatNumToString(char* str, int length, int num);

/* Function Implementations */

void queue_insert (int queue, int index) {
    if (GetMV(queue, QueueRear) == NUM_PASSENGERS*3-1)
        Printf0("ERROR: QUEUE OVERFLOW\n", sizeof("ERROR: QUEUE OVERFLOW\n"));
    else {
/*        queue->_array[++rear] = index;*/
    	SetMV(GetMV(queue, QueueData), GetMV(queue, QueueRear), index);
    	SetMV(queue, QueueRear, GetMV(queue, QueueRear) + 1); /* increment rear*/
    }
}

int queue_pop (int queue) {
	int ret;
    if (GetMV(queue, QueueFront) >= GetMV(queue, QueueRear)) {
        Printf0("ERROR: QUEUE IS EMPTY\n", sizeof("ERROR: QUEUE IS EMPTY\n"));
        return -1;
    }
/*    return queue->_array[front++];*/
    ret = GetMV(GetMV(queue, QueueData), GetMV(queue, QueueFront));
    SetMV(queue, QueueFront, GetMV(queue, QueueFront) + 1);

    return ret;
}

int queue_size (int queue) {
    if (GetMV(queue, QueueFront) >= GetMV(queue, QueueRear)) return 0;
/*    return rear - front + 1;*/
    return GetMV(queue, QueueRear) - GetMV(queue, QueueFront);
}

bool queue_empty (int queue) {
    if (queue_size (queue) == 0) return true;
    else return false;
}

void doCreates() {
	int i;
	int passenger;

	createGlobalData();
	createManager();
	createCargoHandlers();
	createLiaisons();
	createBaggages();
	createSecurityInspectors();
	createScreeningOfficers();
/*	createAirlines();
	createPassengers();*/
}

void createGlobalData() {
	/* Initializing Data */
	NumActivePassengers = CreateMV(
		"NumActivePassengers",
		sizeof("NumActivePassengers"),
		1
		);
	NumActiveLiaisons = CreateMV(
		"NumActiveLiaisons",
		sizeof("NumActiveLiaisons"),
		1
		);
	NumActiveCIS = CreateMV(
		"NumActiveCIS",
		sizeof("NumActiveCIS"),
		1
		);
	NumActiveCargoHandlers = CreateMV(
		"NumActiveCargoHandlers",
		sizeof("NumActiveCargoHandlers"),
		1
		);
	NumActiveScreeningOfficers = CreateMV(
		"NumActiveScreeningOfficers",
		sizeof("NumActiveScreeningOfficers"),
		1
		);
	NumActiveSecurityInspectors = CreateMV(
		"NumActiveSecurityInspectors",
		sizeof("NumActiveSecurityInspectors"),
		1
		);

	/* Locks */
	GlobalDataLock = CreateLock(
		"GlobalDataLock",
		sizeof("GlobalDataLock")
		);
	LiaisonLineLock = CreateLock(
		"LiaisonLineLock",
		sizeof("LiaisonLineLock")
		);
	ConveyorLock = CreateLock(
		"ConveyorLock",
		sizeof("ConveyorLock")
		);
	OfficersLineLock = CreateLock(
		"OfficersLineLock",
		sizeof("OfficersLineLock")
		);
	InspectorLineLock = CreateLock(
		"InspectorLineLock",
		sizeof("InspectorLineLock")
		);

	/* CV */
	OfficersLineCV = CreateCV(
		"OfficersLineCV",
		sizeof("OfficersLineCV")
		);

	/* Data for Security Personel */
/*	SecurityFailResults = CreateMV(
		"SecurityFailResults",
		sizeof("SecurityFailResults"),
		NUM_PASSENGERS
		);*/

}

void createAirlines() {
	int i, j, airline, execQueue, temp;

	airlines = CreateMV("airlines", sizeof("airlines"), NUM_AIRLINES);
	for (i = 0; i < NUM_AIRLINES; ++i) {
		airline = CreateMV(
						concatNumToString("airline", sizeof("airline"), i),
						sizeof("airline") + 3,
						18
						);
		SetMV(airlines, i, airline);
		/* Init Airline */
		SetMV(airline, AirlineLock, CreateLock(
								concatNumToString(
									"airline_lock_", 
									sizeof("airline_lock_"), 
									i
								), 
								sizeof("airline_lock_") + 3)
							);
		SetMV(airline, AirlineExecLineLock, CreateLock(
								concatNumToString(
									"airline_exec_lock_", 
									sizeof("airline_exec_lock_"), 
									i
								), 
								sizeof("airline_exec_lock_") + 3)
							);
		SetMV(airline, AirlineExecLineCV, CreateCV(
								concatNumToString(
									"airline_execLineCV_", 
									sizeof("airline_execLineCV_"), 
									i
								), 
								sizeof("airline_execLineCV_") + 3)
							);
		SetMV(airline, AirlineBoardLoungeCV, CreateCV(
								concatNumToString(
									"airline_boardLoungeCV_", 
									sizeof("airline_boardLoungeCV_"), 
									i
								), 
								sizeof("airline_boardLoungeCV_") + 3)
							);
		SetMV(airline, AirlineExecLineLock, CreateLock(
								concatNumToString(
									"airline_CIS_lock_", 
									sizeof("airline_CIS_lock_"), 
									i
								), 
								sizeof("airline_CIS_lock_") + 3)
							);
		SetMV(airline, AirlineCISLineLock, CreateLock(
								concatNumToString(
									"airline_CIS_lock_", 
									sizeof("airline_CIS_lock_"), 
									i
								), 
								sizeof("airline_CIS_lock_") + 3)
							);
		createCIS(airline);

		/* Queue */
		execQueue = CreateMV(
						concatNumToString(
							"airlineExecQueue",
							sizeof("airlineExecQueue"),
							i
							),
						sizeof("airlineExecQueue") + 3,
						3
			);
		SetMV(airline, AirlineExecQueue, execQueue);

		/* Queue Data */
		temp = CreateMV(
			concatNumToString(
				"airExecQData",
				sizeof("airExecQData"),
				i
				),
			sizeof("airExecQData") + 3,
			NUM_PASSENGERS
			);
		for (j = 0; j < NUM_PASSENGERS; ++j) {
			SetMV(temp, j, -1); /* TODO - Make sure this works by printing out this -1 */
		}
		SetMV(execQueue, QueueData, temp);
		/* Front Pointer */
		SetMV(execQueue, QueueFront, 0);
		/* Rear Pointer */
		SetMV(execQueue, QueueRear, 0);

		/* TODO - do we need this here?? */
		/* Implement Queue first... */
		/*a._execQueue._front = -1;
        a._execQueue._rear = -1;*/

		SetMV(airline, AirlineNumCheckedinPassengers, 0);
		SetMV(airline, AirlineNumReadyPassengers, 0);
		SetMV(airline, AirlineNumExpectedBaggages, 0);
		SetMV(airline, AirlineNumLoadedBaggages, 0);
		SetMV(airline, AirlineNumOnBreakCIS, 0);
		SetMV(airline, AirlineBoarded, 0);
		SetMV(airline, AirlineAllPassengersCheckedIn, 0);
		SetMV(airline, AirlineCISClosed, 0);
	}
}

void createCIS(int airline) {
	int i, cis, cisArray, temp;
	cisArray = CreateMV("CIS", 
						sizeof("CIS"), 
						NUM_CIS_PER_AIRLINE
						);
	SetMV(airline, AirlineCIS, cisArray);
	for (i = 0; i < NUM_CIS_PER_AIRLINE; ++i) {
		cis = CreateMV(concatNumToString("cis", sizeof("cis"), i), sizeof("cis") + 3, 11);
		SetMV(cisArray, i, cis);
		/* Init CIS */

		temp = CreateLock(
				concatNumToString(
						"CIS_lock_", 
						sizeof("CIS_lock_"), 
						i
					), 
				sizeof("CIS_lock_") + 3
			);
		SetMV(cis, CISLock, temp);

		temp = CreateCV(
				concatNumToString(
					"CIS_lineCV_", 
					sizeof("CIS_lineCV_"), 
					i
				), 
				sizeof("CIS_lineCV_") + 3
			);
		SetMV(cis, CISLineCV, temp);

		temp = CreateCV(
				concatNumToString(
					"CIS_commCV_", 
					sizeof("CIS_commCV_"), 
					i
				), 
				sizeof("CIS_commCV_") + 3
			);
		SetMV(cis, CISCommCV, temp);

		SetMV(cis, CISLineSize, 0);
		SetMV(cis, CISState, BUSY);
		SetMV(cis, CISPassCount, 0);
		SetMV(cis, CISBagCount, 0);
		SetMV(cis, CISWeightCount, 0);
		SetMV(cis, CISCurrentPassenger, -1);
		/* TODO - Print out GetMV(cis, CISCurrentPassenger) to make sure we can use negative numbers */
		SetMV(cis, CISAirline, airline);
		SetMV(cis, CISDone, 0);
	}
}

void createCargoHandlers() {
	int i, ch, temp;

	/* ConveyorBelt */
	conveyorBelt = CreateMV(
		"conveyBelt",
		sizeof("conveyBelt"),
		3
		); 
	/* Data */
	temp = CreateMV(
		"conveyBeltData",
		sizeof("conveyBeltData"),
		NUM_PASSENGERS*3
		);
	for (i = 0; i < NUM_PASSENGERS*3; ++i) {
		SetMV(temp, i, -1);
	}
	SetMV(conveyorBelt, QueueData, temp);
	/* Queue Front */
	SetMV(conveyorBelt, QueueRear, 0);
	/* Queue Read */
	SetMV(conveyorBelt, QueueFront, 0);


	/* Cargo Handlers */
	cargoHandlers = CreateMV(
						"cargoHandlers",
						sizeof("cargoHandlers"),
						NUM_CARGO_HANDLERS
					);
	for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
		/* 
		*	Create Cargo Handler 
		*/
		ch = CreateMV(
				concatNumToString(
					"cargoHandler",
					sizeof("cargoHandler"),
					i),
				sizeof("cargoHandler") + 3,
				4
			);
		SetMV(cargoHandlers, i, ch);

		/* 
		*	Init Cargo Handler 
		*/
		/* CHCommCV */
		temp = CreateCV(
					concatNumToString(
						"CHCommCV",
						sizeof("CHCommCV"),
						i
					),
					sizeof("CHCommCV") + 3
				);
		SetMV(ch, CHCommCV, temp);

		/* CHState */
		SetMV(ch, CHState, BUSY);

		/* CHBagCount */
		temp = CreateMV(
					concatNumToString(
						"CHBagCount",
						sizeof("CHBagCount"),
						i
					),
					sizeof("CHBagCount") + 3,
					NUM_AIRLINES
				);
		SetMV(ch, CHBagCount, BUSY);

		/* CHWeightCount */
		temp = CreateMV(
					concatNumToString(
						"CHWeightCount",
						sizeof("CHWeightCount"),
						i
					),
					sizeof("CHWeightCount") + 3,
					NUM_AIRLINES
				);
		SetMV(ch, CHWeightCount, BUSY);
	}
}

void createManager() {
	manager = CreateMV(
				"manager",
				sizeof("manager"),
				5
			);
	SetMV(manager, ManAllLiaisonDone, 0);
	SetMV(manager, ManAllCISDone, 0);
	SetMV(manager, ManAllCargoDone, 0);
	SetMV(manager, ManAllSODone, 0);
	SetMV(manager, ManAllSIDone, 0);
}

void createSecurityInspectors() {
	int i, temp, si, result;

	/* Init MV for all SIs */
	securityInspectors = CreateMV(
							"securityInspectors",
							sizeof("securityInspectors"),
							NUM_SECURITY_INSPECTORS
						);

	/* Create all SIs */
	for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
		/* 
		*	Create SI 
		*/
		si = CreateMV(
					concatNumToString(
						"SI",
						sizeof("SI"),
						i
					),
					sizeof("SI") + 3,
					10
				);
		/* Add SI to array of SIs */
		SetMV(securityInspectors, i, si);
		
		/*
		*	Init SI
		*/
		/* SIID */
		SetMV(si, SIID, si); /* ID is MV ID */
		/* SIRtnPassSize */
		SetMV(si, SIRtnPassSize, 0);
		/* SIState */
		SetMV(si, SIState, BUSY);
		/* SILock */
		temp = CreateLock(
					concatNumToString(
						"SILock",
						sizeof("SILock"),
						i
					),
					sizeof("SILock") + 3
				);
		SetMV(si, SILock, temp);
		/* SICommCV */
		temp = CreateCV(
					concatNumToString(
						"SICommCV",
						sizeof("SICommCV"),
						i
					),
					sizeof("SICommCV") + 3
				);
		SetMV(si, SICommCV, temp);
		/* SIRtnPassCV */
		temp = CreateCV(
					concatNumToString(
						"SIRtnPassCV",
						sizeof("SIRtnPassCV"),
						i
					),
					sizeof("SIRtnPassCV") + 3
				);
		SetMV(si, SIRtnPassCV, temp);
		/* SINewPassCV */
		temp = CreateCV(
					concatNumToString(
						"SINewPassCV",
						sizeof("SINewPassCV"),
						i
					),
					sizeof("SINewPassCV") + 3
				);
		result = SetMV(si, SINewPassCV, temp);
		Printf1("1: %d\n", sizeof("1: %d\n"), si);
		/* SIRtnPassenger */
		result = SetMV(si, SIRtnPassenger, -1);
		Printf1("2: %d\n", sizeof("1: %d\n"), si);
		/* SINewPassenger */
		result = SetMV(si, SINewPassenger, -1);
		Printf1("3: %d\n", sizeof("1: %d\n"), si);
		/* SIPassCount */
		result = SetMV(si, SIPassCount, 0);
		Printf1("4: %d\n", sizeof("1: %d\n"), si);
	}
}

void createScreeningOfficers() {
	int i, so, temp;



	/* officersLine Queue */
	officersLine = CreateMV(
		"officersLine",
		sizeof("officersLine"),
		3
		); 
	/* Data */
	temp = CreateMV(
		"officersLineData",
		sizeof("officersLineData"),
		NUM_PASSENGERS
		);
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		SetMV(temp, i, -1);
	}
	SetMV(conveyorBelt, QueueData, temp);
	/* Queue Front */
	SetMV(conveyorBelt, QueueRear, 0);
	/* Queue Read */
	SetMV(conveyorBelt, QueueFront, 0);




	/* Create Array of Screening Officers (SO) */
	screeningOfficers = CreateMV(
							"screeningOfficers",
							sizeof("screeningOfficers"),
							NUM_SCREENING_OFFICERS
						);
	/* Create all SO */
	for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		/*
		*	Create SO
		*/
		so = CreateMV(
					concatNumToString(
						"SO",
						sizeof("SO"),
						i
					),
					sizeof("SO") + 3,
					7
				);
		/* Add SO to array of SIs */
		SetMV(screeningOfficers, i, so);

		/*
		*	Init SO
		*/
		/* SOLock */
		temp = CreateLock(
					concatNumToString(
						"SOLock",
						sizeof("SOLock"),
						i
					),
					sizeof("SOLock") + 3
				);
		SetMV(so, SOLock, temp);

		/* SOCommCV */
		temp = CreateCV(
					concatNumToString(
						"SOCommCV",
						sizeof("SOCommCV"),
						i
					),
					sizeof("SOCommCV") + 3
				);
		SetMV(so, SOCommCV, temp);

		/* SOState */
		SetMV(so, SOState, BUSY);

		/* SOPassCount */
		SetMV(so, SOPassCount, 0);

		/* SOCurrentPassenger */
		SetMV(so, SOCurrentPassenger, -1);

		/* SOMyNum */
		SetMV(so, SOMyNum, so);

		/* SODone */
		SetMV(so, SODone, 0);
	}
}

void createLiaisons() {
	int i, liason, temp;

	/* Create Array of Screening Officers (SO) */
	liaisons = CreateMV(
						"liaisons",
						sizeof("liaisons"),
						NUM_LIASONS
					);

	/* Create all Liaisons */
	for (i = 0; i < NUM_LIASONS; ++i) {
		/*
		*	Create Liaison
		*/
		liason = CreateMV(
					concatNumToString(
						"liason",
						sizeof("liason"),
						i
					),
					sizeof("liason") + 3,
					8
				);
		/* Add to Liaison array */
		SetMV(liaisons, i, liason);

		/*
		*	Init Liaison
		*/

		/* LiaisonLock */
		temp = CreateLock(
					concatNumToString(
						"LiaisonLock",
						sizeof("LiaisonLock"),
						i
					),
					sizeof("LiaisonLock") + 3
				);
		SetMV(liason, LiaisonLock, temp);

		/* LiaisonLineCV */
		temp = CreateCV(
					concatNumToString(
						"LiaisonLineCV",
						sizeof("LiaisonLineCV"),
						i
					),
					sizeof("LiaisonLineCV") + 3
				);
		SetMV(liason, LiaisonLineCV, temp);

		/* LiaisonCommCV */
		temp = CreateCV(
					concatNumToString(
						"LiaisonCommCV",
						sizeof("LiaisonCommCV"),
						i
					),
					sizeof("LiaisonCommCV") + 3
				);
		SetMV(liason, LiaisonCommCV, temp);

		/* LiaisonLineSize */
		SetMV(liason, LiaisonLineSize, 0);

		/* LiaisonState */
		SetMV(liason, LiaisonState, BUSY);

		/* LiaisonPassCount */
		SetMV(liason, LiaisonPassCount, 0);

		/* LiaisonBagCount */
		SetMV(liason, LiaisonBagCount, 0);

		/* LiaisonCurrentPassenger */
		SetMV(liason, LiaisonCurrentPassenger, -1);

	}
}

void createBaggages() {
	int i;
	/* Create array of baggages */
	baggages = CreateMV(
					"baggages",
					sizeof("baggages"),
					NUM_PASSENGERS * 3
				);
	for (i = 0; i < NUM_PASSENGERS * 3; ++i) {
		SetMV(baggages, i, -1);
	}
}

void createPassengers() {
	int i, j, pass, temp, numBags, bag, bagWeight, airline, isExec;

	/* Create Passengers array */
	passengers = CreateMV(
					"Passengers",
					sizeof("Passengers"),
					NUM_PASSENGERS
				);

	/* Create Passengers */
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		/*
		*	Create Passenger
		*/
		pass = CreateMV(
					concatNumToString(
						"Passenger",
						sizeof("Passenger"),
						i
					),
					sizeof("Passenger") + 3,
					10
				);

		/*
		*	Init Passenger
		*/

		/* PassID */
		SetMV(pass, PassID, pass);

		/* PassInspectorID */
		SetMV(pass, PassInspectorID, -1);

		/* PassOfficerID */
		SetMV(pass, PassOfficerID, -1);

		/* PassLiaisonID */
		SetMV(pass, PassLiaisonID, -1);

		/* PassCISID */
		SetMV(pass, PassCISID, -1);

		/* PassFurtherQuestioning */
		SetMV(pass, PassFurtherQuestioning, 0);

		/* PassNumBaggages */
		numBags = i % 2 + 2;
		SetMV(pass, PassNumBaggages, numBags);

		/* 
		*	Ticket 
		*/

		/* Ticket--Airline */
		airline = (i*17) % NUM_AIRLINES;
		SetMV(pass, PassTicketAirline, airline);
		/* airline->NumExpectedPasenger++ */
		SetMV( 
			GetMV(airlines, airline), 
			AirlineNumExpectedPassenger, 
			GetMV(GetMV(airlines, airline), AirlineNumExpectedPassenger) + 1 
		);
		/* Ticket--Exec */
		if ((i % 4) == 1) {
			isExec = 1;
		} else {
			isExec = 0;
		}
		SetMV(pass, PassTicketExecutive, isExec);
		
		/* Init Baggages */
		for (j = 0; j < numBags; ++j) {
			bag = CreateMV(
						concatNumToString(
							"bag",
							sizeof("bag"),
							i * 10 + j
						),
						sizeof("bag") + 3,
						2
					);
			SetMV(baggages, (i * 3) + j, bag);
			/* Set Bag Variables */
			SetMV(bag, BaggageAirline, airline); /* Airline */
			bagWeight = (i * 13) % 31 + 30;
			SetMV(bag, BaggageWeight, bagWeight); /* Weight */
			/* airline->NumExpectedBaggages++ */
			SetMV( 
				GetMV(airlines, airline), 
				AirlineNumExpectedBaggages, 
				GetMV(GetMV(airlines, airline), AirlineNumExpectedBaggages) + 1 
			);
			/* airline->AirlineWeightCount++ */
			SetMV( 
				GetMV(airlines, airline),
				AirlineWeightCount,
				GetMV(GetMV(airlines, airline), AirlineWeightCount) + bagWeight
			);
		}
	}
}

char* concatNumToString(char* str, int length, int num) { /* TODO - Not working Properly */
	int i;
	for (i=0; i < length - 1; i++) {
		concatString[i] = str[i];
	}
	/* hundreds place */
	concatString[length - 1] = '0' + num / 100;
	/* tens place */
	concatString[length] = '0' + (num % 100) / 10;
	/* one's place */
	concatString[length + 1] = '0' + num % 10;
	/* null terminator */
	concatString[length + 2] = '\0';

	Printf0(concatString, length + 5);

	return concatString;
}





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

/* States */
#define AVAIL 0
#define BUSY 1
#define ONBREAK 2

int passengers;
int airlines;
int cargoHandlers;
char concatString[100];

/* Function Declarations */

void doCreates();
void createAirlines();
void createCIS(int airline);
void createCargoHandlers();
char* concatNumToString(char* str, int length, int num);

/* Function Implementations */

void doCreates() {
	int i;
	int passenger;

	createAirlines();
	createCargoHandlers();


	/* Passengers */
	passengers = CreateMV("passengers", sizeof("passengers"), NUM_PASSENGERS);
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		passenger = CreateMV(
						concatNumToString("passenger", sizeof("passenger"), i),
						sizeof("passenger") + 2,
						10
						);
		/*SetMV(passengers, i, passenger);*/

	}
}

void createAirlines() {
	int i, j, airline, execQueue;

	airlines = CreateMV("airlines", sizeof("airlines"), NUM_AIRLINES);
	for (i = 0; i < NUM_AIRLINES; ++i) {
		airline = CreateMV(
						concatNumToString("airline", sizeof("airline"), i),
						sizeof("airline") + 2,
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
								sizeof("airline_lock_") + 2)
							);
		SetMV(airline, AirlineExecLineLock, CreateLock(
								concatNumToString(
									"airline_exec_lock_", 
									sizeof("airline_exec_lock_"), 
									i
								), 
								sizeof("airline_exec_lock_") + 2)
							);
		SetMV(airline, AirlineExecLineCV, CreateCV(
								concatNumToString(
									"airline_execLineCV_", 
									sizeof("airline_execLineCV_"), 
									i
								), 
								sizeof("airline_execLineCV_") + 2)
							);
		SetMV(airline, AirlineBoardLoungeCV, CreateCV(
								concatNumToString(
									"airline_boardLoungeCV_", 
									sizeof("airline_boardLoungeCV_"), 
									i
								), 
								sizeof("airline_boardLoungeCV_") + 2)
							);
		SetMV(airline, AirlineExecLineLock, CreateLock(
								concatNumToString(
									"airline_CIS_lock_", 
									sizeof("airline_CIS_lock_"), 
									i
								), 
								sizeof("airline_CIS_lock_") + 2)
							);
		SetMV(airline, AirlineCISLineLock, CreateLock(
								concatNumToString(
									"airline_CIS_lock_", 
									sizeof("airline_CIS_lock_"), 
									i
								), 
								sizeof("airline_CIS_lock_") + 2)
							);
		createCIS(airline);

		execQueue = CreateMV(
						concatNumToString(
							"airlineExecQueue",
							sizeof("airlineExecQueue"),
							i
							),
						sizeof("airlineExecQueue") + 2,
						NUM_PASSENGERS
			);
		for (j = 0; j < NUM_PASSENGERS; ++j) {
			SetMV(execQueue, j, -1); /* TODO - Make sure this works by printing out this -1 */
		}

		SetMV(airline, AirlineExecQueue, execQueue);

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
		cis = CreateMV(concatNumToString("cis", sizeof("cis"), i), sizeof("cis") + 2, 11);
		SetMV(cisArray, i, cis);
		/* Init CIS */

		temp = CreateLock(
				concatNumToString(
						"CIS_lock_", 
						sizeof("CIS_lock_"), 
						i
					), 
				sizeof("CIS_lock_") + 2
			);
		SetMV(cis, CISLock, temp);

		temp = CreateCV(
				concatNumToString(
					"CIS_lineCV_", 
					sizeof("CIS_lineCV_"), 
					i
				), 
				sizeof("CIS_lineCV_") + 2
			);
		SetMV(cis, CISLineCV, temp);

		temp = CreateCV(
				concatNumToString(
					"CIS_commCV_", 
					sizeof("CIS_commCV_"), 
					i
				), 
				sizeof("CIS_commCV_") + 2
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
	int i, ch, tempMV;
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
					"cargoHandler"
					sizeof("cargoHandler"),
					i),
				sizeof("cargoHandler") + 2,
				4
			);
		SetMV(cargoHandlers, i, ch);

		/* 
		*	Init Cargo Handler 
		*/
		tempMV = CreateCV(
					concatNumToString(
						"CHCommCV",
						sizeof("CHCommCV"),
						i
					),
					sizeof("CHCommCV") + 2
				);
		SetMV(ch, CHCommCV, tempMV);

		SetMV(ch, CHState, BUSY);

		tempMV = CreateMV(
					concatNumToString(
						"CHBagCount"
						sizeof("CHBagCount"),
						i
					),
					sizeof("CHBagCount") + 2,
					NUM_AIRLINES
				);
		SetMV(ch, CHBagCount, BUSY);

		tempMV = CreateMV(
					concatNumToString(
						"CHWeightCount"
						sizeof("CHWeightCount"),
						i
					),
					sizeof("CHWeightCount") + 2,
					NUM_AIRLINES
				);
		SetMV(ch, CHWeightCount, BUSY);
	}
}

char* concatNumToString(char* str, int length, int num) { /* TODO - Not working Properly */
	int i;
	for (i=0; i < length - 1; i++) {
		concatString[i] = str[i];
	}
	if (num >= 10) {
		concatString[length - 1] = '0' + num/10;
	} else {
		concatString[length - 1] = '0';
	}
	concatString[length] = '0' + num % 10;
	return concatString;
}
#include "create.h"

#define NULL 0

/*
typedef int bool;
enum bool {false, true};
*/

/*
	Utilities	
*/

int concat3Num(int i, int j, int k) {
	return 1000000 * i + 1000 * j + k;
} 

int concat2Num(int i, int j) {
	return 1000 * i + j;
}


void startCheckInStaff() {
#define myAirline GetMV(airlines, _myAirline);
#define my GetMV(GetMV(GetMV(airlines, _myAirline), AirlineCIS), _myIndex);
#define passenger GetMV(passengers, GetMV(my, CISCurrentPassenger))
	/* Claim my CIS */
	int _myAirline;
	int _myIndex; /* ID for currentThread */
    Acquire(GlobalDataLock);
	_myIndex = GetMV(NumActiveCIS, 0) % NUM_CIS_PER_AIRLINE;
	_myAirline = GetMV(NumActiveCIS, 0) / NUM_CIS_PER_AIRLINE;
    IncrementMV(NumActiveCIS, 0);
    Release(GlobalDataLock);

    while (true) {
		/* Check lines */
		Acquire(GetMV(myAirline, AirlineLock);
		if (GetMV(my, CISLIneSize) == 0 && queue_empty(GetMV(myAirline, AirlineExecQueue))) {
			/*Printf0("1\n", sizeof("1\n"));*/
			SetMV(my, CISState, ONBREAK);
			/* 'Clock Out' for Break */
			IncrementMV(myAirline, AirlineNumOnBreakCIS);
/*Printf1("Cis %d going to sleep\n", sizeof("Cis %d going to sleep\n"), _myIndex);*/
			Wait(GetMV(myAirline, AirlineLock), GetMV(my, CISCommCV)); /* Wait on Manager */ /* TODO - make sure okay to wait on aiport lock... maybe better? */
/*Printf1("Cis %d woke up by manager\n", sizeof("Cis %d woke up by manager\n"), _myIndex);*/
			/* Time to go home! TGIF! */
			if (GetMV(my, CISDone)) {
				Printf1("Airline check-in staff %d of airline %d is closing the counter\n",
					sizeof("Airline check-in staff %d of airline %d is closing the counter\n"),
					concat2Num(_myIndex, _myAirline));
/*				Acquire(GetMV(my, CISLock));*/
				Release(GetMV(myAirline, AirlineLock));
/*				Wait(GetMV(my, CISLock), GetMV(my, CISCommCV)); *//* Wait forever, basically */
/*				Release(GetMV(my, CISLock)); *//* Never reaches here, but whatever... */
				break;
			}
			myAirline._numOnBreakCIS--;
		}
		/* Start helping a passenger */
		SetMV(my, CISState, BUSY);
		Acquire(GetMV(myAirline, AirlineCISLineLock));
		Acquire(GetMV(myAirline, AirlineExecLineLock));
		if (queue_size(GetMV(myAirline, AirlineExecQueue)) > 0) {
			SetMV(my, CISCurrentPassenger, queue_pop(GetMV(myAirline, AirlineExecQueue)));
			passenger._cisID = _myIndex;
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concat3Num(_myIndex, _myAirline, GetMV(my, CISLineSize)));
			Signal(GetMV(myAirline, AirlineExecLineLock), GetMV(myAirline, AirlineExecLineCV)); /* Signal Passenger */ 
		} else if (GetMV(my, CISLineSize) > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concat3Num(_myIndex, _myAirline, queue_size(GetMV(myAirline, AirlineExecQueue))));
			Signal(GetMV(myAirline, AirlineCISLineLock), GetMV(my, CISLine));
/*Printf1("Cis %d of airline %d wakes up passenger\n", sizeof("Cis %d of airline %d wakes up passenger\n"), _myIndex*1000+_myAirline);*/
		}
		/* Interact with Passenger */
		Acquire(GetMV(my, CISLock));
		Release(GetMV(myAirline, AirlineCISLineLock));
		Release(GetMV(myAirline, AirlineExecLineLock));
		Release(GetMV(myAirline, AirlineLock));
		if (GetMV(my, CISLineSize) > 0 || GetMV(my, CISCurrentPassenger) != -1) {
/*Printf1("Cis %d of airline %d goes to sleep\n", sizeof("Cis %d of airline %d goes to sleep\n"), _myIndex*1000+_myAirline);*/
			Wait(GetMV(my, CISLock), GetMV(my, CISCommCV)); 
		} /* Otherwise, Manager woke you up for no reason */
		if (GetMV(my, CISCurrentPassenger) != -1) {
			int i;
			/* Assign seat number */
			Acquire(GetMV(myAirline, AirlineLock));
			SetMV(passenger, PassengerTicketSeat, GetMV(myAirline, AirlineNumCheckedinPassengers));
			IncrementMV(myAirline, AirlineNumCheckedinPassengers);
			Release(GetMV(myAirline, AirlineLock));
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
/*				myAirline._numExpectedBaggages++;*/
				GetMV(my, CISWeightCount) += bag._weight;
				#undef bag
/*				#undef bIndex*/
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (passenger._ticket._executive) {
				Printf2("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, GetMV(my, CISCurrentPassenger), _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, GetMV(my, CISCurrentPassenger)), _myAirline);
			} else {
				Printf2("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, GetMV(my, CISCurrentPassenger), _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, GetMV(my, CISCurrentPassenger)), _myAirline);
			}
			Signal(GetMV(my, CISLock), GetMV(my, CISCommCV)); 
			Wait(GetMV(my, CISLock), GetMV(my, CISCommCV)); 
		}
		SetMV(my, CISCurrentPassenger, -1);
		Release(GetMV(my, CISLock));
	} /* end while */
	Exit(0);
#undef passenger
#undef my
#undef myAirline
}

int main() {
	doCreates();
    startCheckInStaff();
}
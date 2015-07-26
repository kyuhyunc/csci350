#include "create.h"


void startCheckInStaff() {
	/* Claim my CIS */
	int _myAirlineIndex;
	int _myAirlineMV;
	int _myIndex; /* ID for currentThread */
	int _myMV;
	int passenger;
	
    Acquire(GlobalDataLock);
	_myIndex = GetMV(NumActiveCIS, 0) % NUM_CIS_PER_AIRLINE;
	_myAirlineIndex = GetMV(NumActiveCIS, 0) / NUM_CIS_PER_AIRLINE;
/*	_myMV = GetMV(GetMV(_myAirline, AirlineCIS), _myIndex);*/
	_myAirlineMV = GetMV(airlines, _myAirlineIndex);
    _myMV = GetMV(GetMV(_myAirlineMV, AirlineCIS), _myIndex);
    incrementMV(NumActiveCIS, 0);
    Release(GlobalDataLock);
/*Printf1("NumActiveCIS = %d\n", sizeof("NumActiveCIS = %d\n"), NumActiveCIS);
Printf1("_myIndex = %d\n", sizeof("_myIndex = %d\n"), _myIndex);
Printf1("_myAirlineIndex = %d\n", sizeof("_myAirlineIndex = %d\n"), _myAirlineIndex);
Printf1("_myMV = %d\n", sizeof("_myMV = %d\n"), _myMV);
Printf1("_myAirlineMV = %d\n", sizeof("_myAirlineMV = %d\n"), _myAirlineMV);*/

    while (true) {
		/* Check lines */
		Acquire(GetMV(_myAirlineMV, AirlineLock));
		if (GetMV(_myMV, CISLineSize) == 0 && queue_empty(GetMV(_myAirlineMV, AirlineExecQueue))) {
			/*Printf0("1\n", sizeof("1\n"));*/
			SetMV(_myMV, CISState, ONBREAK);
			/* 'Clock Out' for Break */
			incrementMV(_myAirlineMV, AirlineNumOnBreakCIS);
Printf1("Airline %d Cis %d going to sleep\n", sizeof("Airline %d Cis %d going to sleep\n"), concat2Num(_myAirlineIndex, _myIndex));
			Wait(GetMV(_myAirlineMV, AirlineLock), GetMV(_myMV, CISCommCV)); /* Wait on Manager */ /* TODO - make sure okay to wait on aiport lock... maybe better? */
Printf1("Airline %d Cis %d woke up by manager\n", sizeof("Airline %d Cis %d woke up by manager\n"), concat2Num(_myAirlineIndex,  _myIndex));
			/* Time to go home! TGIF! */
			if (GetMV(_myMV, CISDone)) {
				Printf1("Airline check-in staff %d of airline %d is closing the counter\n",
					sizeof("Airline check-in staff %d of airline %d is closing the counter\n"),
					concat2Num(_myIndex, _myAirlineIndex));
/*				Acquire(GetMV(my, CISLock));*/
				Release(GetMV(_myAirlineMV, AirlineLock));
/*				Wait(GetMV(my, CISLock), GetMV(my, CISCommCV)); *//* Wait forever, basically */
/*				Release(GetMV(my, CISLock)); *//* Never reaches here, but whatever... */
				break;
			}
			decrementMV(_myAirlineMV, AirlineNumOnBreakCIS);
		}
		/* Start helping a passenger */
		SetMV(_myMV, CISState, BUSY);
		Acquire(GetMV(_myAirlineMV, AirlineCISLineLock));
		Acquire(GetMV(_myAirlineMV, AirlineExecLineLock));
		if (queue_size(GetMV(_myAirlineMV, AirlineExecQueue)) > 0) {
			passenger = queue_pop( GetMV(_myAirlineMV, AirlineExecQueue) );
			SetMV(_myMV, CISCurrentPassenger, passenger);
			/*passenger._cisID = _myIndex;*/
			SetMV(passenger, PassCISID, _myMV);
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concat3Num(_myIndex, _myAirlineIndex, GetMV(_myMV, CISLineSize)));
			Signal(GetMV(_myAirlineMV, AirlineExecLineLock), GetMV(_myAirlineMV, AirlineExecLineCV)); /* Signal Passenger */ 
		} else if (GetMV(_myMV, CISLineSize) > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concat3Num(_myIndex, _myAirlineIndex, queue_size(GetMV(_myAirlineMV, AirlineExecQueue))));
			Signal(GetMV(_myAirlineMV, AirlineCISLineLock), GetMV(_myMV, CISLineCV));
/*Printf1("Cis %d of airline %d wakes up passenger\n", sizeof("Cis %d of airline %d wakes up passenger\n"), _myIndex*1000+_myAirlineIndex);*/
		}
		/* Interact with Passenger */
/*(Printf1("Airline %d cis %d before Acquire(GetMv(_myMV, CISLock))\n", sizeof("Airline %d cis %d before Acquire(GetMv(_myMV, CISLock))\n"), concat2Num(_myAirlineIndex, _myIndex));*/
		Acquire(GetMV(_myMV, CISLock));
/*Printf1("Airline %d cis %d after Acquire(GetMv(_myMV, CISLock))\n", sizeof("Airline %d cis %d after Acquire(GetMv(_myMV, CISLock))\n"), concat2Num(_myAirlineIndex, _myIndex));*/
		Release(GetMV(_myAirlineMV, AirlineCISLineLock));
		Release(GetMV(_myAirlineMV, AirlineExecLineLock));
		Release(GetMV(_myAirlineMV, AirlineLock));
		if (GetMV(_myMV, CISLineSize) > 0 || GetMV(_myMV, CISCurrentPassenger) != -1) {
Printf1("Cis %d of airline %d goes to sleep\n", sizeof("Cis %d of airline %d goes to sleep\n"), _myIndex*1000+_myAirlineIndex);
			Wait(GetMV(_myMV, CISLock), GetMV(_myMV, CISCommCV)); 
Printf1("Cis %d of airline %d woke up\n", sizeof("Cis %d of airline %d woke up\n"), _myIndex*1000+_myAirlineIndex);

		} /* Otherwise, Manager woke you up for no reason */
        passenger = GetMV(_myMV, CISCurrentPassenger);
		if (passenger != -1) {
			int i;
			/* Assign seat number */
/*Printf1("A %d cis %d before Acquire(GetMV(_myAirlineMV, AirlineLock)", sizeof("A %d cis %d before Acquire(GetMV(_myAirlineMV, AirlineLock)"), concat2Num(_myAirlineIndex, _myIndex));*/
			Acquire(GetMV(_myAirlineMV, AirlineLock));
/*Printf1("A %d cis %d after Acquire(GetMV(_myAirlineMV, AirlineLock)", sizeof("A %d cis %d after Acquire(GetMV(_myAirlineMV, AirlineLock)"), concat2Num(_myAirlineIndex, _myIndex));*/
			SetMV(passenger, PassTicketSeat, GetMV(_myAirlineMV, AirlineNumCheckedinPassengers));
			incrementMV(_myAirlineMV, AirlineNumCheckedinPassengers);
			Release(GetMV(_myAirlineMV, AirlineLock));
			/* Deal with baggage */
			Acquire(ConveyorLock);
			for (i = 0; i < GetMV(passenger, PassNumBaggages); ++i) {
				/*Printf0("3\n", sizeof("1\n"));*/
/*				#define bIndex (passenger._id * 3) + i
				#define bag Baggages[bIndex]*/
				/*#define bag Baggages[(passenger._id*3)+i]*/
				SetMV(
					GetMV(
						baggages,
						GetMV(passenger, PassIndex) * 3 + i
						),
					BaggageAirline,
					_myAirlineIndex
					);
/*				queue_insert(&ConveyorBelt, bIndex);*/
				queue_insert(conveyorBelt, GetMV(passenger, PassIndex)*3+i);
				Printf1("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n",
					sizeof("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n"),
					concat2Num(_myIndex, _myAirlineIndex));
/*				myAirline._numExpectedBaggages++;*/
				SetMV(
					_myMV,
					CISWeightCount, 
					GetMV(_myMV, CISWeightCount) + GetMV(GetMV(baggages, GetMV(passenger, PassIndex) * 3 + i), BaggageWeight));
				/*#undef bag*/
/*				#undef bIndex*/
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (GetMV(passenger, PassTicketExecutive)) {
				Printf2("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirlineIndex, GetMV(my, CISCurrentPassenger), _myAirlineIndex), _myIndex);*/
					concat3Num(_myIndex, _myAirlineIndex, GetMV(GetMV(_myMV, CISCurrentPassenger), PassIndex)), _myAirlineIndex);
			} else {
				Printf2("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirlineIndex, GetMV(my, CISCurrentPassenger), _myAirlineIndex), _myIndex);*/
					concat3Num(_myIndex, _myAirlineIndex, GetMV(_myMV, CISCurrentPassenger)), _myAirlineIndex);
			}
			Signal(GetMV(_myMV, CISLock), GetMV(_myMV, CISCommCV)); 
			Wait(GetMV(_myMV, CISLock), GetMV(_myMV, CISCommCV)); 
		}
		SetMV(_myMV, CISCurrentPassenger, -1);
		Release(GetMV(_myMV, CISLock));
	} /* end while */
    Printf1("Airline check-in staff %d of airline %d is going home\n",
					sizeof("Airline check-in staff %d of airline %d is going home\n"),
					concat2Num(_myIndex, _myAirlineIndex));
	Exit(0);
}

int main() {
	doCreates();
    startCheckInStaff();
}

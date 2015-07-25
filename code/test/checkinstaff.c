#include "create.h"


void startCheckInStaff() {
#define myAirlineMACRO GetMV(airlines, _myAirline)
#define passengerMACRO GetMV(_myMV, CISCurrentPassenger)
	/* Claim my CIS */
	int _myAirline;
	int _myIndex; /* ID for currentThread */
	int _myMV;
	
    Acquire(GlobalDataLock);
	_myIndex = GetMV(NumActiveCIS, 0) % NUM_CIS_PER_AIRLINE;
	_myAirline = GetMV(NumActiveCIS, 0) / NUM_CIS_PER_AIRLINE;
	_myMV = GetMV(GetMV(_myAirline, AirlineCIS), _myIndex);
    incrementMV(NumActiveCIS, 0);
    Release(GlobalDataLock);

    while (true) {
		/* Check lines */
		Acquire(GetMV(myAirlineMACRO, AirlineLock));
		if (GetMV(_myMV, CISLineSize) == 0 && queue_empty(GetMV(myAirlineMACRO, AirlineExecQueue))) {
			/*Printf0("1\n", sizeof("1\n"));*/
			SetMV(_myMV, CISState, ONBREAK);
			/* 'Clock Out' for Break */
			IncrementMV(myAirlineMACRO, AirlineNumOnBreakCIS);
/*Printf1("Cis %d going to sleep\n", sizeof("Cis %d going to sleep\n"), _myIndex);*/
			Wait(GetMV(myAirlineMACRO, AirlineLock), GetMV(_myMV, CISCommCV)); /* Wait on Manager */ /* TODO - make sure okay to wait on aiport lock... maybe better? */
/*Printf1("Cis %d woke up by manager\n", sizeof("Cis %d woke up by manager\n"), _myMACROIndex);*/
			/* Time to go home! TGIF! */
			if (GetMV(_myMV, CISDone)) {
				Printf1("Airline check-in staff %d of airline %d is closing the counter\n",
					sizeof("Airline check-in staff %d of airline %d is closing the counter\n"),
					concat2Num(_myIndex, _myAirline));
/*				Acquire(GetMV(my, CISLock));*/
				Release(GetMV(myAirlineMACRO, AirlineLock));
/*				Wait(GetMV(my, CISLock), GetMV(my, CISCommCV)); *//* Wait forever, basically */
/*				Release(GetMV(my, CISLock)); *//* Never reaches here, but whatever... */
				break;
			}
			decrementMV(myAirlineMACRO, AirlineNumOnBreakCIS);
		}
		/* Start helping a passenger */
		SetMV(_myMV, CISState, BUSY);
		Acquire(GetMV(myAirlineMACRO, AirlineCISLineLock));
		Acquire(GetMV(myAirlineMACRO, AirlineExecLineLock));
		if (queue_size(GetMV(myAirlineMACRO, AirlineExecQueue)) > 0) {
			SetMV(_myMV, CISCurrentPassenger, queue_pop(GetMV(myAirlineMACRO, AirlineExecQueue)));
			/*passenger._cisID = _myIndex;*/
			SetMV(passengerMACRO, PassCISID, _myIndex);
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concat3Num(_myIndex, _myAirline, GetMV(_myMV, CISLineSize)));
			Signal(GetMV(myAirlineMACRO, AirlineExecLineLock), GetMV(myAirlineMACRO, AirlineExecLineCV)); /* Signal Passenger */ 
		} else if (GetMV(_myMV, CISLineSize) > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concat3Num(_myIndex, _myAirline, queue_size(GetMV(myAirlineMACRO, AirlineExecQueue))));
			Signal(GetMV(myAirlineMACRO, AirlineCISLineLock), GetMV(_myMV, CISLineCV));
/*Printf1("Cis %d of airline %d wakes up passenger\n", sizeof("Cis %d of airline %d wakes up passenger\n"), _myIndex*1000+_myAirline);*/
		}
		/* Interact with Passenger */
		Acquire(GetMV(_myMV, CISLock));
		Release(GetMV(myAirlineMACRO, AirlineCISLineLock));
		Release(GetMV(myAirlineMACRO, AirlineExecLineLock));
		Release(GetMV(myAirlineMACRO, AirlineLock));
		if (GetMV(_myMV, CISLineSize) > 0 || GetMV(_myMV, CISCurrentPassenger) != -1) {
/*Printf1("Cis %d of airline %d goes to sleep\n", sizeof("Cis %d of airline %d goes to sleep\n"), _myIndex*1000+_myAirline);*/
			Wait(GetMV(_myMV, CISLock), GetMV(_myMV, CISCommCV)); 
		} /* Otherwise, Manager woke you up for no reason */
		if (GetMV(_myMV, CISCurrentPassenger) != -1) {
			int i;
			/* Assign seat number */
			Acquire(GetMV(myAirlineMACRO, AirlineLock));
			SetMV(passengerMACRO, PassTicketSeat, GetMV(myAirlineMACRO, AirlineNumCheckedinPassengers));
			IncrementMV(myAirlineMACRO, AirlineNumCheckedinPassengers);
			Release(GetMV(myAirlineMACRO, AirlineLock));
			/* Deal with baggage */
			Acquire(ConveyorLock);
			for (i = 0; i < GetMV(passengerMACRO, PassNumBaggages); ++i) {
				/*Printf0("3\n", sizeof("1\n"));*/
/*				#define bIndex (passenger._id * 3) + i
				#define bag Baggages[bIndex]*/
				/*#define bag Baggages[(passenger._id*3)+i]*/
				SetMV(
					GetMV(
						baggages,
						GetMV(passengerMACRO, PassIndex) * 3 + i
						),
					BaggageAirline,
					_myAirline
					);
/*				queue_insert(&ConveyorBelt, bIndex);*/
				queue_insert(conveyorBelt, GetMV(passengerMACRO, PassIndex)*3+i);
				Printf1("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n",
					sizeof("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n"),
					concat2Num(_myIndex, _myAirline));
/*				myAirline._numExpectedBaggages++;*/
				SetMV(
					_myMV,
					CISWeightCount, 
					GetMV(_myMV, CISWeightCount) 
					+ GetMV(
						baggages,
						GetMV(passengerMACRO, PassIndex) * 3 + i
						)
					);
				/*#undef bag*/
/*				#undef bIndex*/
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (GetMV(passengerMACRO, PassTicketExecutive)) {
				Printf2("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, GetMV(my, CISCurrentPassenger), _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, GetMV(_myMV, CISCurrentPassenger)), _myAirline);
			} else {
				Printf2("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, GetMV(my, CISCurrentPassenger), _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, GetMV(_myMV, CISCurrentPassenger)), _myAirline);
			}
			Signal(GetMV(_myMV, CISLock), GetMV(_myMV, CISCommCV)); 
			Wait(GetMV(_myMV, CISLock), GetMV(_myMV, CISCommCV)); 
		}
		SetMV(_myMV, CISCurrentPassenger, -1);
		Release(GetMV(_myMV, CISLock));
	} /* end while */
	Exit(0);
#undef passengerMACRO
#undef myAirlineMACRO
}

int main() {
	doCreates();
    startCheckInStaff();
}
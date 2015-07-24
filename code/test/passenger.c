/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */
#include "create.h"

void startPassenger();

/*
	main - start the airport sim
*/
int main() {
	doCreates();
    startPassenger();
}

/*
	startPassenger
*/
void startPassenger() {
	/* Claim my Passenger */
	int i, j; /* for-loop iterators */
	int _myIndex; /* ID for currentThread */
	int _myMV;
	int _minLineSize;
	int _liaison;

    Acquire(GlobalDataLock);
    _myIndex = GetMV(NumActivePassengers, 0);
    incrementMV(NumActivePassengers, 0);
    _myMV = GetMV(passengers, _myIndex);
    Release(GlobalDataLock);

    /*
		Liaison Interaction
    */
	Acquire(LiaisonLineLock);
	_minLineSize = GetMV( /* Set minLineSize to first line lineSize */
		GetMV(liaisons, 0),
		LiaisonLineSize
		);
	/* Find shortest line */
	for (i = 1; i < NUM_LIASONS; i++) {
		/*if (Liaisons[i]._lineSize < _minLineSize) {
			_minLineSize = Liaisons[i]._lineSize;
			my._liaisonID = i;
		}*/

		if (
			GetMV( /* Set minLineSize to first line lineSize */
				GetMV(liaisons, i),
				LiaisonLineSize)
			< _minLineSize
			)
		{
			_liaison = GetMV(liaisons, i);
			_minLineSize = GetMV( /* Set minLineSize to first line lineSize */
					_liaison,
					LiaisonLineSize);
			SetMV(
				_myMV,
				PassLiaisonID,
				i
				);
		}

	}

	Printf1("Passenger %d chose Liaison %d with a line length %d\n", 
		sizeof("Passenger %d chose Liaison %d with a line length %d\n"), 
		concat3Num(_myIndex, _liaison, GetMV(_liaison, LiaisonLineSize)));

	/* Get in line? */
	/*if (liaison._state == BUSY) {
		liaison._lineSize++;
		Wait(LiaisonLineLock, liaison._lineCV);
		liaison._lineSize--;
	}*/

	if (GetMV(_liaison, LiaisonState) == BUSY) {
		incrementMV(_liaison, LiaisonLineSize);
		Wait(
			LiaisonLineLock, 
			GetMV(_liaison, LiaisonLineCV)
		);
		decrementMV(_liaison, LiaisonLineSize);
	}

	Printf1("Passenger %d is moving along...\n", sizeof("Passenger %d is moving along...\n"), _myIndex);
	/* Go to Liaison */
	/*Acquire(liaison._lock);*/
	Acquire(GetMV(_liaison, LiaisonLock));
	Release(LiaisonLineLock);
	/* Give Liaison my Passenger info */
	/*liaison._passCount[my._ticket._airline]++;*/
	incrementMV(
		GetMV(_liaison, LiaisonPassCount), 
		GetMV(_myMV, PassTicketAirline)
		);
	/*liaison._bagCount[my._ticket._airline] += my._numBaggages;*/
	SetMV(
		GetMV(_liaison, LiaisonBagCount),
		GetMV(_myMV, PassTicketAirline),
		GetMV(_liaison, LiaisonBagCount) + GetMV(_myMV, PassNumBaggages)
		);
	/*liaison._currentPassenger = _myIndex;*/
	SetMV(_liaison, LiaisonCurrentPassenger, _myMV);
	Signal(
		GetMV(_liaison, LiaisonLock), 
		GetMV(_liaison, LiaisonCommCV) ); /* Signal Liaison */
	/*Wait(liaison._lock, liaison._commCV);*/ /* Wait for Liaison */
	Wait(
		GetMV(_liaison, LiaisonLock), 
		GetMV(_liaison, LiaisonCommCV) );
	Printf1("Passenger %d of Airline %d is directed to the check-in counter\n", 
		sizeof("Passenger %d of Airline %d is directed to the check-in counter\n"),
		concat2Num(_myIndex, GetMV(_myMV, PassTicketAirline)));
	Signal(
		GetMV(_liaison, LiaisonLock), 
		GetMV(_liaison, LiaisonCommCV) ); /* Signal Liaison */
	Release(GetMV(_liaison, LiaisonLock));
	/* end Liaison Interaction */

	Exit(0);
}

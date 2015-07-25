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
	int _myAirline;
	int _myCIS;
	int _myOff;
	int _myInsp;
	int temp0, temp1, temp2, temp3;

    Acquire(GlobalDataLock);
    _myIndex = GetMV(NumActivePassengers, 0);
    incrementMV(NumActivePassengers, 0);
    _myMV = GetMV(passengers, _myIndex);
    Release(GlobalDataLock);

	/*
		Check-in Staff Interaction
	*/
	_myAirline = GetMV(airlines, GetMV(_myMV, PassTicketAirline));
	if (GetMV(_myMV, PassTicketExecutive)) {
		Acquire(GetMV(_myAirline, AirlineExecLineLock));
		queue_insert( GetMV(_myAirline, AirlineExecQueue), _myMV );
		Printf1("Passenger %d of Airline %d is waiting in the executive class line\n", 
			sizeof("Passenger %d of Airline %d is waiting in the executive class line\n"),
			concat2Num(_myIndex, GetMV(_myMV, PassTicketAirline)));
/*		Wait(myAirline._execLineLock, myAirline._execLineLock); *//* Wait on CIS */
		Wait(GetMV(_myAirline, AirlineExecLineLock), GetMV(_myAirline, AirlineExecLineCV)); /* Wait on CIS */
        _myCIS = GetMV(_myMV, PassCISID);
Printf1("Passenger %d woke up (executive)\n", sizeof("Passenger %d woke up (executive)\n"), _myIndex);
	} else { /* Economy */
		Acquire(GetMV(_myAirline, AirlineCISLineLock));
		/* Find shortest line */
		_minLineSize = GetMV(GetMV(GetMV(_myAirline, AirlineCIS), 0), CISLineSize); /* declare at top of startPassenger */
		_myCIS = GetMV(GetMV(_myAirline, AirlineCIS), 0);
Printf1("GetMV(_myAirline, AirlineCIS) = %d\n", sizeof("GetMV(_myAirline, AirlineCIS) = %d\n"), GetMV(_myAirline, AirlineCIS));
		for (i = 0; i < NUM_CIS_PER_AIRLINE; ++i) {
			int nextSize = GetMV(GetMV(GetMV(_myAirline, AirlineCIS), i), CISLineSize);
			if ( nextSize < _minLineSize ) {
				_minLineSize = nextSize;
				_myCIS = GetMV(GetMV(_myAirline, AirlineCIS), i);
			}
		}
		Printf2("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
			sizeof("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n"),
			concat3Num(_myIndex, GetMV(_myMV, PassTicketAirline), _myCIS), _minLineSize);
			incrementMV(_myCIS, CISLineSize);
		Wait( GetMV(_myAirline, AirlineCISLineLock), GetMV( _myCIS, CISLineCV ) );
Printf1("Passenger %d woke up\n", sizeof("Passenger %d woke up\n"), _myIndex);
	}
	Acquire( GetMV(_myCIS, CISLock) );

Printf1("_myMV = %d\n", sizeof("_myMV = %d\n"), _myMV);
Printf1("_myAirline = %d\n", sizeof("_myairline = %d\n"), _myAirline);
Printf1("_myCIS = %d\n", sizeof("_myCIS = %d\n"), _myCIS);

	if ( GetMV(_myMV, PassTicketExecutive) ) {
		Release( GetMV(_myAirline, AirlineExecLineLock) );
	} else {
		decrementMV( _myCIS, CISLineSize );
		Release( GetMV(_myAirline, AirlineCISLineLock) );
	}
	/* Give baggage to CIS */
	incrementMV( _myCIS, CISPassCount );
	SetMV( _myCIS, CISBagCount, GetMV(_myCIS, CISBagCount) + GetMV(_myMV, PassNumBaggages) );
	SetMV( _myCIS, CISCurrentPassenger, _myMV );
	Signal( GetMV(_myCIS, CISLock), GetMV(_myCIS, CISCommCV) ); /* Signal CIS */
	Wait( GetMV(_myCIS, CISLock), GetMV(_myCIS, CISCommCV) ); /* Wait CIS */
	Printf1("Passenger %d of Airline %d was informed to board at gate %d\n",
		sizeof("Passenger %d of Airline %d was informed to board at gate %d\n"),
		concat3Num(_myIndex, GetMV(_myMV, PassTicketAirline), GetMV(_myMV, PassTicketAirline)));
	Signal( GetMV(_myCIS, CISLock), GetMV(_myCIS, CISCommCV) );
	Release( GetMV(_myCIS, CISLock) );
	/* end Check-in Staff Interaction */

	Exit(0);
}

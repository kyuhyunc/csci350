#include "create.h"

void startManager() {
	int i, j, liaison, person, airlineMV; /* for-loop iterator */
Printf0("Manager Start\n", sizeof("Manager Start\n"));
	while (true) {
		int numReadyAirlines = 0;
		/*
			Check-in Staff 
		*/
		if (!GetMV(manager, ManAllCISDone)) {
			int numDoneAirline = 0;
			int cis;
			for (i = 0; i < NUM_AIRLINES; ++i) {
				airlineMV = GetMV(airlines, i);
				if ( !GetMV( airlineMV, AirlineCISClosed ) ) {
/*Printf0("Before Acquire( GetMV(airlineMV, AirlineLock))\n", sizeof("Before Acquire( GetMV(airlineMV, AirlineLock))\n"));*/
					Acquire( GetMV(airlineMV, AirlineLock) );
/*Printf0("After Acquire( GetMV(airlineMV, AirlineLock))\n", sizeof("After Acquire( GetMV(airlineMV, AirlineLock))\n"));*/
					if ( GetMV(airlineMV, AirlineNumCheckedinPassengers) == GetMV(airlineMV, AirlineNumExpectedPassenger) )	{
						if ( GetMV(airlineMV, AirlineNumOnBreakCIS) == NUM_CIS_PER_AIRLINE ) {
							/* All Passenger have JUST went through, send CIS home */
							numDoneAirline++;
							for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
								cis = GetMV( GetMV(airlineMV, AirlineCIS), j);
								SetMV(cis, CISDone, true);
								Signal( GetMV(airlineMV, AirlineLock), GetMV(cis, CISCommCV) );
							}
							SetMV( airlineMV, AirlineCISClosed, true );
						}
					} else {
						/* There are still passengers to serve */
						for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
/*							Acquire( GetMV(airlineMV, AirlineCISLineLock) );
							Acquire( GetMV(airlineMV, AirlineExecLineLock) );
							Acquire( GetMV(cis, CISLock) );*/
                            cis = GetMV( GetMV(airlineMV, AirlineCIS), j);
							if (
								( 
									!queue_empty( GetMV(airlineMV, AirlineExecQueue)) 
									|| GetMV(cis, CISLineSize) 
								) 
								&&  GetMV(cis, CISState) == ONBREAK
							){
                                Printf1("Airport Manager calls back airline %d cis %d\n", sizeof("Airport Manager calls back airline %d cis %d\n"), 1000*i+j);

								Signal( GetMV(airlineMV, AirlineLock), GetMV( cis, CISCommCV ) );
							}
/*							Release( GetMV(cis, CISLock) );
							Release( GetMV(airlineMV, AirlineExecLineLock) );
							Release( GetMV(airlineMV, AirlineCISLineLock) );*/
						}
					}
/*Printf0("Before Release( GetMV(airlineMV, AirlineLock))\n", sizeof("Before Release( GetMV(airlineMV, AirlineLock))\n"));*/
                    Release( GetMV(airlineMV, AirlineLock) );
/*Printf0("After Release( GetMV(airlineMV, AirlineLock))\n", sizeof("After Release( GetMV(airlineMV, AirlineLock))\n"));*/
				} else {
					numDoneAirline++;
				}	
			}
			if (numDoneAirline == NUM_AIRLINES) {
				SetMV( manager, ManAllCISDone, true );
				/* Exit all Liaisons too! */
				/* If all CIS are done, so are all Liaisons */
				SetMV( manager, ManAllLiaisonDone, true );
				for(i = 0; i < NUM_LIASONS; ++i) {
					liaison = GetMV( liaisons, i );
					Signal( GetMV(liaison, LiaisonLock), GetMV(liaison, LiaisonCommCV) );
				}
			} else {
				SetMV( manager, ManAllCISDone, false );
			}
		} /* end if(!_allCISDone) */
		/* end CIS */

		/*
			Screening Officers
		*/
		Acquire(OfficersLineLock);
		for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
			person = GetMV( screeningOfficers, i );
			if ( !queue_empty(officersLine) &&  GetMV(person, SOState) == ONBREAK ) {
				Signal( OfficersLineLock, GetMV( person, SOCommCV ) );
			}
		}
		Release(OfficersLineLock);
		/* end Screening Officers */

	}
	Exit(0);
}

int main() {
	doCreates();
    startManager();
}


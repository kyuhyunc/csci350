#include "create.h"

void startManager() {
	int i, j, liaison, person, airlineMV; /* for-loop iterator */
/*Printf0("Manager Start\n", sizeof("Manager Start\n"));*/
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
			Check Conveyor Belt - Cargo Handlers
		*/
		if (!GetMV(manager, ManAllCargoDone)) {
			int numDone = 0;
			bool msgToCargo = true;
			int ch;

			Acquire(ConveyorLock);
			for (i = 0; i < NUM_AIRLINES; ++i) {
				airlineMV = GetMV(airlines, i);
				if ( GetMV(airlineMV, AirlineNumExpectedBaggages) == GetMV(airlineMV, AirlineNumLoadedBaggages) ) {
					numDone++;
				}
			}
			if (numDone == NUM_AIRLINES) {
Printf0("All baggages have been processed!\n", sizeof("All baggages have been processed!\n"));
				SetMV( manager, ManAllCargoDone, true );
				for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
					Signal( ConveyorLock, GetMV( GetMV( cargoHandlers, i ), CHCommCV ) );
				}
			}
			for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
				ch = GetMV( cargoHandlers, i );
				if (
					!queue_empty(conveyorBelt) 
					&& GetMV( ch, CHState ) == ONBREAK
				){
					Signal( ConveyorLock, GetMV(ch, CHCommCV) );
					if (msgToCargo) {
						Printf0("Airport manager calls back all the cargo handlers from break\n",
							sizeof("Airport manager calls back all the cargo handlers from break\n"));
						msgToCargo = false;
					}
				}
			}
			Release(ConveyorLock);
		}
		/* end Conveyor Belt / Cargo Handlers */


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

		/*
			Check Boarding Lounge
		*/
		for (i = 0; i < NUM_AIRLINES; ++i) {
			airlineMV = GetMV( airlines, i );
			if (GetMV(airlineMV, AirlineBoarded)) {
				numReadyAirlines++;
			} else if(
				GetMV( airlineMV, AirlineNumExpectedBaggages ) == GetMV( airlineMV, AirlineNumLoadedBaggages )
				&& GetMV( airlineMV, AirlineNumExpectedPassenger ) == GetMV( airlineMV, AirlineNumReadyPassengers ) 
			){
				Printf1("Airport manager gives a boarding call to airline %d\n",
					sizeof("Airport manager gives a boarding call to airline %d\n"),
					i);
				for (j = 0; j <  GetMV( airlineMV, AirlineNumReadyPassengers ); ++j) {
					Acquire( GetMV(airlineMV, AirlineLock) );
					Signal( GetMV(airlineMV, AirlineLock), GetMV(airlineMV, AirlineBoardLoungeCV) );
					Release( GetMV(airlineMV, AirlineLock) );
				}
				numReadyAirlines++;
				SetMV( airlineMV, AirlineBoarded, true );
			}
		}

		if (numReadyAirlines == NUM_AIRLINES) {
			int pass_cnt_liaisons = 0;
			int pass_cnt_SI = 0;
			int pass_cnt_CIS = 0;
			int bag_cnt_liaison = 0;
			int bag_cnt_cargo = 0;
			int weight_cnt_CIS = 0;
			int weight_cnt_cargo = 0;

			Printf0("=====================STATS======================\n", sizeof("=====================STATS======================\n"));

			for (i = 0; i < NUM_LIASONS; ++i) {
				for (j = 0; j < NUM_AIRLINES; ++j) {
					pass_cnt_liaisons += GetMV(  GetMV( GetMV(liaisons, i), LiaisonPassCount ), j  );
				}
			}
			Printf1("Passenger count reported by airport liaison = %d\n",
				sizeof("Passenger count reported by airport liaison = %d\n"),
				pass_cnt_liaisons);

			for (i = 0; i < NUM_AIRLINES; ++i) {	
				for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
					pass_cnt_CIS +=  GetMV( GetMV( GetMV( GetMV(airlines, i), AirlineCIS ), j ), CISPassCount);
				}
			}
			Printf1("Passenger count reported by airline check-in staff = %d\n",
				sizeof("Passenger count reported by airline check-in staff = %d\n"),
				pass_cnt_CIS);

			for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {	
				pass_cnt_SI +=  GetMV(GetMV(securityInspectors, i), SIPassCount);
			}
			Printf1("Passenger count reported by security inspector = %d\n",
				sizeof("Passenger count reported by security inspector = %d\n"),
				pass_cnt_SI);

			for (i = 0; i < NUM_AIRLINES; ++i) {	
				Printf1("From setup: Baggage count of airline %d = %d\n",
					sizeof("From setup: Baggage count of airline %d = %d\n"),
					concat2Num(i, GetMV(GetMV(airlines, i), AirlineNumExpectedBaggages)));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {	
				bag_cnt_liaison = 0;
				for (j = 0; j < NUM_LIASONS; ++j) {	
					bag_cnt_liaison +=  GetMV(GetMV(GetMV(liaisons, j), LiaisonBagCount), i);
				}
				Printf1("From airport liaison: Baggage count of airline %d = %d\n",
					sizeof("From airport liaison: Baggage count of airline %d = %d\n"),
					concat2Num(i, bag_cnt_liaison));
			}	

			for (i = 0; i < NUM_AIRLINES; ++i) {
				bag_cnt_cargo = 0;
				for (j = 0; j < NUM_CARGO_HANDLERS; ++j) {	
					bag_cnt_cargo +=  GetMV(GetMV(GetMV(cargoHandlers, j), CHBagCount), i);
				}
				Printf1("From cargo handlers: Baggage count of airline %d = %d\n",
					sizeof("From cargo handlers: Baggage count of airline %d = %d\n"),
				 	concat2Num(i, bag_cnt_cargo));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {
				Printf1("From setup: Baggage weight of airline %d = %d\n",
					sizeof("From setup: Baggage weight of airline %d = %d\n"),
					concat2Num(i, GetMV( GetMV(airlines, i), AirlineWeightCount )));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {
				weight_cnt_CIS = 0;
				for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
					weight_cnt_CIS += GetMV(GetMV(GetMV(GetMV( airlines, i ), AirlineCIS ), j), CISWeightCount);
				}
				Printf1("From airline check-in staff: Baggage weight of airline %d = %d\n",
					sizeof("From airline check-in staff: Baggage weight of airline %d = %d\n"),
					concat2Num(i, weight_cnt_CIS));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {
				weight_cnt_cargo = 0;
				for (j = 0; j < NUM_CARGO_HANDLERS; ++j) {
					weight_cnt_cargo += GetMV(GetMV(GetMV(cargoHandlers, j), CHWeightCount), i);
				}
				Printf1("From cargo handlers: Baggage weight of airline %d = %d\n", 
				sizeof("From cargo handlers: Baggage weight of airline %d = %d\n"),
					concat2Num(i, weight_cnt_cargo));
			}

			Printf0("================================================\n", sizeof("================================================\n"));

			/* tell all Screening Officers to go home */
			SetMV(manager, ManAllSODone, true);
			for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
				Signal( OfficersLineLock, GetMV(GetMV(screeningOfficers, i), SOCommCV) );
			}

			/* tell all Security Inspectors to go home */
			SetMV(manager, ManAllSIDone, true);
			for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
				Signal( GetMV(GetMV(securityInspectors, i), SILock), GetMV(GetMV(securityInspectors, i), SICommCV) );
			}
			break;
		}


		/*
			Make sure Manager doesn't hog CPU
		*/
		for (i = 0; i < 20; ++i) {
			Yield();
		}

	}
	Exit(0);
}

int main() {
	doCreates();
    startManager();
}


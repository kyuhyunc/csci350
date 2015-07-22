#include "create.h"

#define NULL 0


typedef int bool;
enum bool {false, true};


/*
	Utilities	
*/

int concat3Num(int i, int j, int k) {
	return 1000000 * i + 1000 * j + k;
} 

int concat2Num(int i, int j) {
	return 1000 * i + j;
}


void startManager() {
#define cis Airlines[i]._cis[j]
	int i, j; /* for-loop iterator */
	while (true) {
		int numReadyAirlines = 0;
		/*
			Check-in Staff 
		*/
		if (!Manager._allCISDone) {
			int numDoneAirline = 0;
			for (i = 0; i < NUM_AIRLINES; ++i) {
				if (!Airlines[i]._CISclosed) {
					Acquire(Airlines[i]._lock);
					if (Airlines[i]._numCheckedinPassengers == Airlines[i]._numExpectedPassengers){
						if (Airlines[i]._numOnBreakCIS == NUM_CIS_PER_AIRLINE) {
							/* All Passenger have JUST went through, send CIS home */
							Release(Airlines[i]._lock); /* TODO is this necessary? */
							numDoneAirline++;
							for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
								/*Acquire(cis._lock);*/
								cis._done = true;
								Signal(Airlines[i]._lock, cis._commCV);
								/*Release(cis._lock);*/
							}
							Airlines[i]._CISclosed = true;
						} else {
							Release(Airlines[i]._lock); /* TODO is this necessary? */
						}
					} else {
						/* There are still passengers to serve */
						Release(Airlines[i]._lock);/* TODO is this necessary? */
						for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
							Acquire(Airlines[i]._cisLineLock);
							Acquire(Airlines[i]._execLineLock);
							Acquire(cis._lock);
							if ((!queue_empty(&Airlines[i]._execQueue) || cis._lineSize) && cis._state == ONBREAK) {
/*								Signal(cis._lock, cis._commCV);*/
								Signal(Airlines[i]._lock, cis._commCV);
							}
							Release(cis._lock);
							Release(Airlines[i]._execLineLock);
							Release(Airlines[i]._cisLineLock);
						}
					}
				} else {
					numDoneAirline++;
				}	
			}
			if (numDoneAirline == NUM_AIRLINES) {
				Manager._allCISDone = true;
				/* Exit all Liaisons too! */
				/* If all CIS are done, so are all Liaisons */
				Manager._allLiaisonsDone = true;
				for(i = 0; i < NUM_LIASONS; ++i) {
					Signal(Liaisons[i]._lock, Liaisons[i]._commCV);
				}
			} else {
				Manager._allCISDone = false;
			}
		} /* end if(!_allCISDone) */
#undef cis
		/* end CIS */

		/*
			Check Conveyor Belt - Cargo Handlers
		*/
		if (!Manager._allCargoDone) {
			int numDone = 0;
			bool msgToCargo = true;

			Acquire(ConveyorLock);
			for (i = 0; i < NUM_AIRLINES; ++i) {
/*Printf1("Number loaded: %d, number expected: %d\n", sizeof("Number loaded: %d, number expected: %d\n"), concat2Num(Airlines[i]._numExpectedBaggages, Airlines[i]._numLoadedBaggages));*/
				if (Airlines[i]._numExpectedBaggages == Airlines[i]._numLoadedBaggages) {
					numDone++;
				}
			}
			if (numDone == NUM_AIRLINES) {
/*Printf0("All Cargo Handlers done!\n", sizeof("All Cargo Handlers done!\n"));*/
				Manager._allCargoDone = true;
				for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
					Signal(ConveyorLock, CargoHandlers[i]._commCV);
				}
			}
			for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
				if (!queue_empty(&ConveyorBelt) && CargoHandlers[i]._state == ONBREAK) {
					Signal(ConveyorLock, CargoHandlers[i]._commCV);

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
			Security Officers
		*/
		Acquire(OfficersLineLock);
		for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
			if (!queue_empty(&OfficersLine) && ScreeningOfficers[i]._state == ONBREAK) {
				Signal(OfficersLineLock, ScreeningOfficers[i]._commCV);
			}
		}
		Release(OfficersLineLock);
		/* end Security Officers */

		/*
			Check Boarding Lounge
		*/
		for (i = 0; i < NUM_AIRLINES; ++i) {
			if (Airlines[i]._boarded) {
				numReadyAirlines++;
			} else if(Airlines[i]._numExpectedBaggages == Airlines[i]._numLoadedBaggages
				&& Airlines[i]._numExpectedPassengers == Airlines[i]._numReadyPassengers) {
				Printf1("Airport manager gives a boarding call to airline %d\n",
					sizeof("Airport manager gives a boarding call to airline %d\n"),
					i);
				for (j = 0; j < Airlines[i]._numReadyPassengers; ++j) {
					Acquire(Airlines[i]._lock);
					Signal(Airlines[i]._lock, Airlines[i]._boardLoungeCV);
					Release(Airlines[i]._lock);
				}
				numReadyAirlines++;
				Airlines[i]._boarded = true;
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
					pass_cnt_liaisons += Liaisons[i]._passCount[j];
				}
			}
			Printf1("Passenger count reported by airport liaison = %d\n",
				sizeof("Passenger count reported by airport liaison = %d\n"),
				pass_cnt_liaisons);

			for (i = 0; i < NUM_AIRLINES; ++i) {	
				for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
					pass_cnt_CIS += Airlines[i]._cis[j]._passCount;
				}
			}
			Printf1("Passenger count reported by airline check-in staff = %d\n",
				sizeof("Passenger count reported by airline check-in staff = %d\n"),
				pass_cnt_CIS);

			for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {	
				pass_cnt_SI += SecurityInspectors[i]._passCount;
			}
			Printf1("Passenger count reported by security inspector = %d\n",
				sizeof("Passenger count reported by security inspector = %d\n"),
				pass_cnt_SI);

			for (i = 0; i < NUM_AIRLINES; ++i) {	
				Printf1("From setup: Baggage count of airline %d = %d\n",
					sizeof("From setup: Baggage count of airline %d = %d\n"),
					concat2Num(i, Airlines[i]._numExpectedBaggages));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {	
				bag_cnt_liaison = 0;
				for (j = 0; j < NUM_LIASONS; ++j) {	
					bag_cnt_liaison += Liaisons[j]._bagCount[i];
				}
				Printf1("From airport liaison: Baggage count of airline %d = %d\n",
					sizeof("From airport liaison: Baggage count of airline %d = %d\n"),
					concat2Num(i, bag_cnt_liaison));
			}	

			for (i = 0; i < NUM_AIRLINES; ++i) {
				bag_cnt_cargo = 0;
				for (j = 0; j < NUM_CARGO_HANDLERS; ++j) {	
					bag_cnt_cargo += CargoHandlers[j]._bagCount[i];
				}
				Printf1("From cargo handlers: Baggage count of airline %d = %d\n",
					sizeof("From cargo handlers: Baggage count of airline %d = %d\n"),
				 	concat2Num(i, bag_cnt_cargo));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {
				Printf1("From setup: Baggage weight of airline %d = %d\n",
					sizeof("From setup: Baggage weight of airline %d = %d\n"),
					concat2Num(i, Airlines[i]._weightCount));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {
				weight_cnt_CIS = 0;
				for (j = 0; j < NUM_CIS_PER_AIRLINE; ++j) {
					weight_cnt_CIS += Airlines[i]._cis[j]._weightCount;
				}
				Printf1("From airline check-in staff: Baggage weight of airline %d = %d\n",
					sizeof("From airline check-in staff: Baggage weight of airline %d = %d\n"),
					concat2Num(i, weight_cnt_CIS));
			}

			for (i = 0; i < NUM_AIRLINES; ++i) {
				weight_cnt_cargo = 0;
				for (j = 0; j < NUM_CARGO_HANDLERS; ++j) {
					weight_cnt_cargo += CargoHandlers[j]._weightCount[i];
				}
				Printf1("From cargo handlers: Baggage weight of airline %d = %d\n", 
				sizeof("From cargo handlers: Baggage weight of airline %d = %d\n"),
					concat2Num(i, weight_cnt_cargo));
			}

			Printf0("================================================\n", sizeof("================================================\n"));

			/* tell all Screening Officers to go home */
			Manager._allSODone = true;
			for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
				Signal(OfficersLineLock, ScreeningOfficers[i]._commCV);
			}

			/* tell all Security Inspectors to go home */
			Manager._allSIDone = true;
			for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
				Signal(SecurityInspectors[i]._lock, SecurityInspectors[i]._commCV);
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
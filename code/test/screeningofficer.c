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


void startScreeningOfficer() {
	#define mySO ScreeningOfficers[_myIndex]
	int _myIndex;
	/* Claim my screening officer */
	Acquire(GlobalDataLock);
    _myIndex = NumActiveScreeningOfficers++;
    Release(GlobalDataLock);

    /* Get to work! */
	while (true) {
		int suspicionLevel = false;
		int shortestLineIndex = -1; /* impossible value */
		int i; /* iterator */
		Acquire(OfficersLineLock);
		if (queue_empty(&OfficersLine)) {
			mySO._state = ONBREAK;
/*Printf1("SO %d is going to sleep\n", sizeof("SO %d is going to sleep\n"), _myIndex);*/
			/* Done? */
			if (Manager._allSODone) {
				Release(OfficersLineLock);
				break;
			}
			Wait(OfficersLineLock, mySO._commCV); /* Wait for a passenger */
			/* Done? */
			if (Manager._allSODone) {
				Release(OfficersLineLock);
				break;
			}
/*Printf1("SO %d is waking up\n", sizeof("SO %d is waking up\n"), _myIndex);*/
		}
		if (!queue_empty(&OfficersLine)) {
			Acquire(mySO._lock);
			mySO._state = BUSY;
			mySO._currentPassenger = queue_pop(&OfficersLine);
			Passengers[mySO._currentPassenger]._officerID = _myIndex;
			Signal(OfficersLineLock, OfficersLineCV); /* Wake up passenger */
			Release(OfficersLineLock);
			Wait(mySO._lock, mySO._commCV); /* Wait for passenger to approach */
			/* Generate PASS/FAIL Results */
			suspicionLevel = (17 * mySO._currentPassenger) % 10; /* PSUEDO rand() */
			SecurityFailResults[mySO._currentPassenger] = suspicionLevel > 8;
			if (SecurityFailResults[mySO._currentPassenger]) { /* FAIL */
				Printf1("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
					sizeof("Screening officer %d is suspicious of the hand luggage of passenger %d\n"),
					concat2Num(_myIndex, mySO._currentPassenger));
			} else { /* PASS */
				Printf1("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
					sizeof("Screening officer %d is not suspicious of the hand luggage of passenger %d\n"),
					concat2Num(_myIndex, mySO._currentPassenger));
			}
			/* Find an Available Security Inspector */
			while (shortestLineIndex == -1) {
/*Printf1("SO %d is trying to find SI\n", sizeof("SO %d is trying to find SI\n"), _myIndex);*/
				#define inspector SecurityInspectors[i]
				Acquire(InspectorLineLock);	
/*Printf1("SO %d acquired InspectorLineLock\n", sizeof("SO %d acquired InspectorLineLock\n"), _myIndex);*/
				for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
					Acquire(inspector._lock);
/*Printf1("SO %d is checking SI %d\n", sizeof("SO %d is checking SI %d\n"), concat2Num(_myIndex, i));*/
					if (inspector._state == AVAIL && inspector._newPassenger == -1) {
						/* Found an inspector! */
						shortestLineIndex = inspector._id;
						inspector._newPassenger = mySO._currentPassenger;
						Release(inspector._lock);
						break;
					}
					Release(inspector._lock);
				}
				Release(InspectorLineLock);
				if (shortestLineIndex == -1) {
Printf1("SO %d is about to Yield\n", sizeof("SO %d is about to Yield\n"), _myIndex);
					Yield();
				}
				#undef inspector
/*				Wait(mySO._lock, mySO._commCV);*/
			}
			/* Found an inspetor for the passenger */
			Passengers[mySO._currentPassenger]._inspectorID = shortestLineIndex;
			Printf1("Screening officer %d directs passenger %d to security inspector %d\n",
				sizeof("Screening officer %d directs passenger %d to security inspector %d\n"),
				concat3Num(_myIndex, mySO._currentPassenger, shortestLineIndex));
			Signal(mySO._lock, mySO._commCV);
			Release(mySO._lock);
		} else {
			Release(OfficersLineLock);
		}
	}
	Exit(0);
}

int main() {
	doCreates();
    startScreeningOfficer();
}
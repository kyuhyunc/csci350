#include "create.h"


void startScreeningOfficer() {
	#define mySOMACRO GetMV(screeningOfficers, _myIndex)
	int _myIndex;
    int _myMV;
	/* Claim my screening officer */
	Acquire(GlobalDataLock);
    _myIndex = GetMV(NumActiveScreeningOfficers, 0);
    incrementMV(NumActiveScreeningOfficers, 0);
    _myMV = GetMV(screeningOfficers, _myIndex);
    Release(GlobalDataLock);

    /* Get to work! */
	while (true) {
		int suspicionLevel = false;
		int shortestLineIndex = -1; /* impossible value */
		int i; /* iterator */
		Acquire(OfficersLineLock);
		if (queue_empty(officersLine)) {
			SetMV(mySOMACRO, SOState, ONBREAK);
/*Printf1("SO %d is going to sleep\n", sizeof("SO %d is going to sleep\n"), _myIndex);*/
			/* Done? */
			if (GetMV(manager, ManAllSODone)) {
				Release(OfficersLineLock);
				break;
			}
			Wait(OfficersLineLock, GetMV(mySOMACRO, SOCommCV)); /* Wait for a passenger */
			/* Done? */
			if (GetMV(manager, ManAllSODone)) {
				Release(OfficersLineLock);
				break;
			}
/*Printf1("SO %d is waking up\n", sizeof("SO %d is waking up\n"), _myIndex);*/
		}
		if (!queue_empty(officersLine)) {
			Acquire(GetMV(mySOMACRO, SOLock));
			SetMV(mySOMACRO, SOState, BUSY);
			SetMV(mySOMACRO, SOCurrentPassenger, queue_pop(officersLine));
/*			SetMV(GetMV(passengers, GetMV(mySOMACRO, SOCurrentPassenger)), PassOfficerID, _myIndex);*/
            SetMV(GetMV(mySOMACRO, SOCurrentPassenger), PassOfficerID, _myMV);
			Signal(OfficersLineLock, OfficersLineCV); /* Wake up passenger */
			Release(OfficersLineLock);
			Wait(GetMV(mySOMACRO, SOLock), GetMV(mySOMACRO, SOCommCV)); /* Wait for passenger to approach */
			/* Generate PASS/FAIL Results */
/*			suspicionLevel = (17 * GetMV(mySOMACRO, SOCurrentPassenger)) % 10; *//* PSUEDO rand() */
            suspicionLevel = 0;
			SetMV(SecurityFailResults, GetMV(GetMV(mySOMACRO, SOCurrentPassenger), PassIndex), suspicionLevel > 8);

			if (GetMV(SecurityFailResults, GetMV(GetMV(mySOMACRO, SOCurrentPassenger), PassIndex))) { /* FAIL */
				Printf1("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
					sizeof("Screening officer %d is suspicious of the hand luggage of passenger %d\n"),
					concat2Num(_myIndex, GetMV(mySOMACRO, SOCurrentPassenger)));
			} else { /* PASS */
				Printf1("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
					sizeof("Screening officer %d is not suspicious of the hand luggage of passenger %d\n"),
					concat2Num(_myIndex, GetMV(mySOMACRO, SOCurrentPassenger)));
			}
			/* Find an Available Security Inspector */
			while (shortestLineIndex == -1) {
/*Printf1("SO %d is trying to find SI\n", sizeof("SO %d is trying to find SI\n"), _myIndex);*/
				#define inspector GetMV(securityInspectors, i)
				Acquire(InspectorLineLock);	
/*Printf1("SO %d acquired InspectorLineLock\n", sizeof("SO %d acquired InspectorLineLock\n"), _myIndex);*/
				for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
					Acquire(GetMV(inspector, SILock));
/*Printf1("SO %d is checking SI %d\n", sizeof("SO %d is checking SI %d\n"), concat2Num(_myIndex, i));*/
					if (GetMV(inspector, SIState) == AVAIL && GetMV(inspector, SINewPassenger) == -1) {
						/* Found an inspector! */
						shortestLineIndex = GetMV(inspector, SIID);
						SetMV(inspector, SINewPassenger, GetMV(mySOMACRO, SOCurrentPassenger));
						Release(GetMV(inspector, SILock));
						break;
					}
else {
Printf1("GetMV(inspector, SIState) = %d\n", sizeof("GetMV(inspector, SIState) = %d\n"), GetMV(inspector, SIState));
Printf1("GetMV(inspector, SINewPassenger) = %d\n", sizeof("GetMV(inspector, SINewPassenger) = %d\n"), GetMV(inspector, SINewPassenger));
}
					Release(GetMV(inspector, SILock));
				}
				Release(InspectorLineLock);
				if (shortestLineIndex == -1) {
					Printf1("SO %d is about to Yield\n", sizeof("SO %d is about to Yield\n"), _myIndex);
					Yield();
				}
				#undef inspector
/*				Wait(GetMV(mySOMACRO, SOLock), GetMV(mySOMACRO, SOCommCV));*/
			}
			/* Found an inspetor for the passenger */
			SetMV(GetMV(passengers, GetMV(GetMV(mySOMACRO, SOCurrentPassenger), PassIndex)), PassInspectorID, shortestLineIndex);
			Printf1("Screening officer %d directs passenger %d to security inspector %d\n",
				sizeof("Screening officer %d directs passenger %d to security inspector %d\n"),
				concat3Num(_myIndex, GetMV(mySOMACRO, SOCurrentPassenger), shortestLineIndex));
			Signal(GetMV(mySOMACRO, SOLock), GetMV(mySOMACRO, SOCommCV));
			Release(GetMV(mySOMACRO, SOLock));
		} else {
			Release(OfficersLineLock);
		}
	}
}

int main() {
	doCreates();
    startScreeningOfficer();
}

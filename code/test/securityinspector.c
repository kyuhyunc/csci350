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

void startSecurityInspector() {
	#define mySI SecurityInspectors[_myIndex]
	int _myIndex;
	/* Claim my security insepctor */
	Acquire(GlobalDataLock);
/*    _myIndex = NumActiveScreeningOfficers++;*/
	_myIndex = NumActiveSecurityInspectors++;
    Release(GlobalDataLock);

    /* Start work */
    while (true) {
    	bool suspicious, guilty;
    	Acquire(mySI._lock);
/*Printf1("SI %d has _newPassenger", sizeof("SI %d has _newPassneger"), _myIndex);
Printf1("%d\n", sizeof("%d\n"), mySI._newPassenger);*/
    	if (mySI._rtnPassSize == 0 && mySI._newPassenger == -1) {
    		mySI._state = AVAIL;
			/* Done? */
			if (Manager._allSIDone) {
				Release(mySI._lock);
				break;
			}
    		Wait(mySI._lock, mySI._commCV); /* Wait for Passenger to come */
			/* Done? */
			if (Manager._allSIDone) {
				Release(mySI._lock);
				break;
			}
    		mySI._state = BUSY;
    	}
    	if (mySI._rtnPassSize > 0) { /* priority to returning passengers */
    		while (mySI._rtnPassSize > 0) {
    			Signal(mySI._lock, mySI._rtnPassCV); /* Wake up passener */
    			Wait(mySI._lock, mySI._rtnPassCV); /* Wait on passenger */
    			Printf1("Security inspector %d permits returning passenger %d to board\n",
    				sizeof("Security inspector %d permits returning passenger %d to board\n"),
    				concat2Num(_myIndex, mySI._rtnPassenger));
    			mySI._passCount++;
/*    			mySI._rtnPassenger--;*/
				mySI._rtnPassSize--;
    			Signal(mySI._lock, mySI._rtnPassCV);
    		}
    		mySI._rtnPassenger = -1;
    	}
    	if (mySI._newPassenger != -1) { /* I have a new passenger to help */ 
    		suspicious = (mySI._newPassenger * 23) % 10 > 7; /* Psuedo Random */
    		guilty = suspicious || SecurityFailResults[mySI._newPassenger];
    		if (suspicious) {
    			Printf1("Security Inspector %d is suspicious of the hand luggage of passenger %d\n",
    				sizeof("Security Inspector %d is suspicious of the hand luggage of passenger %d\n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    		} else {
    			Printf1("Security Inspector %d is not suspicious of the hand luggage of passenger %d\n",
    				sizeof("Security Inspector %d is not suspicious of the hand luggage of passenger %d\n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    		}
    		if (guilty) {
				Passengers[mySI._newPassenger]._furtherQuestioning = true;
    			Printf1("Security inspector %d asks passenger %d to go for further examination\n", 
    				sizeof("Security inspector %d asks passenger %d to go for further examination\n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    		} else {
    			Printf1("Security inspector %d allows passenger %d to board \n",
    				sizeof("Security inspector %d allows passenger %d to board \n"),
    				concat2Num(_myIndex, mySI._newPassenger));
    			mySI._passCount++;
    		}
    		mySI._newPassenger = -1;
    		Signal(mySI._lock, mySI._newPassCV);
    	}
    	Release(mySI._lock);
    }
    #undef mySI
	Exit(0);
}

int main() {
	doCreates();
    startSecurityInspector();
}
#include "create.h"

void startLiaison() {
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
	int _myMV;
    Acquire(GlobalDataLock);
    _myIndex = NumActiveLiaisons++;
    Release(GlobalDataLock);

	while(true) {
		/*
			Passenger Interaction
    	*/
	    Acquire(LiaisonLineLock);

	    Acquire(l._lock);
	    if (l._lineSize == 0) {
	    	Release(LiaisonLineLock);
	    	l._state = AVAIL;
			if (Manager._allLiaisonsDone) { /* Done? */
				Release(l._lock);
				break;
			}
	    	Wait(l._lock, l._commCV); /* Wait for new Passenger */
			if (Manager._allLiaisonsDone) { /* Done? */
				Release(l._lock);
				break;
			}
	    } else {
	    	Signal(LiaisonLineLock, l._lineCV); /* Signal Passenger */
	    	Release(LiaisonLineLock); 
	    	Wait(l._lock, l._commCV); /* Wait on Passenger */
	    }
	    l._state = BUSY;
	    Printf1("Airport Liaison %d directed passenger %d of airline %d\n",
	    	sizeof("Airport Liaison %d directed passenger %d of airline %d\n"),
	    	concat3Num(_myIndex, l._currentPassenger, Passengers[l._currentPassenger]._ticket._airline));
	    Signal(l._lock, l._commCV); /* Signal Passenger */
	    Wait(l._lock, l._commCV); /* Wait for Passenger to say goodbye */
	    Release(l._lock);
	    /* end Passenger Interaction */
	}

	Exit(0);
#undef l
}

int main() {
	doCreates();
    startLiaison();
}
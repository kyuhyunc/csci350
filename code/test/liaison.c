#include "create.h"

void startLiaison() {
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
	int _myMV;

    Acquire(GlobalDataLock);
	/*_myIndex = NumActiveLiaisons++;*/
	_myIndex = GetMV(NumActiveLiaisons, 0);
	incrementMV(NumActiveLiaisons, 0);
    Release(GlobalDataLock);

    _myMV = GetMV(liaisons, _myIndex);

	while(true) {
		/*
			Passenger Interaction
    	*/
	    Acquire(LiaisonLineLock);
	    Acquire(GetMV(_myMV, LiaisonLock)); 
	    if (GetMV(_myMV, LiaisonLineSize) == 0) {
	    	Release(LiaisonLineLock);
	    	SetMV(
	    		_myMV,
	    		LiaisonState,
	    		AVAIL
	    		);
	    	if (GetMV(manager, ManAllLiaisonDone)) {
	    		Release(GetMV(_myMV, LiaisonLock));
	    		break;
	    	}
	    	Wait(
	    		GetMV(_myMV, LiaisonLock),
	    		GetMV(_myMV, LiaisonCommCV) );
	    	if (GetMV(manager, ManAllLiaisonDone)) {
	    		Release(GetMV(_myMV, LiaisonLock));
	    		break;
	    	}
	    } else {
	    	Signal(LiaisonLineLock, GetMV(_myMV, LiaisonLineCV)); /* Signal Passenger */
	    	Release(LiaisonLineLock); 
	    	Wait(
	    		GetMV(_myMV, LiaisonLineLock),
	    		GetMV(_myMV, LiaisonCommCV) ); /* Wait on Passenger */
	    }
	    SetMV(_myMV, LiaisonState, BUSY);
	    Printf1("Airport Liaison %d directed passenger %d of airline %d\n",
	    	sizeof("Airport Liaison %d directed passenger %d of airline %d\n"),
	    	concat3Num(
	    		_myIndex, 
	    		GetMV(_myMV, LiaisonCurrentPassenger), 
	    		GetMV(GetMV(_myMV, LiaisonCurrentPassenger), PassTicketAirline) );
	    Signal(
    		GetMV(_myMV, LiaisonLineLock),
    		GetMV(_myMV, LiaisonCommCV) ); /* Wait on Passenger */
	    Wait(
    		GetMV(_myMV, LiaisonLineLock),
    		GetMV(_myMV, LiaisonCommCV) ); /* Wait on Passenger */
	    Release( GetMV(_myMV, LiaisonLineLock) );
	    /* end Passenger Interaction */
	}

	Exit(0);
#undef l
}

int main() {
	doCreates();
    startLiaison();
}
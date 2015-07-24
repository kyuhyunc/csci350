#include "create.h"

void startLiaison() {
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
	int _myMV;

	Printf0(
	    		"startLiaison\n",
	    		sizeof("startLiaison\n")
	    		);

    Acquire(GlobalDataLock);
	/*_myIndex = NumActiveLiaisons++;*/
	_myIndex = GetMV(NumActiveLiaisons, 0);
	incrementMV(NumActiveLiaisons, 0);
    Release(GlobalDataLock);

    _myMV = GetMV(liaisons, _myIndex);

    Printf1(
    		"Liaison's ID: %d\n",
    		sizeof("Liaison's ID: %d\n"),
    		_myMV
    		);

	while(true) {
		/*
			Passenger Interaction
    	*/
	    Acquire(LiaisonLineLock);
	    Acquire(GetMV(_myMV, LiaisonLock)); 
	    if (GetMV(_myMV, LiaisonLineSize) == 0) {
	    	Printf1(
				"Liaison state before: %d\n",
				sizeof("Liaison state before: %d\n"),
				GetMV(_myMV, LiaisonState)
				);
	    	SetMV(
	    		_myMV,
	    		LiaisonState,
	    		AVAIL
	    		);
	    	Printf1(
				"Liaison state: %d\n",
				sizeof("Liaison state: %d\n"),
				GetMV(_myMV, LiaisonState)
				);
	    	Release(LiaisonLineLock);
	    	if (GetMV(manager, ManAllLiaisonDone)) {
	    		Release(GetMV(_myMV, LiaisonLock));
	    		break;
	    	}
	    	Printf0(
	    		"About to go to sleep\n",
	    		sizeof("About to go to sleep\n")
	    		);
	    	Wait(
	    		GetMV(_myMV, LiaisonLock),
	    		GetMV(_myMV, LiaisonCommCV) );
	    	if (GetMV(manager, ManAllLiaisonDone)) {
	    		Release(GetMV(_myMV, LiaisonLock));
	    		Printf0(
		    		"ManAllLiaisonDone\n",
		    		sizeof("ManAllLiaisonDone\n")
		    		);
	    		break;
	    	}
	    } else {
	    	Printf1(
				"Liaison signal. CV: %d, Lock:%d\n",
				sizeof("Liaison signal. CV: %d, Lock:%d\n"),
				concat2Num(GetMV(_myMV, LiaisonLineCV), LiaisonLineLock));
	    	Signal(LiaisonLineLock, GetMV(_myMV, LiaisonLineCV)); /* Signal Passenger */
	    	Release(LiaisonLineLock); 
	    	Printf1(
				"Liaison wait. CV: %d, Lock:%d\n",
				sizeof("Liaison wait. CV: %d, Lock:%d\n"),
				concat2Num(GetMV(_myMV, LiaisonCommCV), GetMV(_myMV, LiaisonLock)));
	    	Wait(
	    		GetMV(_myMV, LiaisonLock),
	    		GetMV(_myMV, LiaisonCommCV) ); /* Wait on Passenger */
	    }
	    Printf0(
	    		"About to help\n",
	    		sizeof("About to help\n")
	    		);
	    /*SetMV(_myMV, LiaisonState, BUSY);
	    Printf1(
	    	"Airport Liaison %d directed passenger %d of airline %d\n",
	    	sizeof("Airport Liaison %d directed passenger %d of airline %d\n"),
	    	concat3Num(
	    		_myIndex,
	    		GetMV(
	    			_myMV, 
	    			LiaisonCurrentPassenger
	    			), 
	    		GetMV(
	    			GetMV(
	    				_myMV, 
	    				LiaisonCurrentPassenger
	    				), 
	    			PassTicketAirline
	    			) 
	    		)
	    	);*/
	    /*Signal(
    		GetMV(_myMV, LiaisonLineLock),
    		GetMV(_myMV, LiaisonCommCV) );*/ /* Wait on Passenger */
	    /*Wait(
    		GetMV(_myMV, LiaisonLineLock),
    		GetMV(_myMV, LiaisonCommCV) );*/ /* Wait on Passenger */
	    /*Release( GetMV(_myMV, LiaisonLineLock) );*/
	    /* end Passenger Interaction */
	}
	Exit(0);
}

int main() {
	doCreates();
    startLiaison();
}
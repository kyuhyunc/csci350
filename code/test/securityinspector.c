#include "create.h"

void startSecurityInspector() {
	int _myIndex, _myMV;
	/* Claim my security insepctor */
	Acquire(GlobalDataLock);
/*    _myIndex = NumActiveScreeningOfficers++;*/
	_myIndex = NumActiveSecurityInspectors++;
    _myMV = GetMV( securityInspectors, _myIndex );
    Release(GlobalDataLock);

    /* Start work */
    while (true) {
    	bool suspicious, guilty;
    	Acquire( GetMV(_myMV, SILock) );
        if ( GetMV(_myMV, SIRtnPassSize) == 0 && GetMV(_myMV, SINewPassenger) == -1 ) {
            SetMV(_myMV, SIState, AVAIL);
			/* Done? */
            if (GetMV(manager, ManAllSIDone)) {
                Release(_myMV, SILock);
                break;
            }
            Wait( GetMV(_myMV, SILock), GetMV(_myMV, SICommCV) ); /* Wait for Passenger to come */
			/* Done? */
			if (GetMV(manager, ManAllSIDone)) {
                Release(_myMV, SILock);
                break;
            }
            SetMV(_myMV, SIState, BUSY);
    	}
        if ( GetMV(_myMV, SIRtnPassSize) > 0 ) { /* priority to returning passengers */
    		while ( GetMV( _myMV, SIRtnPassSize ) > 0 ) {
                Signal( GetMV(_myMV, SILock), GetMV(_myMV, SIRtnPassCV) ); /* Wake up passener */
                Wait( GetMV(_myMV, SILock), GetMV(_myMV, SIRtnPassCV) ); /* Wait on passenger */
    			Printf1("Security inspector %d permits returning passenger %d to board\n",
    				sizeof("Security inspector %d permits returning passenger %d to board\n"),
    				concat2Num(_myIndex, GetMV(_myMV, SIRtnPassenger)));
                incrementMV(_myMV, SIPassCount);
/*    			mySI._rtnPassenger--;*/
				decrementMV(_myMV, SIPassCount);
                Signal( GetMV(_myMV, SILock), GetMV(_myMV, SIRtnPassCV) );
    		}
            SetMV( _myMV, SIRtnPassenger, -1 );
    	}
        if ( GetMV(_myMV, SINewPassenger) != -1 ) { /* I have a new passenger to help */ 
            suspicious = (GetMV(_myMV, SINewPassenger) * 23) % 10 > 7; /* Psuedo Random */
            guilty = suspicious || GetMV( SecurityFailResults, GetMV(GetMV(_myMV, SINewPassenger), PassIndex) );            
    		if (suspicious) {
    			Printf1("Security Inspector %d is suspicious of the hand luggage of passenger %d\n",
    				sizeof("Security Inspector %d is suspicious of the hand luggage of passenger %d\n"),
    				concat2Num(_myIndex, GetMV(_myMV, SINewPassenger)));
    		} else {
    			Printf1("Security Inspector %d is not suspicious of the hand luggage of passenger %d\n",
    				sizeof("Security Inspector %d is not suspicious of the hand luggage of passenger %d\n"),
    				concat2Num(_myIndex, GetMV(_myMV, SINewPassenger)));
    		}
    		if (guilty) {
				Passengers[mySI._newPassenger]._furtherQuestioning = true;
    			Printf1("Security inspector %d asks passenger %d to go for further examination\n", 
    				sizeof("Security inspector %d asks passenger %d to go for further examination\n"),
    				concat2Num(_myIndex, GetMV(_myMV, SINewPassenger)));
    		} else {
    			Printf1("Security inspector %d allows passenger %d to board \n",
    				sizeof("Security inspector %d allows passenger %d to board \n"),
    				concat2Num(_myIndex, GetMV(_myMV, SINewPassenger)));
                incrementMV(_myMV, SIPassCount);
    		}
            SetMV(_myMV, SINewPassenger, -1)
            Signal(GetMV(_myMV, SILock), GetMV(_myMV, SINewPassCV));
    	}
        Release( GetMV(_myMV, SILock) );
    }
	Exit(0);
}

int main() {
	doCreates();
    startSecurityInspector();
}
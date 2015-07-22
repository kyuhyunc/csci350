/*	

	airportsim.c	
	Airport Simulation - User Program
 
 */
#include "create.h"

#define NULL 0


typedef int bool;
enum bool {false, true};

/* States used by various employees */
/*enum State {
    AVAIL,
    BUSY,
    ONBREAK
};*/



/*
	Utilities	
*/

int concat3Num(int i, int j, int k) {
	return 1000000 * i + 1000 * j + k;
} 

int concat2Num(int i, int j) {
	return 1000 * i + j;
}

/* TODO: implement new queue functions and copy to each file */

/*#define front queue->_front
#define rear queue->_rear*/
void queue_insert (Queue* queue, int index) {
    if (queue->_rear == NUM_PASSENGERS*3-1)
        Printf0("ERROR: QUEUE OVERFLOW\n", sizeof("ERROR: QUEUE OVERFLOW\n"));
    else {
        /* If queue is initially empty */
        if (queue->_front == -1)
            queue->_front = 0;
/*        queue->_array[++rear] = index;*/
        queue->_array[++queue->_rear] = index;
    }
}

int queue_pop (Queue* queue) {
    if (queue->_front == -1 || queue->_front > queue->_rear) {
        Printf0("ERROR: QUEUE IS EMPTY\n", sizeof("ERROR: QUEUE IS EMPTY\n"));
        return -1;
    }
/*    return queue->_array[front++];*/
    return queue->_array[queue->_front++];
}

int queue_size (Queue* queue) {
    if (queue->_front == -1 || queue->_front > queue->_rear) return 0;
/*    return rear - front + 1;*/
    return queue->_rear - queue->_front + 1;
}

bool queue_empty (Queue* queue) {
    if (queue_size (queue) == 0) return true;
    else return false;
}
/*#undef front
#undef reqr*/

/*
	Start Functions - functions called by Fork() syscall.
*/
void startPassenger() {    
#define my Passengers[_myIndex]
#define liaison Liaisons[my._liaisonID]
	/* Claim my Passenger */
	int i, j; /* for-loop iterators */
	int _myIndex; /* ID for currentThread */
	int _minLineSize;

    Acquire(GlobalDataLock);
    _myIndex = NumActivePassengers++;
    Release(GlobalDataLock);

    /*
		Liaison Interaction
    */
	Acquire(LiaisonLineLock);
	_minLineSize = Liaisons[0]._lineSize;
	/* Find shortest line */
	for (i = 1; i < NUM_LIASONS; i++) {
		if (Liaisons[i]._lineSize < _minLineSize) {
			_minLineSize = Liaisons[i]._lineSize;
			my._liaisonID = i;
		}
	}

	Printf1("Passenger %d chose Liaison %d with a line length %d\n", 
		sizeof("Passenger %d chose Liaison %d with a line length %d\n"), 
		concat3Num(_myIndex, my._liaisonID, liaison._lineSize));
	/* Get in line? */
	if (liaison._state == BUSY) {
/*		Printf0("Passenger says liaison is BUSY\n", 
			sizeof("Passenger says liaison is BUSY\n"));
		Printf1("Passenger's liaison id: %d\n", 
			sizeof("Passenger's liaison id: %d\n"),
			_myIndex);*/
		liaison._lineSize++;
		Wait(LiaisonLineLock, liaison._lineCV);
		liaison._lineSize--;
	}
	Printf1("Passenger %d is moving along...\n", sizeof("Passenger %d is moving along...\n"), _myIndex);
	/* Go to Liaison */
	Acquire(liaison._lock);
	Release(LiaisonLineLock);
	/* Give Liaison my Passenger info */
	liaison._passCount[my._ticket._airline]++;
	liaison._bagCount[my._ticket._airline] += my._numBaggages;
	liaison._currentPassenger = _myIndex;
	Signal(liaison._lock, liaison._commCV); /* Signal Liaison */
	Wait(liaison._lock, liaison._commCV); /* Wait for Liaison */
	Printf1("Passenger %d of Airline %d is directed to the check-in counter\n", 
		sizeof("Passenger %d of Airline %d is directed to the check-in counter\n"),
		concat2Num(_myIndex, my._ticket._airline));
	Signal(liaison._lock, liaison._commCV);
	Release(liaison._lock);
#undef liaison
	/* end Liaison Interaction */

	/*
		Check-in Staff Interaction
	*/
#define myAirline Airlines[my._ticket._airline]
#define myCIS myAirline._cis[my._cisID]
	if (my._ticket._executive) {
		Acquire(myAirline._execLineLock);
		queue_insert(&myAirline._execQueue, _myIndex);
		Printf1("Passenger %d of Airline %d is waiting in the executive class line\n", 
			sizeof("Passenger %d of Airline %d is waiting in the executive class line\n"),
			concat2Num(_myIndex, my._ticket._airline));
/*		Wait(myAirline._execLineLock, myAirline._execLineLock); *//* Wait on CIS */
		Wait(myAirline._execLineLock, myAirline._execLineCV); /* Wait on CIS */
	} else { /* Economy */
		Acquire(myAirline._cisLineLock);
		/* Find shortest line */
		_minLineSize = myAirline._cis[0]._lineSize; /* declare at top of startPassenger */
		my._cisID = 0;
		for (i = 0; i < NUM_CIS_PER_AIRLINE; ++i) {
			if (myAirline._cis[i]._lineSize < _minLineSize) {
				_minLineSize = myAirline._cis[i]._lineSize;
				my._cisID = i;
			}
		}
		Printf2("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
			sizeof("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n"),
			concat3Num(_myIndex, my._ticket._airline, my._cisID), _minLineSize);
			myCIS._lineSize++;
/*Printf1("Passenger %d of Airline %d is going to sleep and should be woken up by cis %d\n", sizeof("Passenger %d of Airline %d is going to sleep and should be woken up by cis %d\n"), concat3Num(_myIndex, my._ticket._airline, my._cisID));*/
		Wait(myAirline._cisLineLock, myCIS._lineCV);
/*Printf1("Passenger %d of Airline %d is woken up by cis %d\n", sizeof("Passenger %d of Airline %d is woken up by cis %d\n"), concat3Num(_myIndex, my._ticket._airline, my._cisID));*/
	}
	Acquire(myCIS._lock);
	if (my._ticket._executive) {
		Release(myAirline._execLineLock);
	} else {
		myCIS._lineSize--;
		Release(myAirline._cisLineLock);
	}
	/* Give baggage to CIS */
	myCIS._passCount++;
	myCIS._bagCount += my._numBaggages;
	myCIS._currentPassenger = _myIndex;
	Signal(myCIS._lock, myCIS._commCV); /* Signal CIS */
	Wait(myCIS._lock, myCIS._commCV); /* Wait on CIS */
	Printf1("Passenger %d of Airline %d was informed to board at gate %d\n",
		sizeof("Passenger %d of Airline %d was informed to board at gate %d\n"),
		concat3Num(_myIndex, my._ticket._airline, my._ticket._airline));
	Signal(myCIS._lock, myCIS._commCV); /* Signal CIS */
	Release(myCIS._lock);

#undef myCIS
#undef myAirline
	/* end Check-in Staff Interaction */

	/*
		Screening Officer Interaction
	*/
	Acquire(OfficersLineLock);
	queue_insert(&OfficersLine, _myIndex);
	Wait(OfficersLineLock, OfficersLineCV);
	Printf1("Passenger %d gives the hand-luggage to screening officer %d\n",
		sizeof("Passenger %d gives the hand-luggage to screening officer %d\n"),
		concat2Num(_myIndex, my._officerID));
	Acquire(ScreeningOfficers[my._officerID]._lock);
	Release(OfficersLineLock);
	Signal(ScreeningOfficers[my._officerID]._lock, ScreeningOfficers[my._officerID]._commCV);
	Wait(ScreeningOfficers[my._officerID]._lock, ScreeningOfficers[my._officerID]._commCV);
	/* officer lock is released below! */
	/* end Screening Officer Interaction */

	/*
		Security Inspector Interaction
	*/
	#define inspector SecurityInspectors[my._inspectorID]
	Printf1("Passenger %d moves to security inspector %d\n",
		sizeof("Passenger %d moves to security inspector %d\n"),
		concat2Num(_myIndex, my._inspectorID));
	Acquire(inspector._lock);
	Release(ScreeningOfficers[my._officerID]._lock);
	inspector._newPassenger = _myIndex;
	if (inspector._state == AVAIL) { /* Wake up inspector */
		Signal(inspector._lock, inspector._commCV);
	}
	Wait(inspector._lock, inspector._newPassCV); /* Wait for security results */
	if (my._furtherQuestioning) {
		Printf1("Passenger %d goes for futher questioning\n", 
			sizeof("Passenger %d goes for futher questioning\n"),
			_myIndex);
		Release(inspector._lock);
		for (i = 0; i < 10; ++i) {
			Yield(); /* Simulate Further questioning */
		}
		Printf1("Passenger %d comes back to security inspector %d after further examination\n",
			sizeof("Passenger %d comes back to security inspector %d after further examination\n"),
			concat2Num(_myIndex, my._inspectorID));
		Acquire(inspector._lock);
/*		inspector._rtnPassenger++;*/
		inspector._rtnPassSize++;
		if (inspector._state == AVAIL) {
			Signal(inspector._lock, inspector._commCV);
		}
/*Printf0("About to wait\n", sizeof("About to wait\n"));*/
		Wait(inspector._lock, inspector._rtnPassCV);
/*Printf0("Woken up\n", sizeof("Woken up\n"));*/
		inspector._rtnPassenger = _myIndex;
		Signal(inspector._lock, inspector._rtnPassCV);
		Wait(inspector._lock, inspector._rtnPassCV);
	}	
	Release(inspector._lock);

	#undef inspector
	/* end Security Inspector Interaction */

	/*
		Reached the Boarding Lounge
	*/
	#define myAirline Airlines[my._ticket._airline]

	Acquire(myAirline._lock);
	myAirline._numReadyPassengers++;
	Printf1("Passenger %d of Airline %d reached the gate %d\n",
		sizeof("Passenger %d of Airline %d reached the gate %d\n"),
		concat3Num(_myIndex, my._ticket._airline, my._ticket._airline));
	Wait(myAirline._lock, myAirline._boardLoungeCV); /* Wait for boarding call by manager */
	Printf1("Passenger %d of Airline %d boarded airline %d\n",
		sizeof("Passenger %d of Airline %d boarded airline %d\n"),
		concat3Num(_myIndex, my._ticket._airline, my._ticket._airline));
	Release(myAirline._lock);

	#undef myAirline
	/* End Boarding Lounge */

	Exit(0);
#undef my
}


/*
	main - start the airport sim
*/
int main() {
	doCreates();
    startPassenger();
}

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


void startCheckInStaff() {
#define myAirline Airlines[_myAirline]
#define my myAirline._cis[_myIndex]
#define passenger Passengers[my._currentPassenger]
	/* Claim my CIS */
	int _myAirline;
	int _myIndex; /* ID for currentThread */
    Acquire(GlobalDataLock);
	_myIndex = NumActiveCIS % NUM_CIS_PER_AIRLINE;
	_myAirline = NumActiveCIS / NUM_CIS_PER_AIRLINE;
    NumActiveCIS++;
    Release(GlobalDataLock);

    while (true) {
		/* Check lines */
		Acquire(myAirline._lock);
		if (my._lineSize == 0 && queue_empty(&myAirline._execQueue)) {
			/*Printf0("1\n", sizeof("1\n"));*/
			my._state = ONBREAK;
			/* 'Clock Out' for Break */
			myAirline._numOnBreakCIS++;
/*Printf1("Cis %d going to sleep\n", sizeof("Cis %d going to sleep\n"), _myIndex);*/
			Wait(myAirline._lock, my._commCV); /* Wait on Manager */ /* TODO - make sure okay to wait on aiport lock... maybe better? */
/*Printf1("Cis %d woke up by manager\n", sizeof("Cis %d woke up by manager\n"), _myIndex);*/
			/* Time to go home! TGIF! */
			if (my._done) {
				Printf1("Airline check-in staff %d of airline %d is closing the counter\n",
					sizeof("Airline check-in staff %d of airline %d is closing the counter\n"),
					concat2Num(_myIndex, _myAirline));
/*				Acquire(my._lock);*/
				Release(myAirline._lock);
/*				Wait(my._lock, my._commCV); *//* Wait forever, basically */
/*				Release(my._lock); *//* Never reaches here, but whatever... */
				break;
			}
			myAirline._numOnBreakCIS--;
		}
		/* Start helping a passenger */
		my._state = BUSY;
		Acquire(myAirline._cisLineLock);
		Acquire(myAirline._execLineLock);
		if (queue_size(&myAirline._execQueue) > 0) {
			my._currentPassenger = queue_pop(&myAirline._execQueue);
			passenger._cisID = _myIndex;
			Printf1("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an executive class passenger and economy line length = %d\n"),
				concat3Num(_myIndex, _myAirline, my._lineSize));
			Signal(myAirline._execLineLock, myAirline._execLineCV); /* Signal Passenger */
		} else if (my._lineSize > 0) {
			Printf1("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				sizeof("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n"),
				concat3Num(_myIndex, _myAirline, queue_size(&myAirline._execQueue)));
			Signal(myAirline._cisLineLock, my._lineCV);
/*Printf1("Cis %d of airline %d wakes up passenger\n", sizeof("Cis %d of airline %d wakes up passenger\n"), _myIndex*1000+_myAirline);*/
		}
		/* Interact with Passenger */
		Acquire(my._lock);
		Release(myAirline._cisLineLock);
		Release(myAirline._execLineLock);
		Release(myAirline._lock);
		if (my._lineSize > 0 || my._currentPassenger != -1) {
/*Printf1("Cis %d of airline %d goes to sleep\n", sizeof("Cis %d of airline %d goes to sleep\n"), _myIndex*1000+_myAirline);*/
			Wait(my._lock, my._commCV); 
		} /* Otherwise, Manager woke you up for no reason */
		if (my._currentPassenger != -1) {
			int i;
			/* Assign seat number */
			Acquire(myAirline._lock);
			passenger._ticket._seat = myAirline._numCheckedinPassengers;
			myAirline._numCheckedinPassengers++;
			Release(myAirline._lock);
			/* Deal with baggage */
			Acquire(ConveyorLock);
			for (i = 0; i < passenger._numBaggages; ++i) {
				/*Printf0("3\n", sizeof("1\n"));*/
/*				#define bIndex (passenger._id * 3) + i
				#define bag Baggages[bIndex]*/
				#define bag Baggages[(passenger._id*3)+i]
				bag._airline = _myAirline; /* Tag the bag */
/*				queue_insert(&ConveyorBelt, bIndex);*/
				queue_insert(&ConveyorBelt, passenger._id*3+i);
				Printf1("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n",
					sizeof("Airline check-in staff %d of airline %d dropped bags to the conveyor system \n"),
					concat2Num(_myIndex, _myAirline));
/*				myAirline._numExpectedBaggages++;*/
				my._weightCount += bag._weight;
				#undef bag
/*				#undef bIndex*/
			}
			Release(ConveyorLock);
			/* Direct Passenger to Airline */
			if (passenger._ticket._executive) {
				Printf2("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, my._currentPassenger, _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, my._currentPassenger), _myAirline);
			} else {
				Printf2("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
					sizeof("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n"),
/*					concat3Num(_myAirline, my._currentPassenger, _myAirline), _myIndex);*/
					concat3Num(_myIndex, _myAirline, my._currentPassenger), _myAirline);
			}
			Signal(my._lock, my._commCV); 
			Wait(my._lock, my._commCV); 
		}
		my._currentPassenger = -1;
		Release(my._lock);
	} /* end while */
	Exit(0);
#undef passenger
#undef my
#undef myAirline
}

int main() {
	doCreates();
    startCheckInStaff();
}
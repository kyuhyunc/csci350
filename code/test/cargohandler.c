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


void startCargoHandler() {
#define my CargoHandlers[_myIndex]
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
	int bIndex;
    Acquire(GlobalDataLock);
    _myIndex = NumActiveCargoHandlers++;
    Release(GlobalDataLock);
	
	while (true) {
		#define bag Baggages[bIndex]
		Acquire(ConveyorLock);
		if (queue_empty(&ConveyorBelt)) {
			my._state = ONBREAK;
			Printf1("Cargo Handler %d is going for a break\n", sizeof("Cargo Handler %d is going for a break\n"), _myIndex);
			Wait(ConveyorLock, my._commCV);
			/* Done? */
			if (Manager._allCargoDone) {
				Release(ConveyorLock);
				break;
			}
			Printf1("Cargo Handler %d returned from break\n",
				sizeof("Cargo Handler %d returned from break\n"),
				_myIndex);
		} else {
			bIndex = queue_pop(&ConveyorBelt);
			Printf1("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n",
				sizeof("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n"),
				concat3Num(_myIndex, bag._airline, bag._weight));
			Airlines[bag._airline]._numLoadedBaggages++;
			my._bagCount[bag._airline]++;
			my._weightCount[bag._airline] += bag._weight;
		}
		Release(ConveyorLock);	
		Yield();
		#undef bag	
	}
	Exit(0);
#undef my
}

int main() {
    startCargoHandler();
}

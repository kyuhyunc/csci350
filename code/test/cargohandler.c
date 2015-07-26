#include "create.h"



void startCargoHandler() {
#define myMACRO GetMV(cargoHandlers, _myIndex)
	/* Claim my Liaison */
	int _myIndex; /* ID for currentThread */
	int bIndex;
    Acquire(GlobalDataLock);
    _myIndex = GetMV(NumActiveCargoHandlers, 0);
    incrementMV(NumActiveCargoHandlers, 0);
    Release(GlobalDataLock);
	
	while (true) {
		#define bagMACRO GetMV(baggages, bIndex)
		Acquire(ConveyorLock);
		if (queue_empty(conveyorBelt)) {
			if (GetMV(manager, ManAllCargoDone)) {
				Release(ConveyorLock);
				break;
			}
			SetMV(myMACRO, CHState, ONBREAK);
			Printf1("Cargo Handler %d is going for a break\n", sizeof("Cargo Handler %d is going for a break\n"), _myIndex);
			Wait(ConveyorLock, GetMV(myMACRO, CHCommCV));
			/* Done? */
			if (GetMV(manager, ManAllCargoDone)) {
				Release(ConveyorLock);
				break;
			}
			Printf1("Cargo Handler %d returned from break\n",
				sizeof("Cargo Handler %d returned from break\n"),
				_myIndex);
		} else {
			bIndex = queue_pop(conveyorBelt);
			Printf1("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n",
				sizeof("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n"),
				concat3Num(_myIndex, GetMV(bagMACRO, BaggageAirline), GetMV(bagMACRO, BaggageWeight)));

			incrementMV(GetMV(airlines, GetMV(bagMACRO, BaggageAirline)), AirlineNumLoadedBaggages);

			incrementMV(GetMV(myMACRO, CHBagCount), GetMV(bagMACRO, BaggageAirline));

			SetMV(
                GetMV(myMACRO, CHWeightCount),
                GetMV(bagMACRO, BaggageAirline),
                GetMV(GetMV(myMACRO, CHWeightCount), GetMV(bagMACRO, BaggageAirline)) + GetMV(bagMACRO, BaggageWeight));
		}
		Release(ConveyorLock);	
		Yield();
		#undef bagMACRO	
	}
    Printf1("Cargo Handler %d is going home\n", sizeof("Cargo Handler %d is going home\n"), _myIndex);
	Exit(0);
#undef myMACRO
}

int main() {
	doCreates();
    startCargoHandler();
}

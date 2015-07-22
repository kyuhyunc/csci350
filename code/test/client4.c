#include "create.h"

int main(){
	int i;
	for (i = 0; i < NUM_SCREENING_OFFICERS; ++i) {
		Exec("../test/screeningofficer", sizeof("../test/screeningofficer"));
	}
	for (i = 0; i < NUM_SECURITY_INSPECTORS; ++i) {
		Exec("../test/securityinspector", sizeof("../test/securityinspector"));
	}	}
	
}
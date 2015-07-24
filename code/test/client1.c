#include "create.h"

extern void doCreates();
extern void doInitialize();

int main(){
	int i;
	char* test;
	char buf[30];

	/* Client 1 must be run first to ensure values are initialized */
	doCreates();
	doInitialize();

	for (i = 0; i < NUM_PASSENGERS; ++i) {
		Exec("../test/passenger", sizeof("../test/passenger"));
	}
}
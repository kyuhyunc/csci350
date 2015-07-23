#include "create.h"

extern void doCreates();

int main(){
	int i;
	char* test;
	char buf[30];

	for (i = 0; i < 30; ++i) {
		buf[i] = '3';
	}

	for (i = 0; i < NUM_PASSENGERS; ++i) {
		/*Exec("../test/passenger", sizeof("../test/passenger"));*/
	}


	/*
	*	Testing 
	*		Okay to Delete, once it's time to start simulating the airport
	*/
	/*doCreates();*/
	Printf0("About to create lock", sizeof("About to create lock"));
	test = CreateLock(
			ConcatNumToString(
				"Lock",
				sizeof("Lock"),
				30),
			sizeof("Lock") + 2
			);
	Printf1("%d\n", sizeof("%d\n"), test);
}
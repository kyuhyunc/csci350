#include "create.h"

extern void doCreates();

int main(){
	int i;
	char* test;
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		/*Exec("../test/passenger", sizeof("../test/passenger"));*/
	}


	/*
	*	Testing 
	*		Okay to Delete, once it's time to start simulating the airport
	*/
	/*doCreates();*/
	test = 	(char*) (ConcatNumToString(
							"Test",
							sizeof("Test"),
							23
						)
					);
	Printf0(
			test,
			6
		);
}
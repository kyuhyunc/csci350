#include "create.h"

extern void doCreates();

int main(){
	int i;
	char* test;
	char buf[30];
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		/*Exec("../test/passenger", sizeof("../test/passenger"));*/
	}


	/*
	*	Testing 
	*		Okay to Delete, once it's time to start simulating the airport
	*/
	/*doCreates();*/
	test = 	(char*) (ConcatNumToString(
							"Uhh",
							sizeof("Uhh"),
							30
						)
					);
	Printf0(
			buf,
			6
		);
}
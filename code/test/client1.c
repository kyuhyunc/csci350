#include "create.h"

extern void doCreates();

int main(){
	int i;
	char* test;
	char buf[30];

	for (i = 0; i < NUM_PASSENGERS; ++i) {
		Exec("../test/passenger", sizeof("../test/passenger"));
	}
}
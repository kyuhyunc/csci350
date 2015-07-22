#include "create.h"

int main(){
	int i;
	for (i = 0; i < NUM_PASSENGERS; ++i) {
		Exec("../test/passenger", sizeof("../test/passenger"));
	}
}
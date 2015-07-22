#include "create.h"

int main(){
	int i;
	for (i = 0; i < NUM_CARGO_HANDLERS; ++i) {
		Exec("../test/cargohandler", sizeof("../test/cargohandler"));
	}
	
}
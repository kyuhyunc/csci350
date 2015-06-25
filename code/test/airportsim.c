/*	airportsim.c
	Airport Simulation
 */

#include "syscall.h"

struct random {
	int a;
	int b;
	int c;
};

int main() {
	struct random Try1;
	Try1.a = 1;
	Try1.b = 2;
	Try1.c = 3;

	Printf0("Write all airport code in this file!\n", sizeof("Write all airport code in this file!\n"));

	Printf1("Try1.a = %d\n", sizeof("Try1.a = %d\n"), Try1.a);


	Exit(0);

}

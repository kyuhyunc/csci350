/* testexithelper.c
 */

#include "syscall.h"

void g1() {
	Printf0("g1\n", sizeof("g1\n"));
	Exit(0);
}

void g2() {
	Printf0("g2\n", sizeof("g2\n"));
	Exit(0);
}

void g3() {
	Printf0("g3\n", sizeof("g3\n"));
	Exit(0);
}

void g4() {
	Printf0("g4\n", sizeof("g4\n"));
	Exit(0);
}

void g5() {
	Printf0("g5\n", sizeof("g5\n"));
	Exit(0);
}

int main() {
	Printf0("m2\n", sizeof("m2\n"));

	Fork(g1, "changeme", sizeof("changeme"));
	Fork(g2, "changeme", sizeof("changeme"));
	Fork(g3, "changeme", sizeof("changeme"));
	Fork(g4, "changeme", sizeof("changeme"));
	Fork(g5, "changeme", sizeof("changeme"));

}

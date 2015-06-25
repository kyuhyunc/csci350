/* testexit.c
 */

#include "syscall.h"

void f1() {
	Printf0("f1\n", sizeof("f1\n"));
	Exit(0);
}

void f2() {
	Printf0("f2\n", sizeof("f2\n"));
	Exit(0);
}

void f3() {
	Printf0("f3\n", sizeof("f3\n"));
	Exit(0);
}

void f4() {
	Printf0("f4\n", sizeof("f4\n"));
	Exit(0);
}

void f5() {
	Printf0("f5\n", sizeof("f5\n"));
	Exit(0);
}

int main() {
	Exec("../test/testexithelper", sizeof("../test/testexithelper"));

	Printf0("m1\n", sizeof("m1\n"));

	Fork(f1);
	Fork(f2);
	Fork(f3);
	Fork(f4);
	Fork(f5);

}

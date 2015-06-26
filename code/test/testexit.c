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

	CreateLock("abc", sizeof("abc"));
	CreateCV("def", sizeof("def"));

	Fork(f1, "changeme", sizeof("changeme"));
	Fork(f2, "changeme", sizeof("changeme"));
	Fork(f3, "changeme", sizeof("changeme"));
	Fork(f4, "changeme", sizeof("changeme"));
	Fork(f5, "changeme", sizeof("changeme"));

}

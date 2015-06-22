/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

/*void a() {
	Printf0("printf0", sizeof("printf0"));
	Write("a\n", 2, ConsoleOutput);
	Exit(0);
}

void b() {
	Printf1("printf%d", sizeof("printf%d"), 1);
	Write("b\n", 2, ConsoleOutput);
	Exit(0);
}

void c() {
	Printf2("printf%d, works %d!", sizeof("printf%d, works %d!"), 2, 2);
	Write("c\n", 2, ConsoleOutput);
	Exit(0);
}

void d() {
	Printf0("printf0d\n", sizeof("printf0d\n"));
	Exit(0);
}

void e() {
	Write("e\n", 2, ConsoleOutput);
	Exit(0);
}*/

void newpf() {
	Printf1("1:%d,2:%d,3:%d\n", sizeof("1:%d,2:%d,3:%d\n"), 1*1000000+2*1000+3);
	Exit(0);
}

void test_print() {
	Write("Testing print functions...\n", sizeof("Testing print functions...\n"), ConsoleOutput);
	Write("Testing Printf0...\n", sizeof("Testing Printf0...\n"), ConsoleOutput);
	Printf0("Printf0 is saying hi!\n", sizeof("Printf0 is saying hi!\n"));
/*	Write("If this line printed, Printf0 worked.\n", sizeof("If this line printed, Printf0 worked.\n"));*/
	Write("Testing Printf1 with 1 number: 23...\n", sizeof("Testing Printf1 with 1 number: 23...\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the number %d\n", sizeof("Printf1 is printing out the number %d\n"), 23);
	Write("Testing Printf1 with 2 numbers: 72 and 35...\n", sizeof("Testing Printf1 with 2 numbers: 72 and 35...\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the numbers %d and %d\n", sizeof("Printf1 is printing out the numbers %d and %d\n"), 72*1000+35);
	Write("Testing Printf1 with 3 numbers: 419, 742, and 129\n", sizeof("Testing Printf1 with 3 numbers: 419, 742, and 129\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the nubmers %d, %d, and %d\n", sizeof("Printf1 is printing out the nubmers %d, %d, and %d\n"), 419*1000000+742*1000+129);
	Write("Testing Printf2 with 4 numbers: 914, 388, 526, and 40\n", sizeof("Testing Printf2 with 4 numbers: 914, 388, 526, and 40\n"), ConsoleOutput);
	Printf2("Printf2 is printing out the numbers %d, %d, %d, and %d\n", sizeof("Printf2 is printing out the numbers %d, %d, %d, and %d\n"),
				914*1000000+388*1000+526, 40);
	Write("Testing Printf2 with 5 numbers: 63, 7, 823, 473, and 756\n", sizeof("Testing Printf2 with 5 numbers: 63, 7, 823, 473, and 756\n"), ConsoleOutput);
	Printf2("Printf2 is printing out the numbers %d, %d, %d, %d, and %d\n", sizeof("Printf2 is printing out the numbers %d, %d, %d, %d, and %d\n"),
				63*1000000+7*1000+823, 473*1000+756);
	Write("Testing Printf2 with 6 numbers: 321, 465, 738, 924, 172, and 54\n", sizeof("Testing Printf2 with 6 numbers: 321, 465, 738, 924, 172, and 54\n"), ConsoleOutput);
	Printf2("Printf2 is printing out the numbers %d, %d, %d, %d, %d, and %d\n", sizeof("Printf2 is printing out the numbers %d, %d, %d, %d, %d, and %d\n"),
				321*1000000+465*1000+738, 924*1000000+172*1000+54);
	
	Exit(0);
}

int main() {

/*	Fork(a);
	Fork(b);
	Fork(c);
	Fork(d);
	Fork(e);*/

/*	Printf0("About to call Exec...\n", sizeof("About to call Exec...\n"));*/
/*	Fork(test_print);*/
	Fork(newpf);
	Exec("../test/halt", sizeof("../test/halt"));
}

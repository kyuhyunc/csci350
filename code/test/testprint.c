/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

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
	Write("Testing Printf1 with 2 numbers: 3 and 35...\n", sizeof("Testing Printf1 with 2 numbers: 3 and 35...\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the numbers %d and %d\n", sizeof("Printf1 is printing out the numbers %d and %d\n"), 3*1000+35);
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
	Write("Testing Printf1 with 0's...\n", sizeof("Testing Printf1 with 0's...\n"), ConsoleOutput);
	Write("Testing Printf1 with 1 number: 0...\n", sizeof("Testing Printf1 with 1 number: 0...\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the number %d\n", sizeof("Printf1 is printing out the number %d\n"), 0);
	Write("Testing Printf1 with 2 numbers: 0 and 0...\n", sizeof("Testing Printf1 with 2 numbers: 0 and 0...\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the numbers %d and %d\n", sizeof("Printf1 is printing out the numbers %d and %d\n"), 0*1000+0);
	Write("Testing Printf1 with 3 numbers: 0, 0, and 0\n", sizeof("Testing Printf1 with 3 numbers: 0, 0, and 0\n"), ConsoleOutput);
	Printf1("Printf1 is printing out the nubmers %d, %d, and %d\n", sizeof("Printf1 is printing out the nubmers %d, %d, and %d\n"), 0*1000000+0*1000+0);
	Write("Testing Printf2 with 0's...\n", sizeof("Testing Printf2 with 0's...\n"), ConsoleOutput);
	Write("Testing Printf2 with 4 numbers: 0, 0, 0, and 0\n", sizeof("Testing Printf2 with 4 numbers: 0, 0, 0, and 0\n"), ConsoleOutput);
	Printf2("Printf2 is printing out the numbers %d, %d, %d, and %d\n", sizeof("Printf2 is printing out the numbers %d, %d, %d, and %d\n"),
				0*1000000+0*1000+0, 0);
	Write("Testing Printf2 with 5 numbers: 0, 0, 0, 0, and 0\n", sizeof("Testing Printf2 with 5 numbers: 0, 0, 0, 0, and 0\n"), ConsoleOutput);
	Printf2("Printf2 is printing out the numbers %d, %d, %d, %d, and %d\n", sizeof("Printf2 is printing out the numbers %d, %d, %d, %d, and %d\n"),
				0*1000000+0*1000+0, 0*1000+0);
	Write("Testing Printf2 with 6 numbers: 0, 0, 0, 0, 0, and 0\n", sizeof("Testing Printf2 with 6 numbers: 0, 0, 0, 0, 0, and 0\n"), ConsoleOutput);
	Printf2("Printf2 is printing out the numbers %d, %d, %d, %d, %d, and %d\n", sizeof("Printf2 is printing out the numbers %d, %d, %d, %d, %d, and %d\n"),
				0*1000000+0*1000+0, 0*1000000+0*1000+0);
	
	Exit(0);
}

int main() {

/*	Fork(a);
	Fork(b);
	Fork(c);
	Fork(d);
	Fork(e);*/

/*	Printf0("About to call Exec...\n", sizeof("About to call Exec...\n"));*/
	Fork(test_print, "changeme", sizeof("changeme"));
	Fork(newpf, "changeme", sizeof("changeme"));
/*	Exec("../test/halt", sizeof("../test/halt"));*/

}

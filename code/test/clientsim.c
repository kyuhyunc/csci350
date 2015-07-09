/*
*
*	A Nachos user program that interacts with the Server.
*	Project 3
*
*/
#include "syscall.h"

/*
*	"main"
*/
int test1_1, test1_2, test1_3;

void test1();

int main() {
	Fork(test1);

}

void test1() {

	Write("Test1: CreateLock index test\n", sizeof("Test1: CreateLock index test\n"), ConsoleOutput);

	test1_1 = CreateLock("lock1", sizeof("lock1"));
	test1_2 = CreateLock("lock2", sizeof("lock2"));
	test1_3 = CreateLock("lock1", sizeof("lock1"));

	if(test1_1 == test1_3) {
		Write("Test1 passed!\n", sizeof("Test1 passed!\n"), ConsoleOutput);
	}else{
		Write("Test1 failed!\n", sizeof("Test1 failed!\n"), ConsoleOutput);
	}
}
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
	Fork(test1, "test1", sizeof("test1"));

}

void test1() {

	Printf0("Test1: CreateLock index test\n", sizeof("Test1: CreateLock index test\n"));

	test1_1 = CreateLock("lock1", sizeof("lock1"));
	test1_2 = CreateLock("lock2", sizeof("lock2"));
	test1_3 = CreateLock("lock1", sizeof("lock1"));

	Printf0("About to DestroyLock", sizeof("About to DestroyLock"));
	DestroyLock(test1_3);

	if(test1_1 == test1_3) {
		Printf0("Test1 passed!\n", sizeof("Test1 passed!\n"));
	}else{
		Printf0("Test1 failed!\n", sizeof("Test1 failed!\n"));
	}

	Exit(0);
}

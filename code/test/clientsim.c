/*
*
*	A Nachos user program that interacts with the Server.
*	Project 3
*
*/
#include "syscall.h"
#define true 1
#define false 0
typedef char bool;

/*
*	Test 1 - CreateLock test
*/
bool test0_createLock() {
	int lock0, lock1, lock2;

	Printf0("Running test0_createLock\n", sizeof("Running test0_createLock\n"));

	lock0 = CreateLock("lock0", sizeof("lock0"));
	lock1 = CreateLock("lock1", sizeof("lock1"));
	lock2 = CreateLock("lock0", sizeof("lock0")); /* should return same index as lock0 */

	if(lock0 == 0 && lock1 == 1 && lock2 == 0) {
		Printf0("test0_createLock passed!\n", sizeof("test0_createLock passed!\n"));
		return true;
	}else{
		Printf0("test0_createLock failed!\n", sizeof("test0_createLock failed!\n"));
		return false;
	}
}

/*
*	Test 1 - DeleteLock test
*/
bool test1_deleteLock() {
	int lock0, lock1, destroyResult;
	
	Printf0("Running test1_deleteLock\n", sizeof("Running test1_deleteLock\n"));

	lock0 = CreateLock("lock0", sizeof("lock0"));
	destroyResult = DestroyLock(lock0);

	lock1 = CreateLock("lock0", sizeof("lock0")); /* should not return same address */

	if(destroyResult == lock0 && lock0 != lock1) {
		Printf0("test1_deleteLock passed!\n", sizeof("test1_deleteLock passed!\n"));
		return true;
	} else {
		Printf0("test1_deleteLock failed!\n", sizeof("test1_deleteLock failed!\n"));
		return false;
	}
}

/*
*	"main"
*/
int main() {
	test0_createLock();
	test1_deleteLock();
}

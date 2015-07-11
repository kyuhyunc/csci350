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

/* Test 2 - Acquire Lock */
int lock_t2;
int numLockAcquires;

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

void test2_thread() {
	int id = numLockAcquires, loopCnt;
	loopCnt = 0;
	numLockAcquires++;
	Printf1("TEST 2 - Thread %d is about to call acquire\n", sizeof("TEST 2 - Thread %d is about to call acquire\n"), id);
	Acquire(lock_t2);
	Printf1("TEST 2 - Thread %d successfully acquired a lock\n", sizeof("TEST 2 - Thread %d successfully acquired a lock\n"), id);
	while (numLockAcquires < 2 || loopCnt < 2) { /* wait for both threads to acquire lock */
		Printf1("TEST 2 - Thread %d is yielding\n", sizeof("TEST 2 - Thread %d is yielding\n"), id);
		Yield();
		loopCnt++;
	}
	Printf1("TEST 2 - Thread %d is definitely releasing the lock\n", sizeof("TEST 2 - Thread %d is releasing the lock\n"), id);
	Release(lock_t2);
	Exit(0);
}

bool test2_acquireLock() {
	Printf0("Running test2_acquireLock\n", sizeof("Running test2_acquireLock\n"));

	lock_t2 = CreateLock("lock_t2", sizeof("lock_t2"));

	Printf1("******** lock_t2: %d\n", sizeof("******** lock_t2: %d\n"), lock_t2);

	numLockAcquires = 0;

	Fork(test2_thread, "test2_thread_0", sizeof("test2_thread_0"));
	Fork(test2_thread, "test2_thread_1", sizeof("test2_thread_1"));
	return true; /* Must look at print statements to determine */
}

/*
*	"main"
*/
int main() {

	/*test0_createLock();*/
	/*test1_deleteLock();*/
	test2_acquireLock();

/*	if (
		test0_createLock() &&
		test1_deleteLock() &&
		test2_acquireLock()
	) {
		Printf0("All tests passed!\n", sizeof("All tests passed!\n"));
	} else {
		Printf0("Not all tests passed...\n", sizeof("Not all tests passed...\n"));
	}*/
}

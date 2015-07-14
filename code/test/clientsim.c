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
#define BUSY 0
#define AVAIL 1

/* Test 2 - Acquire Lock */
int lock_t2;
int numLockAcquires;

/* Test 6 - Wait */
int lock_t6, cv_t6, waitState_t6;

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
*	Test 2 Helper
* 	Thread acquires the lock, gives cpu up, then releases lock. 
*	The yielding is there so that other threads can have a chance to acquire the lock, 
*	and hence get put on the waitQueue
*/
void test2_thread() {
	int id = numLockAcquires, loopCnt;
	loopCnt = 0;
	numLockAcquires++;
	Printf1("TEST 2 - Thread %d is about to call acquire\n", 
		sizeof("TEST 2 - Thread %d is about to call acquire\n"), 
		id);
	Acquire(lock_t2);
	Printf1("TEST 2 - Thread %d successfully acquired a lock\n", 
		sizeof("TEST 2 - Thread %d successfully acquired a lock\n"), 
		id);
	while (numLockAcquires < 2 || loopCnt < 2) { /* wait for both threads to acquire lock */
		Printf1("TEST 2 - Thread %d is yielding\n", 
			sizeof("TEST 2 - Thread %d is yielding\n"), 
			id);
		Yield();
		loopCnt++;
	}
	Printf1("TEST 2 - Thread %d is definitely releasing the lock\n", 
		sizeof("TEST 2 - Thread %d is releasing the lock\n"), 
		id);
	Release(lock_t2);
	Exit(0);
}

/*
*	Test 2 - Tests Acquire functionality
*/
bool test2_acquireLock() {
	Printf0("Running test2_acquireLock\n", 
		sizeof("Running test2_acquireLock\n"));

	lock_t2 = CreateLock("lock_t2", sizeof("lock_t2"));

	numLockAcquires = 0;

	Fork(test2_thread, "test2_thread_0", 
		sizeof("test2_thread_0"));
	Fork(test2_thread, "test2_thread_1", 
		sizeof("test2_thread_1"));
	return true; /* Must look at print statements to determine */
}

/*
*	Test 3 - Checks that on a release, the release will destroy a lock is need be. 
*/
bool test3_releaseAndDestroy() {
	int lock0_t3, lock1_t3;

	Printf0("Running test3_releaseAndDestroy\n", sizeof("Running test3_releaseAndDestroy\n"));

	/* isToBeDeleted case */
	Printf0("Running test3_releaseAndDestroy case 1\n", sizeof("Running test3_releaseAndDestroy case 1\n"));
	lock0_t3 = CreateLock("lock_t3", sizeof("lock_t3"));
	Acquire(lock0_t3);
	DestroyLock(lock0_t3);
	Release(lock0_t3);

	/* Normal case */
	Printf0("Running test3_releaseAndDestroy case 2\n", sizeof("Running test3_releaseAndDestroy case 2\n"));
	lock1_t3 = CreateLock("lock_t3", sizeof("lock_t3"));
	Acquire(lock1_t3);
	Release(lock1_t3);
	DestroyLock(lock1_t3);

	if(lock0_t3 != lock1_t3) {
		Printf0("test3_releaseAndDestroy passed!\n", sizeof("test3_releaseAndDestroy passed!\n"));
		return true;
	}else{
		Printf0("test3_releaseAndDestroy failed!\n", sizeof("test3_releaseAndDestroy failed!\n"));
		return false;
	}
}

/*
*	Test 4 - Test CreateCV functionality
*		The first and second CV have the same name, 
*		and hence the same CV is returned
*		This test is very similar to Test 0 -- CreateLock test
*/
bool test4_createCV() {
	int cv0, cv1, cv2;

	Printf0("Running test4_createCV\n", sizeof("Running test4_createCV\n"));

	cv0 = CreateCV("cv0", sizeof("cv0"));
	cv1 = CreateCV("cv1", sizeof("cv1"));
	cv2 = CreateCV("cv0", sizeof("cv0")); /* should return same index as lock0 */

	if(cv0 == 0 && cv1 == 1 && cv2 == 0) {
		Printf0("test4_createCV passed!\n", sizeof("test4_createCV passed!\n"));
		return true;
	}else{
		Printf0("test4_createCV failed!\n", sizeof("test4_createCV failed!\n"));
		return false;
	}
}

/*
*	Test 5 - Test Destroy CV. Very similar to destroy lock test
*/
bool test5_destroyCV() {
	int cv0, cv1, destroyResult;
	
	Printf0("Running test5_destroyCV\n", sizeof("Running test5_destroyCV\n"));

	cv0 = CreateCV("cv0", sizeof("cv0"));
	destroyResult = DestroyCV(cv0);

	cv1 = CreateCV("cv0", sizeof("cv0")); /* should not return same address */

	if(destroyResult == cv0 && cv0 != cv1) {
		Printf0("test5_destroyCV passed!\n", sizeof("test5_destroyCV passed!\n"));
		return true;
	} else {
		Printf0("test5_destroyCV failed!\n", sizeof("test5_destroyCV failed!\n"));
		return false;
	}
}

/*
* 	Test 6 - See if Wait works
*/
void test6_thread() {
	Acquire(lock_t6);
	if ( GetMV(waitState_t6, 0) == AVAIL ) {
		SetMV(waitState_t6, 0, BUSY);
		Printf1("TEST 6 - Thread %d is about to Wait\n", 
			sizeof("TEST 6 - Thread %d is about to Wait\n"), 
			0);
		Wait(lock_t6, cv_t6);
	} else {
		Printf1("TEST 6 - Thread %d is about to Signal\n", 
			sizeof("TEST 6 - Thread %d is about to Signal\n"), 
			1);
		Signal(lock_t6, cv_t6);
	}
	Printf1("TEST 6 - Thread %d is about to Release\n", 
		sizeof("TEST 6 - Thread %d is about to Release\n"), 
		0);	
	Release(lock_t6);

	Exit(0);
}

bool test6_waitAndSignal() {
	Printf0("Running test6_waitAndSignal\n", sizeof("Running test6_waitAndSignal\n"));
	
	/* Init Values */
	lock_t6 = CreateLock("lock_t6", sizeof("lock_t6"));
	cv_t6 = CreateCV("cv_t6", sizeof("cv_t6"));
	waitState_t6 = CreateMV("waitState_t6", sizeof("waitState_t6"), 1);

	SetMV(waitState_t6, 0, AVAIL);

	/* Start test */
	Fork(test6_thread, "test6_thread_0", sizeof("test6_thread_0"));
	/*Fork(test6_thread, "test6_thread_1", sizeof("test6_thread_1"));*/

	/* TODO add long while loop of yields that checks values, return true here if conditions are met. */

	return true; /* Must look at output to determine... */
}

/*
*	Test 7 - Create MV
*/
bool test7_createMV() {
	int mv0, mv1, mv2, mv3;
	int mv0_size = 3, mv1_size = 1;

	Printf0("Running test7_createMV\n", sizeof("Running test7_createMV\n"));

	mv0 = CreateMV("mv0", sizeof("mv0"), mv0_size);
	mv1 = CreateMV("mv1", sizeof("mv1"), mv1_size);
	mv2 = CreateMV("mv0", sizeof("mv0"), mv0_size);
	mv3 = CreateMV("mv2", sizeof("mv2"), 0);

	if(mv0 == 0 && mv1 == 1 && mv2 == 0 && mv3 == -1) {
		Printf0("test7_createMV passed!\n", sizeof("test7_createMV passed!\n"));
		return true;
	}else{
		Printf0(
			"test7_createMV failed!\n", 
			sizeof("test7_createMV failed!\n"));
		return false;
	}
}

/*
*	Test 8 - SetMV
*		This will test whether get and set methods work
*/
bool test8_setAndGetMV() {
	int mv0, mv1, mv2;
	int mv0_size = 1, mv1_size = 3;
	int rn_negSetIndex, rn_bigSetIndex, rp_validSetIndex, rn_negIndex, rn_bigIndex;
	int rn_negGetIndex, rn_bigGetIndex, rn_negMVIndex, rn_bigMVIndex; /* r stands for result. p = pos, n = neg */
	int value = 23;

	Printf0("Running test7_createMV\n", sizeof("Running test7_createMV\n"));

	mv0 = CreateMV("mv0", sizeof("mv0"), mv0_size);
	mv1 = CreateMV("mv1", sizeof("mv1"), mv1_size);
	mv2 = CreateMV("mv0", sizeof("mv0"), mv0_size);

	/* Invalid Set */
	rn_negSetIndex = SetMV(mv1, -1, value);
	rn_bigSetIndex = SetMV(mv1, mv1_size, value);
	rn_negIndex = SetMV(-1, 0, 0);
	rn_bigIndex = SetMV(2, 0, 0);
	/* Valid Set */
	rp_validSetIndex = SetMV(mv1, mv1_size - 1, value);

	/* Invalid Get */
	rn_negGetIndex = GetMV(mv1, -1);
	rn_bigGetIndex = GetMV(mv1, mv1_size);
	rn_negMVIndex = GetMV(-1, 0);
	rn_bigMVIndex = GetMV(2, 0);

	if(
		GetMV(mv1, mv1_size - 1) == value /* Valid Get */
		&& rn_negSetIndex == -1
		&& rn_bigSetIndex == -1
		&& rn_negIndex == -1
		&& rn_bigIndex == -1
		&& rn_negGetIndex == -1
		&& rn_bigGetIndex == -1
		&& rn_negMVIndex == -1
		&& rn_bigMVIndex == -1
	) {
		Printf0("test7_createMV passed!\n", sizeof("test7_createMV passed!\n"));
		return true;
	}else{
		Printf0(
			"test7_createMV failed!\n", 
			sizeof("test7_createMV failed!\n"));
		return false;
	}

	return false;
}

/*
*	Test 9 - Destroy Monitor Variable
*/
bool test9_deleteMV() {
	int mv0, mv1, mv2;
	int mv0_size = 3, mv1_size = 1;
	int rp_goodDelete, rn_deletedIndex, rn_negIndex, rn_bigIndex;

	Printf0("Running test9_deleteMV\n", sizeof("Running test9_deleteMV\n"));

	/* Check DEstroy functionality */
	mv0 = CreateMV("mv0", sizeof("mv0"), mv0_size);
	mv1 = CreateMV("mv1", sizeof("mv1"), mv1_size);
	rp_goodDelete = DestroyMV(mv1);
	mv2 = CreateMV("mv2", sizeof("mv2"), 23);
	rn_deletedIndex = GetMV(mv1, 0);

	/* Check Destroy boundaries */
	rn_negIndex = DestroyMV(-1);
	rn_bigIndex = DestroyMV(3);

	if(
		rp_goodDelete == mv1
		&& rn_negIndex == -1
		&& rn_bigIndex == -1
		&& rn_deletedIndex == -1
		&& mv2 == 2
		) {
		Printf0("test9_deleteMV passed!\n", sizeof("test9_deleteMV passed!\n"));
		return true;
	}else{
		Printf1(
			"test9_deleteMV failed! %d %d %d %d\n", 
			sizeof("test9_deleteMV failed! %d %d %d\n"),
			1000000*rp_goodDelete + 1000*rn_negIndex + rn_bigIndex);
		return false;
	}
}

/*
*
*
*	"main"
*
*
*/
int main() {
	/*test0_createLock();*/
	/*test1_deleteLock();*/
	/*test2_acquireLock();*/
	/*test3_releaseAndDestroy();*/
	/*test4_createCV();*/
	/*test5_destroyCV();*/
	/*test6_waitAndSignal();*/
	/*test7_createMV();*/
	/*test8_setAndGetMV();*/
	test9_deleteMV();

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

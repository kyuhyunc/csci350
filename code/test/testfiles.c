/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

/*initialize all int variable that be used as return value of create LOCK and CV so we can use it as index in lock/CV table for acquire/release/wait/signal/broadcast*/
 int LockIndex1, LockIndex2, LockIndex3, LockIndex4, LockIndex5, LockIndex6, LockIndex7, LockIndex8;
 int CVIndex1, CVIndex2, CVIndex3, CVIndex4, CVIndex5, CVIndex6, CVIndex7, CVIndex8;
 int invCheck1, invCheck2;
 int testing1 = 1;
 int testing2 = 0;
 int testing3 = 0;
 int testing5 = 0;
void testStart2();
void testStart3();
void testStart4();
void testStart5();
void testStart6();
void testStart7();
void testStart8();
/*fucntion 1,2,3 are used for one test to see wait are actually waiting for the Broadcast.*/
/*This first test is for testing if wait is actaully waiting until it gets signal/broadcast*/
/*function 1 is called first and function 2 is next and function 3 is the last in order to see if broadcast waits other contion variable work*/
void function1() {
	/*In this function, it increments integer(testing1) by 5 after function 3 wake it up*/
	Acquire(LockIndex1);
	Wait(LockIndex1, CVIndex1);
	testing1 = testing1 + 5;
	Release(LockIndex1);
	DestroyLock(LockIndex1);
	DestroyCV(CVIndex1);
	Exit(0);
}
void function2() {
	/*In this function, it increments integer(testing1) by 3 after function 3 wake it up*/
	Acquire(LockIndex1);
	Wait(LockIndex1, CVIndex1);
	testing1 = testing1 + 3;
	Release(LockIndex1);
	/*Sinec this function is waken up the last so integer value all calculation 
	Therefore, we need to check the result and if we can get the expected result, then the test passes*/
	if(testing1 == 10) {
		Write("Broadcast/signal test passed\n", sizeof("Broadcast/signal test passed\n"), ConsoleOutput);
	}else {
		Write("Broadcast/signal test failed\n", sizeof("Broadcast/signal test failed\n"), ConsoleOutput);
	}
	/*At this point, we are done with the test so going to the next function that we can start the next test!*/
	Fork(testStart2, "changeme", sizeof("changeme"));
	Exit(0);
}
void function3() {
	/*In this function, it mutiply the integer(testing1) before waking other condition variable in other two function.
	in order to see if this broadcast syscall actually wake others condition variable up.
	Otherwise, integer value is going to be different at the end. */
	Acquire(LockIndex1);
	testing1 = testing1 * 2;
	Broadcast(LockIndex1, CVIndex1);
	Release(LockIndex1);
	Exit(0);
}
/*function4, function5, and function6 are used for test2.
this function test whether lock and condition variable can wake up different lock and CV
Since it shouldn't, function 4 and function 6 have the same Lock and CV, fucntion 4 waits for function6 waking function4 up, but
function 6 can't wake function 5 up, which means that integer(testing2) can never be incremented
Therefore, at the end, we can check the integer value if the integer value is bigger than expectation.
*/
void function4() {
	Acquire(LockIndex2);
	Wait(LockIndex2, CVIndex2);
	testing2 = testing2 + 1;	/*incremented after it wakes up*/
	Release(LockIndex2);
	Exit(0);
}
void function5() {
	Acquire(LockIndex3);
	Wait(LockIndex3, CVIndex3);
	testing2 = testing2 + 1;	/*incremented after it wakes up*/
	Release(LockIndex3);
	Exit(0);
}
void function6() {
	Acquire(LockIndex2);

	Broadcast(LockIndex2, CVIndex2); /*it only wakes up function4*/

	Release(LockIndex2);
	/*this function is called the last among the functions that are used for the same test.
	Check if we get expeted integer value, then print out the result and FORK next text function*/
	if(testing2 == 1) {
		Write("Two different lock TEST passed\n", sizeof("Two different lock TEST passed\n"), ConsoleOutput);
		Fork(testStart3, "changeme", sizeof("changeme"));
	}else {
		Write("Two different lock TEST failed\n", sizeof("Two different lock TEST failed\n"), ConsoleOutput);
		Fork(testStart3, "changeme", sizeof("changeme"));
	}
	Exit(0);
}
/*Three functions(function7/function8/function9) are used for this test.
basic setup is the same except for putting Destory Lock after wait syscall in function7
it should not create error even though we destory lock before lock is released*/
void function7() {
	Acquire(LockIndex4);
	Wait(LockIndex4, CVIndex4);
	DestroyLock(LockIndex4);
	testing3 = testing3 + 1;
	Release(LockIndex4);
	Exit(0);
}
void function8() {
	Acquire(LockIndex4);
	Wait(LockIndex4, CVIndex4);
	testing3 = testing3 + 1;
	Release(LockIndex4);
	/*this function is called the last among the functions that are used for the same test. print out the result and FORK next text function*/
	if(testing3 == 2) {
		Write("DestroyLock TEST passed\n", sizeof("DestroyLock TEST passed\n"), ConsoleOutput);
		Fork(testStart4, "changeme", sizeof("changeme"));
	}else {
		Write("DestroyLock TEST failed\n", sizeof("DestroyLock TEST failed\n"), ConsoleOutput);
		Fork(testStart4, "changeme", sizeof("changeme"));
	}
	Exit(0);
}
void function9() {
	Acquire(LockIndex4);
	Broadcast(LockIndex4, CVIndex4);
	Release(LockIndex4);
	Exit(0);
}
/*only one function(function10) is used for test 4
intentionally put invalid index in acqurie and release and see if it return -1 and print correct erorr message!*/
void function10() {
	invCheck1 = Acquire(LockIndex5+20);
	Broadcast(LockIndex4, CVIndex4);
	invCheck2 = Release(LockIndex4+20);
	/*this function is called the last among the functions that are used for the same test. print out the result and FORK next text function*/
	if(invCheck1 == - 1 && invCheck2 == -1) {
		Write("invalid index TEST passed\n", sizeof("invalid index TEST passed\n"), ConsoleOutput);
		Fork(testStart5, "changeme", sizeof("changeme"));
	}else{
		Write("invalid index TEST failed\n", sizeof("invalid index TEST failed\n"), ConsoleOutput);
		Fork(testStart5, "changeme", sizeof("changeme"));
	}
	Exit(0);
}
/*Three functions(function11/function12/function13) are used for this test.
basic setup is the same except for putting Destory CV before wait syscall in function11
it should not create error even though we destory CV before wait syscall does not wake up*/
void function11() {
	invCheck1 = Acquire(LockIndex6);
	DestroyCV(CVIndex6);
	Wait(LockIndex6, CVIndex6);
	testing5 = testing5 + 1;
	invCheck2 = Release(LockIndex6);
	Exit(0);
}
void function12() {
	invCheck1 = Acquire(LockIndex6);
	Wait(LockIndex6, CVIndex6);
	testing5 = testing5 + 1;
	invCheck2 = Release(LockIndex6);
	/*this function is called the last among the functions that are used for the same test. print out the result and FORK next text function*/
	if(testing5) {
		Write("Destroy CV TEST passed\n", sizeof("Destroy CV TEST passed\n"), ConsoleOutput);
		Fork(testStart6, "changeme", sizeof("changeme"));
	}else{
		Write("Destroy CV TEST failed\n", sizeof("Destroy CV TEST failed\n"), ConsoleOutput);
		Fork(testStart6, "changeme", sizeof("changeme"));
	}
	Exit(0);
}
void function13() {
	invCheck1 = Acquire(LockIndex6);
	Broadcast(LockIndex6, CVIndex6);
	invCheck2 = Release(LockIndex6);
	Exit(0);
}
void testStart2() {
	/*this function starts test 2.
	In this function two Locks and CVs are created by syscall so that we can see if there are interation between two different Locks and CVs
	Three function are used for this test and they are forked after creating locks and CVs*/
	Write("Test2: Two different lock TEST\n", sizeof("Test2: Two different lock TEST\n"), ConsoleOutput);
	LockIndex2 = CreateLock("SecondLOCK", 9);
	CVIndex2 = CreateCV("SecondCV", 7);
	LockIndex3 = CreateLock("ThirdLOCK", 9);
	CVIndex3 = CreateCV("ThirdCV", 7);
	/*Fork three functions that are used for the test!*/
	Fork(function4, "changeme", sizeof("changeme"));
	Fork(function5, "changeme", sizeof("changeme"));
	Fork(function6, "changeme", sizeof("changeme"));
	Exit(0);
}
void testStart3() {
	/*for this test, one Lock and CV are created and three functions are used
	it fork function 7 first then function8 and function9 */
	Write("Test3: DestroyLock TEST\n", sizeof("Test3: DestroyLock TEST\n"), ConsoleOutput);
	LockIndex4 = CreateLock("FOURTHLOCK", 10);
	CVIndex4 = CreateCV("FOURTHCV", 8);
	/*Fork three functions that are used for the test!*/
	Fork(function7, "changeme", sizeof("changeme"));
	Fork(function8, "changeme", sizeof("changeme"));
	Fork(function9, "changeme", sizeof("changeme"));
	Exit(0);
}
void testStart4() {
	/*For this test, One lock and CV are created and one function is used.*/
	Write("Test4: Invalid index TEST\n", sizeof("Test4: Invalid index TEST\n"), ConsoleOutput);
	LockIndex5 = CreateLock("FIFTHLOCK", 9);
	CVIndex5 = CreateCV("FIFTHCV", 7);
	/*Fork function that are used for the test!*/
	Fork(function10, "changeme", sizeof("changeme"));
	Exit(0);
}
void testStart5() {
	/*for this test, one Lock and CV are created and three functions are used
	it fork function 7 first then function8 and function9 */
	Write("Test5: Destroy CV TEST\n", sizeof("Test5: Destroy CV TEST\n"), ConsoleOutput);
	LockIndex6 = CreateLock("SIXTHLOCK", 9);
	CVIndex6 = CreateCV("SIXTHCV", 7);
	/*Fork three functions that are used for the test!*/
	Fork(function11, "changeme", sizeof("changeme"));
	Fork(function12, "changeme", sizeof("changeme"));
	Fork(function13, "changeme", sizeof("changeme"));
	Exit(0);
}
void testStart6() {
	/*For this test, we are making environment that LOCK and CV going to be full. then I see if we can add more in the table that is full*/
	int i;
	Write("Test6: 'Trying to create LOCK / CV over Maximum' TEST\n", sizeof("Test6: 'Trying to create LOCK / CV over Maximum' TEST\n"), ConsoleOutput);
	LockIndex8 = CreateLock("SIXTHLOCK", 9);
	CVIndex7 = CreateCV("SIXTHCV", 7);
	CVIndex8 = CreateCV("SIXTHCH", 7);
	for(i = 0; i < 997	; i++) {
		LockIndex8 = CreateLock("SIXTHLOCK", 9);
		CVIndex8 = CreateCV("SIXTHCH", 7);
	}
	if(LockIndex8 == -1 && CVIndex8 == -1) {
		Write("'Trying to create LOCK / CV over Maximum TEST' passed\n", sizeof("'Trying to create LOCK / CV over Maximum TEST' passed\n"), ConsoleOutput);
	}else{
		Write("'Trying to create LOCK / CV over Maximum TEST' failed\n", sizeof("'Trying to create LOCK / CV over Maximum TEST' failed\n"), ConsoleOutput);
	}
	/*Fork function that are used for the test!*/
	Fork(testStart7, "changeme", sizeof("changeme"));
	Exit(0);
}
void testStart7() {
	/*for this test, put invalid integer value in syscall to see if they create ERROR and returns -1*/
	int test7_1, test7_2, test7_3, test7_4, test7_5, test7_6, test7_7;
	Write("Test7: 'Passing in invalid index' TEST\n", sizeof("Test7: 'Passing in invalid index' TEST\n"), ConsoleOutput);

	test7_1 = Acquire(LockIndex8);
	test7_2 = Release(CVIndex8);
	test7_3 = Acquire(1001);
	test7_4 = Release(-5);
	test7_5 = Wait(LockIndex8, CVIndex8);
	test7_6 = Signal(-1, 5000);
	test7_7 = Broadcast(-50, 60);

	if(test7_1 == -1 && test7_2 == -1 && test7_3 == -1 && test7_4 == -1 && test7_5 == -1 && test7_6 == -1 && test7_7 == -1) {
		Write("'Passing in invalid index TEST' passed\n", sizeof("'Passing in invalid index TEST' passed\n"), ConsoleOutput);
		Fork(testStart8, "changeme", sizeof("changeme"));
	}else{
		Write("'Passing in invalid index TEST' failed\n", sizeof("'Passing in invalid index TEST' failed\n"), ConsoleOutput);
		Fork(testStart8, "changeme", sizeof("changeme"));
	}

	Exit(0);
}
void testStart8() {
	/*text if exec is correctly working!*/
	Write("Test8: Exec TEST\n", sizeof("Test7: 'Exec' TEST\n"), ConsoleOutput);


	Exit(0);	
}
int main() {

	OpenFileId fd;
	int bytesread;
	char buf[20];

	Create("testfile", 8);
	fd = Open("testfile", 8);

	Write("testing a write\n", 16, fd );
	Close(fd);

	fd = Open("testfile", 8);
	bytesread = Read( buf, 100, fd );
	Write( buf, bytesread, ConsoleOutput );
	Close(fd);
	Write("Test1: Broadcast/signal TEST\n", sizeof("Test1: Broadcast/signal TEST\n"), ConsoleOutput);

	LockIndex1 = CreateLock("FirstLOCK", 9);
	CVIndex1 = CreateCV("FirstCV", 7);
	/*we fork these threee function so that we can start the first test for syscall!*/
	Fork(function1, "changeme", sizeof("changeme"));
	Fork(function2, "changeme", sizeof("changeme"));
	Fork(function3, "changeme", sizeof("changeme"));

}

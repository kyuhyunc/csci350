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
void function1() {
	Acquire(LockIndex1);
	Wait(LockIndex1, CVIndex1);
	testing1 = testing1 + 5;
	Release(LockIndex1);
	DestroyLock(LockIndex1);
	DestroyCV(CVIndex1);
	Exit(0);
}
void function2() {
	Acquire(LockIndex1);
	Wait(LockIndex1, CVIndex1);
	testing1 = testing1 + 3;
	Release(LockIndex1);
	/*since this function is finished the last among three functions.*/
	if(testing1 == 10) {
		Write("Broadcast/signal test passed\n", sizeof("Broadcast/signal test passed\n"), ConsoleOutput);
	}else {
		Write("Broadcast/signal test failed\n", sizeof("Broadcast/signal test failed\n"), ConsoleOutput);
	}
	/*Fork test2 function so we can start the next one!*/
	Fork(testStart2, "changeme", sizeof("changeme"));
	Exit(0);
}
void function3() {
	Acquire(LockIndex1);
	testing1 = testing1 * 2;
	Broadcast(LockIndex1, CVIndex1);
	Release(LockIndex1);
	Exit(0);
}
void function4() {
	Acquire(LockIndex2);
	Wait(LockIndex2, CVIndex2);
	testing2 = testing2 + 1;
	Release(LockIndex2);
	Exit(0);
}
void function5() {
	Acquire(LockIndex3);

	testing2 = testing2 + 1;
	Release(LockIndex3);
	Exit(0);
}
void function6() {
	Acquire(LockIndex2);

	Signal(LockIndex2, CVIndex2);

	Release(LockIndex2);
	/*this function is called the last among the functions that are used for the same test. print out the result and FORK next text function*/
	if(testing2 == 1) {
		Write("Two different lock TEST passed\n", sizeof("Two different lock TEST passed\n"), ConsoleOutput);
		Fork(testStart3, "changeme", sizeof("changeme"));
	}else {
		Write("Two different lock TEST failed\n", sizeof("Two different lock TEST failed\n"), ConsoleOutput);
		Fork(testStart3, "changeme", sizeof("changeme"));
	}
	Exit(0);
}
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
	Write("Test4: Invalid index TEST\n", sizeof("Test4: Invalid index TEST\n"), ConsoleOutput);
	LockIndex5 = CreateLock("FIFTHLOCK", 9);
	CVIndex5 = CreateCV("FIFTHCV", 7);
	/*Fork function that are used for the test!*/
	Fork(function10, "changeme", sizeof("changeme"));
	Exit(0);
}
void testStart5() {

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
	for(i = 0; i < 996; i++) {
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
	Write("Test8: '' TEST\n", sizeof("Test7: '' TEST\n"), ConsoleOutput);
	Exec("../test/testexit", sizeof("../test/testexit"));


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

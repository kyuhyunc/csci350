/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

<<<<<<< HEAD
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
	
=======
 int LockIndex1, LockIndex2, LockIndex3, LockIndex4, LockIndex5, LockIndex6;
 int CVIndex1, CVIndex2, CVIndex3, CVIndex4, CVIndex5, CVIndex6;
 int invCheck1, invCheck2;
 int testing1 = 1;
 int testing2 = 0;
 int testing3 = 0;
 int testing5 = 0;
void testStart2();
void testStart3();
void testStart4();
void testStart5();

void function1() {
	Acquire(LockIndex1);
	Wait(LockIndex1, CVIndex1);
	Write("2\n", 2, ConsoleOutput);
	testing1 = testing1 + 5;
	Release(LockIndex1);
	DestroyLock(LockIndex1);
	DestroyCV(CVIndex1);
	Exit(0);
}
void function2() {
	Acquire(LockIndex1);
	Wait(LockIndex1, CVIndex1);
	Write("3\n", 2, ConsoleOutput);
	testing1 = testing1 + 3;
	Release(LockIndex1);
	if(testing1 == 10) {
		Write("Broadcast/signal test passed\n", sizeof("Broadcast/signal test passed\n"), ConsoleOutput);
	}else {
		Write("Broadcast/signal test failed\n", sizeof("Broadcast/signal test failed\n"), ConsoleOutput);
	}
	Fork(testStart2);
	Exit(0);
}
void function3() {
	Acquire(LockIndex1);
	Write("1\n", 2, ConsoleOutput);
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
	if(testing2 == 1) {
		Write("Two different lock TEST passed\n", sizeof("Two different lock TEST passed\n"), ConsoleOutput);
		Fork(testStart3);
	}else {
		Write("Two different lock TEST failed\n", sizeof("Two different lock TEST failed\n"), ConsoleOutput);
		Fork(testStart3);
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
	if(testing3 == 2) {
		Write("DestroyLock TEST passed\n", sizeof("DestroyLock TEST passed\n"), ConsoleOutput);
		Fork(testStart4);
	}else {
		Write("DestroyLock TEST failed\n", sizeof("DestroyLock TEST failed\n"), ConsoleOutput);
		Fork(testStart4);
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
	if(invCheck1 == - 1 && invCheck2 == -1) {
		Write("invalid index TEST passed\n", sizeof("invalid index TEST passed\n"), ConsoleOutput);
		Fork(testStart5);
	}else{
		Write("invalid index TEST failed\n", sizeof("invalid index TEST failed\n"), ConsoleOutput);
		Fork(testStart5);
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
	if(testing5) {
		Write("Destroy CV TEST passed\n", sizeof("Destroy CV TEST passed\n"), ConsoleOutput);
	}else{
		Write("Destroy CV TEST failed\n", sizeof("Destroy CV TEST failed\n"), ConsoleOutput);
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
	
	Fork(function4);
	Fork(function5);
	Fork(function6);
	Exit(0);
}
void testStart3() {
	Write("Test3: DestroyLock TEST\n", sizeof("Test3: DestroyLock TEST\n"), ConsoleOutput);
	LockIndex4 = CreateLock("FOURTHLOCK", 10);
	CVIndex4 = CreateCV("FOURTHCV", 8);

	Fork(function7);
	Fork(function8);
	Fork(function9);
	Exit(0);
}
void testStart4() {
	Write("Test4: Invalid index TEST\n", sizeof("Test4: Invalid index TEST\n"), ConsoleOutput);
	LockIndex5 = CreateLock("FIFTHLOCK", 9);
	CVIndex5 = CreateCV("FIFTHCV", 7);

	Fork(function10);
	Exit(0);
}
void testStart5() {
	Write("Test5: Destroy CV TEST\n", sizeof("Test5: Destroy CV TEST\n"), ConsoleOutput);
	LockIndex6 = CreateLock("SIXTHLOCK", 9);
	CVIndex6 = CreateCV("SIXTHCV", 7);


	Fork(function11);
	Fork(function12);
	Fork(function13);
>>>>>>> p2_taegyum
	Exit(0);
}

int main() {

<<<<<<< HEAD
/*	Fork(a);
	Fork(b);
	Fork(c);
	Fork(d);
	Fork(e);*/

/*	Printf0("About to call Exec...\n", sizeof("About to call Exec...\n"));*/
	Fork(test_print);
	Fork(newpf);
	Exec("../test/halt", sizeof("../test/halt"));

	Exit(0);
}
=======
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

	LockIndex1 = CreateLock("FirstLOCK", 9);
	CVIndex1 = CreateCV("FirstCV", 7);
	
	Fork(function1);
	Fork(function2);
	Fork(function3);

}
>>>>>>> p2_taegyum

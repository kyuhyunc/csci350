/*
Testing File Number one for Project 4
This should be run before running test 2
*/

#include "syscall.h"

void useMV();

void createWait() {
	int firstLock;
	int firstCV;

	firstLock = CreateLock("first", sizeof("first"));
	firstCV = CreateCV("first", sizeof("first"));

	Acquire(firstLock);
	Wait(firstLock, firstCV);
	Printf0("after wait in test 1\n", sizeof("after wait in test 1\n"));
	Release(firstLock);
	Printf0("after release in test 1\n", sizeof("after release in test 1\n"));


	useMV();

	Exit(0);
}

void useMV() {
	int firstMV, secondMV;
	int gettingMV1, gettingMV2;
	Printf0("Getting MV!\n", sizeof("GettingMV!\n"));
	firstMV = CreateMV("firstMV", sizeof("firstMV"), 10);
	secondMV = CreateMV("secondMV", sizeof("secondMV"), 9);

	gettingMV1 = GetMV(firstMV, 4);
	gettingMV2 = GetMV(secondMV, 2);

	Printf1(
	"Checking MV %d %d %d\n", 
	sizeof("Checking MV %d %d %d\n"),
	1000000*gettingMV1 + 1000*gettingMV2 + 21);
}
int main() {
	Fork(createWait, "createWait", sizeof("createWait"));
	Fork(createWait, "createWait", sizeof("createWait"));
	Fork(createWait, "createWait", sizeof("createWait"));
}
/*
Testing File Number one for Project 4
This should be run before running test 2
*/

#include "syscall.h"


void createWait() {
	int firstLock;
	int firstCV;

	firstLock = CreateLock("first", sizeof("first"));
	firstCV = CreateCV("first", sizeof("first"));

	Acquire(firstLock);
	Wait(firstLock, firstCV);
	Printf0("after wait in test 1", sizeof("after wait in test 1"));
	Release(firstLock);
	Printf0("after release in test 1", sizeof("after release in test 1"));

	Exit(0);
}

int main() {
	Fork(createWait, "createWait", sizeof("createWait"));
	Fork(createWait, "createWait", sizeof("createWait"));
	Fork(createWait, "createWait", sizeof("createWait"));
}
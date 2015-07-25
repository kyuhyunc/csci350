/*
Testing File Number two for Project 4
*/

#include "syscall.h"

void setMV();

void createSignal() {
	int secondLock;
	int secondCV;

	secondLock = CreateLock("first", sizeof("first"));
	secondCV = CreateCV("first", sizeof("first"));

	Acquire(secondLock);
	Broadcast(secondLock, secondCV);
	Printf0("after signal in test 2\n", sizeof("after signal in test 2\n"));
	Release(secondLock);
	Printf0("after release in test 2\n",  sizeof("after release in test 2\n"));

	setMV();

	Exit(0);
}

void setMV() {
	int firstMV, secondMV;
	int MVsetting1, MVsetting2;
	Printf0("Setting MV!\n", sizeof("SettingMV!\n"));
	firstMV = CreateMV("firstMV", sizeof("firstMV"), 10);
	secondMV = CreateMV("secondMV", sizeof("secondMV"), 9);

	MVsetting1 = SetMV(firstMV, 4, 100);
	MVsetting2 = SetMV(secondMV, 2, 777);
}


int main() {
	
	createSignal();

}
/*
Testing File Number two for Project 4
*/

#include "syscall.h"

void createSignal() {
	int secondLock;
	int secondCV;

	secondLock = CreateLock("first", sizeof("first"));
	secondCV = CreateCV("first", sizeof("first"));

	Acquire(secondLock);
	Broadcast(secondLock, secondCV);
	Printf0("after signal in test 2", sizeof("after signal in test 2"));
	Release(secondLock);
	Printf0("after release in test 2",  sizeof("after release in test 2"));

	Exit(0);
}


int main() {
	
	createSignal();

}
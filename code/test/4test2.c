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
	Signal(secondLock, secondCV);
	printf0("after signal in test 2", sizeof("after signal in test 2"));
	Release(secondLock);
	printf0("after release in test 2",  sizeof("after release in test 2"));

	Exit(0);
}


int main() {
	
	createSignal();

}
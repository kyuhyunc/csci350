/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int firstLock, secondLock;
int firstCV, secondCV;
void testing2();
void testing1() {
	Printf0("Entering testing1\n", sizeof("Entering testing1\n"));
	firstLock = CreateLock("first", sizeof("first"));
	firstCV = CreateCV("first", sizeof("first"));
	Acquire(firstLock);
	Fork(testing2, "testing2", sizeof("testing2"));
	Printf0("After Acquire\n", sizeof("After Acquire\n"));
	Wait(firstLock,firstCV);
	Printf0("After Wait\n", sizeof("After Wait\n"));
	Release(firstLock);
	Printf0("After Release 1\n", sizeof("After Release 1\n"));

	Exit(0);
}
void testing2() {
	Printf0("Entering testing2\n", sizeof("Entering testing2\n"));
/*	secondLock = CreateLock("first", sizeof("first"));
	secondCV = CreateCV("first", sizeof("first"));*/
	Acquire(firstLock);
	Printf0("After Acquire 2 \n", sizeof("After Acquire 2 \n"));
	Signal(firstLock, firstCV);
	Printf0("After signal 2 \n", sizeof("After signal 2 \n"));
	Release(firstLock);
	Printf0("After Release 2\n", sizeof("After Release 2\n"));

	Exit(0);
}

int
main()
{
	Printf0("Testing Starts\n", sizeof("Testing Starts\n"));
	Fork(testing1, "testing1", sizeof("testing1"));
	


}

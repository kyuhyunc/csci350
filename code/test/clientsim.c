/*
*
*	A Nachos user program that interacts with the Server.
*	Project 3
*
*/
#include "syscall.h"

/*
*	"main"
*/
int main() {
	int lock = CreateLock("lock1", sizeof("lock1"));
	Printf1("Lock id: %d\n", sizeof("Lock id: %d\n"), lock);
	Exit(0);
}
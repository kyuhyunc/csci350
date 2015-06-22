/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


void a() {
	Printf0("printf0", sizeof("printf0"));
	Write("a\n", 2, ConsoleOutput);
	Exit(0);
}

void b() {
	Write("b\n", 2, ConsoleOutput);
	Exit(0);
}

void c() {
	Write("c\n", 2, ConsoleOutput);
	Exit(0);
}

void d() {
	Write("d\n", 2, ConsoleOutput);
	Exit(0);
}

void e() {
	Write("e\n", 2, ConsoleOutput);
	Exit(0);
}
void f() {
	Acquire(0);
	Wait(0, 0);
	Write("f\n", 2, ConsoleOutput);
	Release(0);
	Exit(0);
}
void g() {
	Acquire(0);
	Write("g\n", 2, ConsoleOutput);
	Signal(0, 0);
	Release(0);
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

	
	CreateLock("FirstLOCK", 9);
	CreateCV("FirstCV", 7);
	
	/*
	Fork(a);
	Fork(b);
	Fork(c);
	Fork(d);
	Fork(e);
	*/
	
	Fork(f);
	Fork(g);



}
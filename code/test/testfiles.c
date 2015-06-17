/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void a() {
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

	Fork(a);
	Fork(b);
	Fork(c);
	Fork(d);
	Fork(e);
}

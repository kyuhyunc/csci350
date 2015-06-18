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
	Printf1("printf%d", sizeof("printf%d"), 1);
	Write("b\n", 2, ConsoleOutput);
	Exit(0);
}

void c() {
	Printf2("printf%d, works %d!", sizeof("printf%d, works %d!"), 2, 2);
	Write("c\n", 2, ConsoleOutput);
	Exit(0);
}

void d() {
	Printf0("printf0d\n", sizeof("printf0d\n"));
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

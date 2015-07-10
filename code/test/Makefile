# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = 
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR = 
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort matmult2 sort2 testfiles testprint testexit testexithelper airportsim airstart

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

sort2.o: sort2.c
	$(CC) $(CFLAGS) -c sort2.c
sort2: sort2.o start.o
	$(LD) $(LDFLAGS) start.o sort2.o -o sort2.coff
	../bin/coff2noff sort2.coff sort2

matmult2.o: matmult2.c
	$(CC) $(CFLAGS) -c matmult2.c
matmult2: matmult2.o start.o
	$(LD) $(LDFLAGS) start.o matmult2.o -o matmult2.coff
	../bin/coff2noff matmult2.coff matmult2

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

testprint.o: testprint.c
	$(CC) $(CFLAGS) -c testprint.c
testprint: testprint.o start.o
	$(LD) $(LDFLAGS) start.o testprint.o -o testprint.coff
	../bin/coff2noff testprint.coff testprint

testexit.o: testexit.c
	$(CC) $(CFLAGS) -c testexit.c
testexit: testexit.o start.o
	$(LD) $(LDFLAGS) start.o testexit.o -o testexit.coff
	../bin/coff2noff testexit.coff testexit

testexithelper.o: testexithelper.c
	$(CC) $(CFLAGS) -c testexithelper.c
testexithelper: testexithelper.o start.o
	$(LD) $(LDFLAGS) start.o testexithelper.o -o testexithelper.coff
	../bin/coff2noff testexithelper.coff testexithelper

airportsim.o: airportsim.c
	$(CC) $(CFLAGS) -c airportsim.c
airportsim: airportsim.o start.o
	$(LD) $(LDFLAGS) start.o airportsim.o -o airportsim.coff
	../bin/coff2noff airportsim.coff airportsim

airstart.o: airstart.c
	$(CC) $(CFLAGS) -c airstart.c
airstart: airstart.o start.o
	$(LD) $(LDFLAGS) start.o airstart.o -o airstart.coff
	../bin/coff2noff airstart.coff airstart

clean:
	rm -f *.o *.coff
	rm halt sort matmult sort2 matmult2 testfiles testprint testexit testexithelper airportsim airstart
	rm *~

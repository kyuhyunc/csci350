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

all: halt matmult sort matmult2exec matmult2fork sort2exec sort2fork testfiles testprint testexit testexithelper airportsim airstart clientsim passenger
	#client1 client2 client3 client4 client5 passenger screeningofficer checkinstaff liaison securityinspector manager cargohandler


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

sort2exec.o: sort2exec.c
	$(CC) $(CFLAGS) -c sort2exec.c
sort2exec: sort2exec.o start.o
	$(LD) $(LDFLAGS) start.o sort2exec.o -o sort2exec.coff
	../bin/coff2noff sort2exec.coff sort2exec

sort2fork.o: sort2fork.c
	$(CC) $(CFLAGS) -c sort2fork.c
sort2fork: sort2fork.o start.o
	$(LD) $(LDFLAGS) start.o sort2fork.o -o sort2fork.coff
	../bin/coff2noff sort2fork.coff sort2fork

matmult2exec.o: matmult2exec.c
	$(CC) $(CFLAGS) -c matmult2exec.c
matmult2exec: matmult2exec.o start.o
	$(LD) $(LDFLAGS) start.o matmult2exec.o -o matmult2exec.coff
	../bin/coff2noff matmult2exec.coff matmult2exec

matmult2fork.o: matmult2fork.c
	$(CC) $(CFLAGS) -c matmult2fork.c
matmult2fork: matmult2fork.o start.o
	$(LD) $(LDFLAGS) start.o matmult2fork.o -o matmult2fork.coff
	../bin/coff2noff matmult2fork.coff matmult2fork

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

airportsim.o: airportsim.c create.h
	$(CC) $(CFLAGS) -c airportsim.c
airportsim: airportsim.o start.o
	$(LD) $(LDFLAGS) start.o airportsim.o -o airportsim.coff
	../bin/coff2noff airportsim.coff airportsim

airstart.o: airstart.c
	$(CC) $(CFLAGS) -c airstart.c
airstart: airstart.o start.o
	$(LD) $(LDFLAGS) start.o airstart.o -o airstart.coff
	../bin/coff2noff airstart.coff airstart

clientsim.o: clientsim.c
	$(CC) $(CFLAGS) -c clientsim.c
clientsim: clientsim.o start.o
	$(LD) $(LDFLAGS) start.o clientsim.o -o clientsim.coff
	../bin/coff2noff clientsim.coff clientsim

client1.o: client1.c
	$(CC) $(CFLAGS) -c client1.c
client1: client1.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o client1.coff
	../bin/coff2noff client1.coff client1

client2.o: client2.c
	$(CC) $(CFLAGS) -c client2.c
client2: client2.o start.o
	$(LD) $(LDFLAGS) start.o client2.o -o client2.coff
	../bin/coff2noff client2.coff client2

client3.o: client3.c
	$(CC) $(CFLAGS) -c client3.c
client3: client3.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o client3.coff
	../bin/coff2noff client3.coff client3

client4.o: client4.c
	$(CC) $(CFLAGS) -c client4.c
client4: client4.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o client4.coff
	../bin/coff2noff client4.coff client4

client5.o: client5.c
	$(CC) $(CFLAGS) -c client5.c
client5: client5.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o client5.coff
	../bin/coff2noff client5.coff client5

passenger.o: passenger.c
	$(CC) $(CFLAGS) -c passenger.c
passenger: passenger.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o passenger.coff
	../bin/coff2noff passenger.coff passenger

# screeningofficer.o: screeningofficer.c
# 	$(CC) $(CFLAGS) -c screeningofficer.c
# screeningofficer: screeningofficer.o start.o
# 	$(LD) $(LDFLAGS) start.o client1.o -o screeningofficer.coff
# 	../bin/coff2noff screeningofficer.coff screeningofficer

# checkinstaff.o: checkinstaff.c
# 	$(CC) $(CFLAGS) -c checkinstaff.c
# checkinstaff: checkinstaff.o start.o
# 	$(LD) $(LDFLAGS) start.o client1.o -o checkinstaff.coff
# 	../bin/coff2noff checkinstaff.coff checkinstaff

# liaison.o: liaison.c
# 	$(CC) $(CFLAGS) -c liaison.c
# liaison: liaison.o start.o
# 	$(LD) $(LDFLAGS) start.o client1.o -o liaison.coff
# 	../bin/coff2noff liaison.coff liaison

# manager.o: manager.c
# 	$(CC) $(CFLAGS) -c manager.c
# manager: manager.o start.o
# 	$(LD) $(LDFLAGS) start.o client1.o -o manager.coff
# 	../bin/coff2noff manager.coff manager

# securityinspector.o: securityinspector.c
# 	$(CC) $(CFLAGS) -c securityinspector.c
# securityinspector: securityinspector.o start.o
# 	$(LD) $(LDFLAGS) start.o client1.o -o securityinspector.coff
# 	../bin/coff2noff securityinspector.coff securityinspector

# cargohandler.o: cargohandler.c
# 	$(CC) $(CFLAGS) -c cargohandler.c
# cargohandler: cargohandler.o start.o
# 	$(LD) $(LDFLAGS) start.o client1.o -o cargohandler.coff
# 	../bin/coff2noff cargohandler.coff cargohandler

clean:
	rm -f *.o *.coff
	rm halt sort matmult sort2exec sort2fork matmult2exec matmult2fork testfiles testprint testexit testexithelper airportsim airstart clientsim 
	rm passenger
	#client1 client2 client3 client4 client5 
	#rm passenger screeningofficer checkinstaff liaison manager securityinspector cargohandler

	rm *~

rowhammer:	find_targets.o	rowhammer.o
	gcc -O2 -o rowhammer find_targets.o rowhammer.o

find_targets.o:	find_targets.c
	gcc -O2 -c -o find_targets.o find_targets.c

rowhammer.o:	rowhammer.c
	gcc -O2 -c -o rowhammer.o rowhammer.c

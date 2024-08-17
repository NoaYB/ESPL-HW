all: AntiVirus 

AntiVirus: AntiVirus.o
	gcc -m32 -o AntiVirus AntiVirus.o

AntiVirus.o: AntiVirus.c
	gcc -m32 -Wall -c AntiVirus.c

clean:
	rm -f *.o AntiVirus 

.PHONY: all clean
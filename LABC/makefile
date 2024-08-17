

myshell: LineParser.o myshell.o looper
	gcc -m32 -g -Wall -o myshell LineParser.o myshell.o  

LineParser.o: LineParser.c
	gcc -g -Wall -m32 -c -o LineParser.o LineParser.c

myshell.o: myshell.c
	gcc -g -Wall -m32 -c -o myshell.o myshell.c

looper: looper.o
	gcc -m32 -g -Wall -o looper looper.o

looper.o: looper.c
	gcc -g -Wall -m32 -c -o looper.o looper.c
 


.PHONY: clean 

clean:
	rm -f *.o lab
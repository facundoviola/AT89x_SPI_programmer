programer: programmer.c readhex.h pagelist.h readhex.o pagelist.o spi.o
	gcc -o ksu8051p $^

spi.o: spi.c spi.h
	gcc -c -o spi.o spi.c

readhex.o: readhex.c readhex.h
	gcc -c -o readhex.o readhex.c

pagelist.o: pagelist.c pagelist.h
	gcc -c -o pagelist.o pagelist.c

clean:
	rm ksu8051p pagelist.o readhex.o spi.o

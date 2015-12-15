CC=gcc
CFLAGS=

all: clean main

clean:
	rm -rf *.o

main: main.o parser.o analyzer.o 
	$(CC) $(CFLAGS) main.o parser.o analyzer.o -o prog 

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c

analyzer.o: analyzer.c
	$(CC) $(CFLAGS) -c analyzer.c

test:
	./prog tests/parser-input 2> /dev/null > tests/parser-result
	./prog tests/analyzer-input &> tests/analyzer-result
	diff tests/parser-expected tests/parser-result
	diff tests/analyzer-expected tests/analyzer-result

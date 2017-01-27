# Dear heavenly father we pray for our eternal soul.
# We have made a makefile and are sinful.

CFLAGS=-std=c++11 -O2 -fopenmp
OBJ = board.o book.o
DEPS = board.h polyglot.h

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)

server: $(OBJ) server.cpp
	g++ -o betachess-server $(CFLAGS) $(OBJ) server.cpp -levent

tests: $(OBJ) test.cpp
	g++ -o betachess-tests $(CFLAGS) $(OBJ) test.cpp

gen-book: $(OBJ)
	g++ -o betachess-gen-book $(CFLAGS) $(OBJ) genBook.cpp

clean:
	rm -f betachess-* *.o *.gch

run-server: server
	./betachess-server

run-test: tests
	./betachess-tests

run-gen-book: gen-book
	./betachess-gen-book

#time g++ -std=c++14 -fopenmp -O2 -o betachess board.h board.cpp main.cpp
#  time ./betachess-server
#  time ./betachess

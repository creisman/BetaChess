# Dear heavenly father we pray for our eternal soul.
# We have made a makefile and are sinful.

CFLAGS=-std=c++11 -fopenmp -O2
OBJ = board.o book.o
DEPS = board.h polyglot.h

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)

server: $(OBJ) server.cpp
	g++ -o betachess-server $(CFLAGS) $(OBJ) server.cpp -levent

tests: $(OBJ) tests.cpp
	g++ -o betachess-tests $(CFLAGS) $(OBJ) tests.cpp

eval-tests: $(OBJ) evalTests.cpp
	g++ -o betachess-eval-tests $(CFLAGS) $(OBJ) evalTests.cpp

gen-book: $(OBJ) genBook.cpp
	g++ -o betachess-gen-book $(CFLAGS) $(OBJ) genBook.cpp

clean:
	rm -f betachess-* *.o *.gch

run-server: server
	./betachess-server

run-test: tests
	./betachess-tests

run-eval-test: eval-tests
	./betachess-eval-tests

run-gen-book: gen-book
	./betachess-gen-book

#time g++ -std=c++14 -fopenmp -O2 -o betachess board.h board.cpp main.cpp
#  time ./betachess-server
#  time ./betachess

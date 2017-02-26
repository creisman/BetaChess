# Dear heavenly father we pray for our eternal soul.
# We have made a makefile and are sinful.

CFLAGS=-std=c++11 -fopenmp -O2
SRC = board.cpp book.cpp flags.cpp
OBJ = ${SRC:.cpp=.o}
LIBS = -lgflags
SERVER_LIBS = -levent

%.o: %.cpp flags.h flags.cpp
	g++ -c -o $@ $< $(CFLAGS)

server: $(OBJ) server.cpp
	g++ -o betachess-server server.cpp $(OBJ) $(CFLAGS) $(LIBS) $(SERVER_LIBS)

tests: $(OBJ) tests.cpp
	g++ -o betachess-tests tests.cpp $(OBJ) $(CFLAGS) $(LIBS)

eval-tests: $(OBJ) evalTests.cpp
	g++ -o betachess-eval-tests evalTests.cpp $(OBJ) $(CFLAGS) $(LIBS)

gen-book: $(OBJ) genBook.cpp
	g++ -o betachess-gen-book genBook.cpp $(OBJ) $(CFLAGS) $(LIBS)

clean:
	rm -f betachess-* *.o *.gch

run-server: server
	./betachess-server

run-tests: tests
	./betachess-tests

run-eval-test: eval-tests
	./betachess-eval-tests

run-gen-book: gen-book
	./betachess-gen-book

#time g++ -std=c++14 -fopenmp -O2 -o betachess board.h board.cpp main.cpp
#  time ./betachess-server
#  time ./betachess

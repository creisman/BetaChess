# Dear heavenly father we pray for our eternal soul.
# We have made a makefile and are sinful.

CFLAGS=-std=c++11 -O2 -fopenmp
OBJ = board.o book.o
DEPS = board.h

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)

server: $(OBJ) server.cpp
	g++ -o betachess-server $(CFLAGS) -levent $(OBJ) server.cpp

betachess: $(OBJ) main.cpp
	g++ -o betachess $(CFLAGS) $(OBJ) main.cpp

clean:
	rm -f betachess betachess-server *.o *.gch

run-server: server
	./betachess-server

#time g++ -std=c++14 -fopenmp -O2 -o betachess board.h board.cpp main.cpp
#  time ./betachess-server
#  time ./betachess

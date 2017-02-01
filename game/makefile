# Dear heavenly father we pray for our eternal soul.
# We have made a makefile and are sinful.

CFLAGS=-std=c++11 -O2 -fopenmp -I/home/pi/tbbsrc/include/ -Wl,-rpath,/home/pi/tbbsrc/build/linux_armv7_gcc_cc4.9.2_libc2.19_kernel4.1.19_release/ -ltbb
OBJ = board.o
DEPS = board.h polyglot.h

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)

tests: $(OBJ) tests.cpp
	g++ -o betachess-tests $(CFLAGS) $(OBJ) tests.cpp

clean:
	rm -f betachess-* *.o *.gch

run-test: tests
	./betachess-tests

#time g++ -std=c++14 -fopenmp -O2 -o betachess board.h board.cpp main.cpp
#  time ./betachess-server
#  time ./betachess

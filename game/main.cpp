#include <iostream>

#include "board.h"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  board::Board b(true /* init */);
  int ply = 4;

  int count = 0;
  int captures = 0;
  int mates = 0;
  b.perft(ply, &count, &captures, &mates);

  cout << "Perft results for" <<
          " depth (ply): " << ply << endl;
  cout << "\tcount: " << count << 
          "\tcaptures: " << captures << 
          "\tmates: " << mates << endl;

  return 0;
}

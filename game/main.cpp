#include <atomic>
#include <iostream>

#include "board.h"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  board::Board b(true /* init */);
  cout << "Size of Board: " << sizeof(b) << endl;

  int ply = 6;

  atomic<int> count(0);
  atomic<int> captures(0);
  atomic<int> mates(0);
  b.perft(ply, &count, &captures, &mates);

  cout << "Perft results for" <<
          " depth (ply): " << ply << endl;
  cout << "\tcount: " << count << 
          "\tcaptures: " << captures << 
          "\tmates: " << mates << endl;
  cout << endl << endl;

  b.printBoard();
  return 0;
}

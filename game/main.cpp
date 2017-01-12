#include <atomic>
#include <iostream>

#include "board.h"

using namespace std;

void perft(string fen) {
  board::Board b(true /* init */);
  if (!fen.empty()) {
    b = board::Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  b.printBoard();
  cout << "Size of Board class: " << sizeof(b) << endl;

  int ply = 3;

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
}


int main(int argc, char *argv[]) {
  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  //perft("");

  // "Position 2 - Kiwipete"
  perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w kQkq -");

  return 0;
}

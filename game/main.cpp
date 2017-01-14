#include <atomic>
#include <iostream>

#include "board.h"

using namespace std;

void perft(int ply, string fen) {
  board::Board b(true /* init */);
  if (!fen.empty()) {
    b = board::Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  b.printBoard();
  cout << "Size of Board class: " << sizeof(b) << endl;

  atomic<int> count(0);
  atomic<int> captures(0);
  atomic<int> ep(0);
  atomic<int> castles(0);
  atomic<int> mates(0);
  b.perft(ply, &count, &captures, &ep, &castles, &mates);

  cout << "Perft results for" <<
          " depth (ply): " << ply << endl;
  cout << "\tcount: " << count << 
          "\tcaptures: " << captures << 
          "\tEn Passant: " << ep << 
          "\tcastles: " << castles << 
          "\tmates: " << mates << endl;
  cout << endl << endl;
}


int main(int argc, char *argv[]) {
  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  //perft("");

  // "Position 2 - Kiwipete"
  perft(5, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  // "Position 3"
  //perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

  return 0;
}

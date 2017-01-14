#include <atomic>
#include <iostream>

#include "board.h"

using namespace std;
using namespace board;

void perft(int ply, string fen) {
  Board b(true /* init */);
  if (!fen.empty()) {
    b = Board(fen);
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


void playGame(int ply, string fen) {
  Board b(true /* init */);
  if (!fen.empty()) {
    b = Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  b.printBoard();

  for (int t = 0; t < 50; t++) {
    auto scoredMove = b.findMove(ply);
    move_t m = scoredMove.second;

    cout << "iter: " << t << "\tScore: " << scoredMove.first << endl;
    cout << Board::moveNotation(m) <<
         "\t("   << (int)get<0>(m) << ", " << (int)get<1>(m) <<
         " to " << (int)get<2>(m) << ", " << (int)get<3>(m) <<
         "\tpiece: " <<(int) get<4>(m) << "\ttakes: " << (int)get<5>(m) << endl;

    // signals win.
    if (get<4>(m) == 0) {
      break;
    }

    // Make the move.
    b.makeMove(get<0>(m), get<1>(m), get<2>(m), get<3>(m));
    b.printBoard();
  }
}


int main(int argc, char *argv[]) {
  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  playGame(10, "");
  //playGame(10, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");


  // Initial Position
  // rPi seems to generate ~1.1M nodes / second with pragma on.
  //perft(3, "");

  // "Position 2 - Kiwipete"
  // rPi seems to generate ~1.5M nodes / second with pragma on.
  //perft(6, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  // "Position 3"
  //perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

  

  return 0;
}

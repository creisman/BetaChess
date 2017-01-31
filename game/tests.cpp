#include <atomic>
#include <chrono>
#include <cstdio>
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

  auto T0 = chrono::system_clock().now();

  atomic<int> count(0);
  atomic<int> captures(0);
  atomic<int> ep(0);
  atomic<int> castles(0);
  atomic<int> promotions(0);
  atomic<int> mates(0);
  b.perft(ply, &count, &captures, &ep, &castles, &promotions, &mates);

  auto T1 = chrono::system_clock().now();
  chrono::duration<double> duration = T1 - T0;
  double duration_s = duration.count();

  cout << "Perft results for" <<
          " depth (ply): " << ply << endl;
  cout << "\tcount: " << count <<
          "\tcaptures: " << captures <<
          "\ten passant: " << ep <<
          "\tcastles: " << castles <<
          "\tpromotions: " << promotions <<
          "\tmates: " << mates << endl;
  printf("\tevaled: %.0f knodes/s (%.2f seconds)\n", (count / duration_s / 1000), duration_s);
  cout << endl << endl;
}


void playGame(int moves, int ply, string fen) {
  Board b(true /* init */);
  if (!fen.empty()) {
    b = Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  auto T0 = chrono::system_clock().now();

  for (int move = 0; move < moves; move++) {
    auto scoredMove = b.findMove(ply);
    move_t m = scoredMove.second;
    cout << "iter: " << ply << "\tScore: " << scoredMove.first << endl;
    b.makeMove(m);
    //b.printBoard();
  }

  auto T1 = chrono::system_clock().now();
  chrono::duration<double> duration = T1 - T0;
  double duration_s = duration.count();
  printf("evaled: %d moves (%.2f seconds)\n\n", moves, duration_s);
}


int main(int argc, char *argv[]) {
  bool testPlay = false;
  bool testPerft = false;
  bool testHash = true;

  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  cout << "Size of Board class: " << sizeof(Board) << endl << endl;

  if (testPlay) {
    playGame(4, 5, "");
    //playGame(10, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  }

  if (testPerft) {
    // Initial Position
    // rPi seems to generate ~1.1M nodes / second with pragma on.
    perft(5, "");

    // "Position 2 - Kiwipete"
    // rPi seems to generate ~1.5M nodes / second with pragma on.
    // perf 5 had a miss by a couple thousand.
    perft(4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

    // "Position 3"
    perft(6, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

    // "Position 4"
    perft(5, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -");

    // From "perf(7) challange" - 14,794,751,816
    // perft(7, "rnb1kbnr/pp1pp1pp/1qp2p2/8/Q1P5/N7/PP1PPPPP/1RB1KBNR b Kkq - 2 4");
  }


  if (testHash) {
    Board b(true /* init */);
    cout << hex << b.getZobrist() << " " << (b.getZobrist() == 0x463b96181691fc9c) << dec << endl;

    string fen = "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3";
    b = Board(fen);
    cout << hex << b.getZobrist() << " " << (b.getZobrist() == 0x652a607ca3f242c1) << dec << endl;

    // TODO make moves, then verify fen and zobrist a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3
    fen = "rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4";
    b = Board(fen);
    cout << hex << b.getZobrist() << " " << (b.getZobrist() == 0x5c3f9b829b279560) << dec << endl;
    cout << fen.size() << ": \"" << fen << "\"" << endl;
    string gen = b.generateFen_slow();
    cout << gen.size() << ": \"" << gen << "\" matches: " << (gen == fen) << endl;
  }

  return 0;
}

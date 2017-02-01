#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "board.h"

using namespace std;
using namespace board;

bool verifySeriesOfMoves(
    string stringOfMoves,
    string fen,
    uint64_t zobrist) {
  Board b(true /* init */);
  
  stringstream ss(stringOfMoves);
  istream_iterator<string> begin(ss);
  istream_iterator<string> end;
  vector<string> moves(begin, end);

  for (string move : moves) {
  //  cout << move << endl;
    //for (Board c : b.getLegalChildren()) {
    //  cout << "\t" << b.algebraicNotation_slow(c.getLastMove()) << endl;
    //}
    assert( b.makeAlgebraicMove_slow(move) );
  //  cout << "after " << move << "\t| " << b.generateFen_slow() << endl;
  }

  string gen = b.generateFen_slow();
  cout << "          " << gen.size() << ": \"" << gen << "\" matches: " << (gen == fen) << endl;
  if (gen != fen) {
    cout << "expected: " << fen.size() << ": \"" << fen << "\"" << endl;
  };

  uint64_t genZ = b.getZobrist();

  cout << "          " << hex << genZ << " matches: " << (genZ == zobrist) << dec << endl;
  if (genZ != zobrist) {
    cout << "expected: " << hex << zobrist << dec << endl;
  }

  cout << endl;
  return (gen == fen) && (genZ == zobrist);
}

void perft(int ply, bool simple, string fen) {
  Board b(true /* init */);
  if (!fen.empty()) {
    b = Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  b.printBoard();

  auto T0 = chrono::system_clock().now();

  atomic<long> count(0);
  atomic<long> captures(0);
  atomic<int> ep(0);
  atomic<int> castles(0);
  atomic<int> promotions(0);
  atomic<int> mates(0);

  if (simple) {
    count += b.perftMoveOnly(ply);
  } else {
    b.perft(ply, &count, &captures, &ep, &castles, &promotions, &mates);
  }

  auto T1 = chrono::system_clock().now();
  chrono::duration<double> duration = T1 - T0;
  double duration_s = duration.count();

  cout << "Perft results for" <<
          " depth (ply): " << ply << endl;
  cout << "\tcount: " << count;

  if (simple) {
    cout << endl;
  } else {
    cout<<"\tcaptures: " << captures <<
          "\ten passant: " << ep <<
          "\tcastles: " << castles <<
          "\tpromotions: " << promotions <<
          "\tmates: " << mates << endl;
  }

  printf("\tevaled: %.0f knodes/s (%.2f seconds)\n",
      count/ duration_s / 1000.0, duration_s);
  cout << endl << endl;
}


int main(int argc, char *argv[]) {
  bool testPerft = true;
  bool testHash = false;

  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  cout << "Size of Board class: " << sizeof(Board) << endl << endl;

  if (testPerft) {
    // Initial Position
    // rPi seems to generate ~1.1M nodes / second with pragma on.
    perft(5, false, "");
    perft(5, true, "");

    // "Position 2 - Kiwipete"
    // rPi seems to generate ~1.5M nodes / second with pragma on.
    //perft(4, false, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");

    // "Position 3"
    //perft(6, false, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0");

    // "Position 4"
    //perft(5, false, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 0");

    // From "perf(7) challange" - 14,794,751,816
    // perft(7, "rnb1kbnr/pp1pp1pp/1qp2p2/8/Q1P5/N7/PP1PPPPP/1RB1KBNR b Kkq - 2 4");
  }


  if (testHash) {
    Board b(true /* init */);
    assert (b.getZobrist() == 0x463b96181691fc9c);

    b = Board("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3");
    assert (b.getZobrist() == 0x652a607ca3f242c1);

    // Make moves verify fen and zobrist (after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3)
    assert (verifySeriesOfMoves(
        "a4 b5  h4 b4  c4 bxc3  Ra3",
        "rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 1 4",
        0x5c3f9b829b279560));

    // Verify BLACK_OOO, en_passant, WHITE_OO, and underpromotion (to check).
    // TODO find some way to verify this zobrist.
    assert (verifySeriesOfMoves(
        "a4   b5      axb5 c5      bxc6  Bb7     Nf3 Na6   e3    Qa5  "
        "Bxa6 O-O-O   O-O  Qxa6    cxb7+ Kc7     Ra5 Kc6   b8=N+ Rxb8 "
        "Qe2  Qxe2    Ng5  Qxf2+   Kh1   Qxf1#",
        "1r3bnr/p2ppppp/2k5/R5N1/8/4P3/1PPP2PP/1NB2q1K w - - 0 14",
        0x7aa9d458c3cd289f));
  }

  return 0;
}

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "board.h"
#include "polyglot.h"

using namespace std;
using namespace board;

bool verifySeriesOfMoves(
    string stringOfMoves,
    string fen,
    board_hash_t zobrist) {
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
    bool valid = b.makeAlgebraicMove_slow(move);
    if (!valid) {
      cout << "Move: " << move << endl;
      assert( false ); // unknown move.
    }
  //  cout << "after " << move << "\t| " << b.generateFen_slow() << endl;
  }

  string gen = b.generateFen_slow();
  cout << "          " << gen.size() << ": \"" << gen << "\" matches: " << (gen == fen) << endl;
  if (gen != fen) {
    cout << "expected: " << fen.size() << ": \"" << fen << "\"" << endl;
  };

  board_hash_t genZ = b.getZobrist();

  cout << "          " << hex << genZ << " matches: " << (genZ == zobrist) << dec << endl;
  if (genZ != zobrist) {
    cout << "expected: " << hex << zobrist << dec << endl;
  }

  cout << endl;
  return (gen == fen) && (genZ == zobrist);
}

void perft(int ply, map<int, long> countToVerify, string fen) {
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

  long expected = countToVerify[ply];
  if (expected > 0 && expected != count) {
    cout << "Count mismatch " << count << " != " << expected << " (expected)" << endl;
    assert( false );
  }

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
  bool testPlay = true;
  bool testPerft = true;
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
    perft(5,
          {{1, 20},
           {2, 400},
           {3, 8902},
           {4, 197281},
           {5, 4865609},
           {6, 119060324},
           {7, 3195901860}},
          "");

    // "Position 2 - Kiwipete"
    // rPi seems to generate ~1.5M nodes / second with pragma on.
    // perf 5 had a miss by a couple thousand.
    perft(4,
          {{4, 4085603}, {5, 193690690}},
          "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");

    // "Position 3"
    perft(6,
          {{5, 674624}, {6, 11030083}, {7, 178633661}},
          "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0");

    // "Position 4"
    perft(5,
          {{4, 422333}, {5, 15833292}},
          "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 0");

    // From "a perf(7) challange"
    perft(5,
          {{5, 17251342}, {6, 490103130}, {7, 14794751816}},
          "rnb1kbnr/pp1pp1pp/1qp2p2/8/Q1P5/N7/PP1PPPPP/1RB1KBNR b Kkq - 2 4");
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


    // En Passant verification.
    assert (verifySeriesOfMoves(
        "a4 h6   a5 b5",
        "rnbqkbnr/p1ppppp1/7p/Pp6/8/8/1PPPPPPP/RNBQKBNR w KQkq - 0 3",
        0xde68558cff2df99c));

    // Same board position but with no En Passant.
    assert (verifySeriesOfMoves(
        "a4 h6   a5 b5   Nf3 Nf6   Ng1 Ng8",
        "rnbqkbnr/p1ppppp1/7p/Pp6/8/8/1PPPPPPP/RNBQKBNR w KQkq - 4 5",
        0xde68558cff2df99c ^ POLYGLOT_RANDOM[772 + 1]));
  }

  return 0;
}

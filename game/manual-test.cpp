#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include "board.h"
#include "polyglot.h"
#include "search.h"
#include "flags.h"

using namespace std;
using namespace board;
using namespace search;

void playGame(string fen) {
  Board b;
  if (!fen.empty()) {
    b = Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  auto T0 = chrono::system_clock().now();

  Search s(b, false /* useTimeControl */);
  FindMoveStats stats;
  scored_move_t suggest = s.findMove(
      4,      // Min Ply
      250000,  // Min Nodes
      &stats);
  double score = get<0>(suggest);
  move_t move = get<1>(suggest);
  string alg = s.getRoot().algebraicNotation_slow(move);

  cout << "\tsuggested Move: " << alg
       << " score: " << score / 100.00
       << " (searched " << stats.nodes << " nodes)" << endl;

  s.makeMove(move);

  auto T1 = chrono::system_clock().now();
  chrono::duration<double> duration = T1 - T0;
  double duration_s = duration.count();
  printf("\t(%.2f seconds)\n\n", duration_s);
}


int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  //playGame(4,  120000, "");
  playGame("2kr4/1pp5/p4K1p/8/8/P1P5/1Pb5/3q4 b - - 4 55");


  return 0;
}

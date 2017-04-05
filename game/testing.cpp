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
#include "flags.h"
#include "polyglot.h"
#include "search.h"

using namespace std;
using namespace board;
using namespace search;

void playGame(int numMoves, int nodes, string fen) {
  Board b;
  if (!fen.empty()) {
    b = Board(fen);
    cout << "Loaded from fen: \"" << fen << "\"" << endl;
  }

  auto T0 = chrono::system_clock().now();

  for (int iter = 0; iter < numMoves; iter++) {
    FindMoveStats stats;
    Search search(b);
    scored_move_t suggest = search.findMove(3, nodes, &stats);
    double score = get<0>(suggest);
    move_t move = get<1>(suggest);
    string alg = b.algebraicNotation_slow(move);

    cout << "iter: " << iter << "\tsuggested Move: " << alg
         << " score: " << score / 100.00
         << " (searched " << stats.nodes << " nodes)" << endl;
    b.makeMove(move);
    //b.printBoard();
  }

  auto T1 = chrono::system_clock().now();
  chrono::duration<double> duration = T1 - T0;
  double duration_s = duration.count();
  printf("evaled: %d moves (%.2f seconds)\n\n", numMoves, duration_s);
}


int main(int argc, char *argv[]) {
  playGame(4,  120000, "");
  playGame(10, 200000, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");

  return 0;
}

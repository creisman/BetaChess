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
  atomic<int> promotions(0);
  atomic<int> mates(0);
  b.perft(ply, &count, &captures, &ep, &castles, &promotions, &mates);

  cout << "Perft results for" <<
          " depth (ply): " << ply << endl;
  cout << "\tcount: " << count << 
          "\tcaptures: " << captures << 
          "\ten passant: " << ep <<
          "\tcastles: " << castles <<
          "\tpromotions: " << promotions <<
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


  bool weAreWhite;
  cout << "[W]hite or [B]lack?" << endl;
  {
    string test;
    cin >> test;
    weAreWhite = (test == "w" || test == "W" ||
                  test == "White" || test == "white");
  }

  cout << "We are playing white: " << weAreWhite << endl;

  // white start with c4.
  if (weAreWhite) {
    b.makeMove(1, 2, 3, 2);
    b.printBoard();
  }

  for (int halfCount = weAreWhite ? 1 : 0; halfCount < 50; halfCount++) {
    move_t m;

    // Make the move.
    if (weAreWhite == (halfCount % 2 == 0)) {
      auto scoredMove = b.findMove(ply);
      m = scoredMove.second;
      cout << "iter: " << halfCount << "\tScore: " << scoredMove.first << endl;
    } else {
      // read move from input.
      string oldS, newS;
      while (true) {
        cout << "What move did they play?" << endl;
        cin >> oldS;
        cin >> newS;

        cout << "confirm [" << oldS << "] to [" << newS << "]" <<endl;
        string confirm;
        cin >> confirm; 
        if (confirm == "y" || confirm == "t") {
          break;
        }
      }
      m = make_tuple(oldS[1] - '1', oldS[0] - 'a',
                     newS[1] - '1', newS[0] - 'a',
                     1 /* moving */, 0 /* removing */);
    }

    cout << Board::moveNotation(m) <<
         "\t("   << (int)get<0>(m) << ", " << (int)get<1>(m) <<
         " to " << (int)get<2>(m) << ", " << (int)get<3>(m) <<
         "\tpiece: " <<(int) get<4>(m) << "\ttakes: " << (int)get<5>(m) << endl;

    // signals win.
    if (get<4>(m) == 0) {
      break;
    }


    b.makeMove(get<0>(m), get<1>(m), get<2>(m), get<3>(m));
    b.printBoard();
  }
}


int main(int argc, char *argv[]) {
  if (argc > 1){
    cout << "Called with " << argc << " args" << endl;
  }

  //playGame(8, "");
  //playGame(10, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");


  // Initial Position
  // rPi seems to generate ~1.1M nodes / second with pragma on.
  //perft(6, "");

  // "Position 2 - Kiwipete"
  // rPi seems to generate ~1.5M nodes / second with pragma on.
  // perf 5 had a miss by a couple thousand.
  perft(5, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  // "Position 3"
  //perft(6, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

  return 0;
}

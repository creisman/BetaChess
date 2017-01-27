#include <cassert>
#include <iostream>
#include <cstring>
#include <string>
#include <random>

#include "board.h"
#include "book.h"

using namespace std;
using namespace board;
using namespace book;

// Some globals to make life easy.
minstd_rand generator;

Book bookT;
int gameNum = 0;
Board *boardT = nullptr;
vector<move_t> moves;

// Read-Evaluate-Play loop.
void repLoop(void) {
  // Some Book stuff here.
  scored_move_t suggest;

  move_t *bookMove = bookT.multiArmBandit(moves);
  bool foundBookResponse = false;
  if (bookMove != nullptr) {
    cout << "\tGot move from book" << endl;
    suggest = make_pair(NAN, *bookMove);

    // Verify it's a real move
    for (Board c : boardT->getLegalChildren()) {
      if (*bookMove == c.getLastMove()) {
        foundBookResponse = true;
        break;
      }
    }
    if (!foundBookResponse) {
      cerr << "ERROR: Got bad suggestion from book" << endl;
    }
  }

  if (!foundBookResponse) {
    // TODO parameters
    int nodes = 100000;
    nodes += generator() % 300000;

    if (moves.size() < 4) {
      nodes += generator() % 2000000;
    }

    suggest = boardT->findMove(nodes);
  }

  double score = get<0>(suggest);
  move_t move = get<1>(suggest);
  string alg = boardT->algebraicNotation(move);

  cout << "\t" << gameNum << ", " << moves.size() << ": " << alg << "\t\tscore: " << score << endl;

  boardT->makeMove(move);

  if (boardT->getLastMove() != move) {
    cout << (int) get<0>(move) << ", " << (int) get<1>(move) << "  "
         << (int) get<2>(move) << ", " << (int) get<3>(move) << "  w "
         << (int) get<4>(move) << " s " << (int) get<5>(move) << " sp: "
         << (int) get<6>(move) << endl;

    move_t amove = boardT->getLastMove();
    cout << (int) get<0>(amove) << ", " << (int) get<1>(amove) << "  "
         << (int) get<2>(amove) << ", " << (int) get<3>(amove) << "  w "
         << (int) get<4>(amove) << " s " << (int) get<5>(amove) << " sp: "
         << (int) get<6>(amove) << endl;
  }

  assert(boardT->getLastMove() == move);
  moves.push_back(move);
}

void playGame(void) {
  gameNum += 1;
  cout << "Playing a new game board (" << gameNum << ")" << endl;
  if (boardT != nullptr) {
    free(boardT);
  }
  boardT = new Board(true /* init */);
  moves.clear();

  while (!boardT->getGameResult() != Board::RESULT_IN_PROGRESS) {
    repLoop();

    // TODO hack for fifty move rule.
    if (moves.size() > 100) {
      return;
    }
  }

  cout << "Game ended with result " << (int) boardT->getGameResult() << endl;
  bookT.updateResult(moves, boardT->getGameResult());
  bookT.write();
}


int main(void) {
  cout << endl << "Starting book gen, theory time" << endl << endl;

  Book bookT;
  bookT.load();

  // Do some serious thery
  for (int i = 0; i < 1000; i++) {
    // TODO add timing.
    playGame();
  }

  return 0;
}

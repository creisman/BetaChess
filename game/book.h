#ifndef BOOK_H
#define BOOK_H

#include <cstring>
#include <random>
#include <tuple>
#include <vector>

#include "board.h"

using namespace std;
using namespace board;

namespace book {
  // TODO book isn't smart enough to allow transpositions (e4, e5, d4, d5 = d4, d5, e4, e5).
  struct BetaChessBookEntry {
    move_t move;
    short played;
    short wins; // Measured by white.
    short losses; // Measured by white.

    vector<BetaChessBookEntry*> children;
  };

  // Book Class
  class Book {
    public:
      static const size_t MAX_DEPTH;
      static const string ANTICHESS_FILE;

      Book();

      bool load(void);
      bool write(void);

      move_t* multiArmBandit(vector<move_t> moves);

      // see Board.h getGameResult() for player agnostic result.
      bool updateResult(vector<move_t> moves, board_s result);

      void printBook(void);

    private:
      // class variables
      int positionsLoaded = 0;
      BetaChessBookEntry root;
      mt19937 randomGenerator;

      // private methods
      BetaChessBookEntry* recurseMove(BetaChessBookEntry *entry, move_t move);
      BetaChessBookEntry* recurseMoves(BetaChessBookEntry *entry, vector<move_t> moves);
      void printBook(BetaChessBookEntry *entry, int depth, int recurse);

      // helper printer method.
      string stringRecord(BetaChessBookEntry *entry);
      string stringMove(move_t move);

  };
}
#endif // BOOK_H

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
    board_hash_t hash;
    short played;
    short wins; // Measured by white.
    short losses; // Measured by white.
  };

  // Book Class
  class Book {
    public:
      static const size_t MAX_DEPTH;
      static const string ANTICHESS_FILE;

      Book();

      bool load(void);
      bool write(void);

      string multiArmBandit(Board &b);

      // see Board.h getGameResult() for player agnostic result.
      bool updateResult(vector<string> moves, board_s result);

      void printBook(void);

    private:
      // class variables
      map<board_hash_t, BetaChessBookEntry*> bookMap;
      mt19937 randomGenerator;

      // helper printer method.
      void printBook(Board &b, string moveName, int depth, int recurse);
      BetaChessBookEntry* findOrCreateEntry(board_hash_t hash);
      void updateEntry(BetaChessBookEntry *entry, board_s result);

      string stringRecord(BetaChessBookEntry *entry);
      string stringMove(move_t move);
  };
}
#endif // BOOK_H

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
      static const string SAVE_FILE_PREFIX;

      Book();

      bool load(void);
      bool write(void);

      string multiArmBandit(Board &b);

      // see Board.h getGameResult() for player agnostic result.
      bool updateResult(vector<string> moves, board_s result);

      void printBook(void);

      // These helper methods just ended up here organically.
      static int saveMoves(vector<string> moves);
      static void loadMoves(int number, vector<string> *moves);
    private:
      // Book is great because it allows for transpositions (but not path).
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

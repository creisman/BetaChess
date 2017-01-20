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
    move_t move;
    short played;
    short wins;
    short losses;

    vector<BetaChessBookEntry*> children;
  };

  // Book Class
  class Book {
    public:
      static const string ANTICHESS_FILE;

      Book();

      bool load(void);
      bool write(void);

      move_t* multiArmBandit(vector<move_t> moves);

      bool incrementPlayed(vector<move_t> moves);
    
      // TODO
      //bool updateResult(vector<move_t> moves, bool result);

      void printBook(void);

    private:
      // class variables
      int positionsLoaded = 0;
      BetaChessBookEntry root;
      mt19937 randomGenerator;

      // private methods
      BetaChessBookEntry* recurse(vector<move_t> moves);
      void printBook(BetaChessBookEntry *entry, int depth, int recurse);

      // helper printer method.
      string stringRecord(BetaChessBookEntry *entry);
      string stringMove(move_t move);

  };
}
#endif // BOOK_H

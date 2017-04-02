#ifndef TTABLE_H
#define TTABLE_H

#include <unordered_map>

#include "board.h"

using namespace std;
using namespace board;

namespace ttable {
  const char LOWER_BOUND = 1;
  const char UPPER_BOUND = 2;
  const char EXACT_BOUND = 3;

  // TODO book isn't smart enough to allow transpositions (e4, e5, d4, d5 = d4, d5, e4, e5).
  struct TTableEntry {
    char type;
    char depth;
    int score;
    move_t suggested;
  };

  void clearTT(void);
  int sizeTT(void);

  void storeTT(board_hash_t position, TTableEntry* entry);
  TTableEntry* lookupTT(board_hash_t position);

  void clearHistory(void);
  void updateHistory(bool isWhite, int from, int to, int delta);
  int lookupHistory(bool isWhite, int from, int to);
}

#endif // TTABLE_H

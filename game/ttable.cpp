#include <cassert>
#include <unordered_map>

#include "ttable.h"

using namespace std;
using namespace board;

namespace ttable {
  unordered_map<board_hash_t, TTableEntry* > globalTT;
  int globalHistory[2][64][64] = {};

  void clearTT() {
    globalTT.clear();
  }

  int sizeTT() {
    return globalTT.size();
  }

  void storeTT(board_hash_t position, TTableEntry* entry) {
    globalTT[position] = entry;
  }

  TTableEntry* lookupTT(board_hash_t position) {
    auto test = globalTT.find(position);
    return (test != globalTT.end()) ?
      test->second : nullptr;
  }

  void clearHistory() {
    for (int color = 0; color < 2; color++) {
      for (int from = 0; from < 64; from++) {
        for (int to = 0; to < 64; to++) {
          globalHistory[color][from][to] = 0;
        }
      }
    }
  }

  void updateHistory(bool isWhite, int from, int to, int delta) {
    globalHistory[isWhite][from][to] += delta;
  }

  int lookupHistory(bool isWhite, int from, int to) {
    return globalHistory[isWhite][from][to];
  }
}

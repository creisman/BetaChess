#include <cassert>
#include <unordered_map>

#include "ttable.h"

using namespace std;
using namespace board;

namespace ttable {
  unordered_map<board_hash_t, TTableEntry* > globalTT;

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
}

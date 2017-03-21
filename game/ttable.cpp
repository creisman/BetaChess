#include <cassert>
#include <unordered_map>

#include "ttable.h"

using namespace std;
using namespace board;

namespace ttable {
  unordered_map<board_hash_t, TTableEntry* > global_tt;

  void clear_tt() {
    global_tt.clear();
  }

  int size_tt() {
    return global_tt.size();
  }

  void store_tt(board_hash_t position, TTableEntry* entry) {
    global_tt[position] = entry;
  }

  TTableEntry* lookup_tt(board_hash_t position) {
    auto test = global_tt.find(position);
    return (test != global_tt.end()) ?
      test->second : nullptr;
  }
}

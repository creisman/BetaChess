#ifndef SEARCH_H
#define SERCH_H

#include <atomic>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include "flags.h"

using namespace std;
using namespace board;

namespace search {
  // score concatonated to end of move_t
  typedef pair<int, move_t> scored_move_t;

  // Search Class
  class Search {
    public:
      // Constructors
      Search();

      scored_move_t findMove(const Board& root, int minPly, int minNodes, FindMoveStats *info);

      static void perft(
          const Board&b,
          int ply,
          atomic<long> *count,
          atomic<long> *captures,
          atomic<int> *ep,
          atomic<int> *castles,
          atomic<int> *promotions,
          atomic<int> *mates);

    private:
      // Helper methods.
      static void orderChildren(vector<Board> &children);
      static int moveOrderingValue(const Board& b);
      static int getGameResultScore(board_s gameResult, int depth);

      // 1-arg version is public.
      scored_move_t findMoveHelper(const Board& b, char ply, int alpha, int beta);

      // Quiesce is a search at a leaf node which tries to avoid the horizon effect
      //   (if Queen just captured pawn make sure the queen can't be recaptured)
      static int quiesce(const Board& b, int alpha, int beta);
      static int evalCaptures(const Board& b, int alpha, int beta, int depth);

      // Variables
      // Used for counting moves in findMove
      static atomic<int> nodeCounter;
      static atomic<int> quiesceCounter;
      static atomic<int> ttCounter;

      int plySearchDepth;
  };
}
#endif // SEARCH_H

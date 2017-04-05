#include <algorithm>
#include <atomic>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "board.h"
#include "flags.h"
#include "search.h"
#include "ttable.h"
//Maybe needed in future
//#include "polyglot.h"
//#include "pst.h"

using namespace std;
using namespace board;
using namespace search;
using namespace ttable;


Search::Search(Board toSearch) {
 root = toSearch;
}


void Search::orderChildren(vector<Board> &children) {
  if (IS_ANTICHESS) {
    // TODO this is the simple version, could be partially moved into getChildren.
    return;
  }

  //auto comparitor = [](const Board&a, const Board&b) {
  //  return Search::moveOrderingValue(a) > Search::moveOrderingValue(b);
  //};
  //sort(children.begin(), children.end(), comparitor);
  //return;

  int n = children.size();

  pair<int, int> test[n];
  for (int i = 0; i < n; i++) {
    test[i] = make_pair(Search::moveOrderingValue(children[i]), i);
  }

  auto comparitor = [](const pair<int, int>&a, const pair<int, int>&b) { return a.first > b.first; };
  sort(test, test + n, comparitor);

  vector<Board> result;
  for (int i = 0; i < n; i++) {
    result.push_back(children[get<1>(test[i])]);
  }

  swap(children, result);
  return;
}


int Search::moveOrderingValue(const Board& b) {
  // 4. "Good" captures (taking higher value piece)
  // 3. Equal captures  (taking piece of ~equal~ value)
  // 2. Scary looking captures
  // 1. Quiet moves     (move with no capture)


  const int MAJOR_ORDERING = 1000000;

  move_t lastMove = b.getLastMove();
  board_s moving  = abs(get<4>(lastMove));
  board_s capture = abs(get<5>(lastMove));

  assert( moving != 0 );
  int movingValue = Board::getPieceValue(moving);

  int fromS = (get<0>(lastMove) << 3) + get<1>(lastMove);
  int toS = (get<2>(lastMove) << 3) + get<3>(lastMove);
  int historyHeuristic = lookupHistory(b.getIsWhiteTurn(), fromS, toS);
  // int historyHeuristic = 0;

  int captureScore = 0;
  if (capture != 0) {
    int captureValue = Board::getPieceValue(capture);
    if (captureValue > movingValue) {
      // Good captures
      captureScore = 4 * MAJOR_ORDERING + captureValue - movingValue;
    } else if (captureValue >= (movingValue - 50)) {
      // Equal Captures (including bishop for knight)
      captureScore = 3 * MAJOR_ORDERING + captureValue;
    } else {
      assert (captureValue < movingValue);
      // These moves might be good but they are scary to evaluate.
      captureScore = 2 * MAJOR_ORDERING + captureValue;
    }
  } else {
    // Quiet Move (sorted by how heavy a piece we are moving).
    captureScore = 1 * MAJOR_ORDERING + movingValue;
  }

  return captureScore + historyHeuristic;
}


// Public method that setups and calls helper method.
atomic<int> Search::nodeCounter(0);
atomic<int> Search::ttCounter(0);
atomic<int> Search::quiesceCounter(0);
scored_move_t Search::findMove(int minPly, int minNodes, FindMoveStats *stats) {
  Search::nodeCounter = 0;
  Search::ttCounter = 0;
  Search::quiesceCounter = 0;

  clearTT();
  clearHistory();

  if (stats) {
    stats->plyR = 0;
    stats->nodes = 0;
  }

  // Check if game has a result
  board_s result = root.getGameResult_slow();
  if (result != Board::RESULT_IN_PROGRESS) {
    int score = root.getGameResultScore(result);
    return make_pair(score, Board::NULL_MOVE);
  }

  // Check if we only have one move (if so no real choice).
  // Really useful for anti chess where this happens often.
  auto c = root.getLegalChildren();
  if (c.size() == 1) {
    return make_pair(NAN, c[0].getLastMove());
  }

  int plyR = max(2, minPly);

  // Checkmate this turn
  int maxScore = Board::SCORE_WIN + 100;

  scored_move_t scoredMove;
  int totalNodes = 0;
  while (true) {
    scoredMove = findMoveHelper(root, plyR, -maxScore, maxScore);
    totalNodes = nodeCounter + quiesceCounter;
    if (abs(scoredMove.first) >= Board::SCORE_WIN || totalNodes > minNodes) {
      break;
    }
    plyR += 1;
  }

  // scoredMove.first == NAN when it's a forced move, otherwise the in search window.
  assert (-maxScore <= scoredMove.first && scoredMove.first <= maxScore);

  string name = root.algebraicNotation_slow(scoredMove.second);
  string ttableDebug = !FLAGS_use_ttable ?
    "" : ("(tt " + to_string(sizeTT()) + ", " + to_string(Search::ttCounter) + ")");

  if (stats) {
    stats->plyR = plyR;
    stats->nodes = totalNodes;
  }

  if (FLAGS_verbosity >= 2) {
    cout << "\t\tplyR " << plyR << "=> "
         << nodeCounter << " + " << quiesceCounter << " nodes "
         << ttableDebug
         << " => " << name << " (@ " << scoredMove.first << ")" << endl;
  }

  // TODO add some code for PV.

  return scoredMove;
}


scored_move_t Search::findMoveHelper(const Board& b, char plyR, int alpha, int beta) {
  Search::nodeCounter += 1;

  if (FLAGS_use_ttable) {
    TTableEntry* lookup = lookupTT(b.getZobrist());
    if (lookup != nullptr) {
      if (lookup->depth >= plyR) {
        Search::ttCounter += 1;

        // TODO verify this is correct code cause I'm struggling at 3am.
        if (lookup->type == LOWER_BOUND) {
          alpha = max(alpha, lookup->score);

        } else if (lookup->type == UPPER_BOUND) {
          beta = max(beta, lookup->score);
        }

        // TODO it seems like this can return outside the bounds which is not allowed? (fail hard?)

        // If we have the old exact score or are out of the search window return move.
        if (alpha >= beta || lookup->type == EXACT_BOUND) {
          return make_pair(lookup->score, lookup->suggested);
        }
      }
    }
  }

  if (plyR == 0) {
    return make_pair(quiesce(b, alpha, beta), b.getLastMove());
  }

  vector<Board> children = b.getLegalChildren();
  if (children.empty()) {
    // Node is end of game!
    board_s status = b.getGameResult_slow();

    // TODO adjust this score for how many moves later it hapens.
    int score = b.getGameResultScore(status);

    // This might be possible if they load from FEN
    assert( b.getLastMove() != Board::NULL_MOVE );
    return make_pair(score, b.getLastMove());
  }

  if (plyR >= 1) {
    // Take the time and try and order in some reasonable way.
    orderChildren(children);
  }

  bool isWhiteTurn = b.getIsWhiteTurn();

  atomic<int>    bestIndex(-1);
  atomic<int>    atomic_alpha(alpha);
  atomic<int>    atomic_beta(beta);
  atomic<bool>   shouldBreak(false);

  //#pragma omp parallel for if (!FLAGS_use_ttable)
  for (int ci = 0; ci < children.size(); ci++) {
    if (shouldBreak) {
      continue;
    }

    Board child = children[ci];
    auto suggest = findMoveHelper(child, plyR - 1, atomic_alpha, atomic_beta);
    int value = suggest.first;

    auto lastMove = child.getLastMove();
    int fromS = (get<0>(lastMove) << 3) + get<1>(lastMove);
    int toS = (get<2>(lastMove) << 3) + get<3>(lastMove);

    if (isWhiteTurn) {
      if (value > atomic_alpha) {
        bestIndex = ci;
        atomic_alpha = value;
        if (atomic_alpha >= atomic_beta) {
          // Beta cut-off  (Opp won't pick this brach because we can do too well)
          updateHistory(isWhiteTurn, fromS, toS, 1 << plyR);

          shouldBreak = true;
        }
      }
    } else {
      if (value < atomic_beta) {
        bestIndex = ci;
        atomic_beta = value;
        if (atomic_beta <= atomic_alpha) {
          // Alpha cut-off  (We have a strong defense so opp will play older better branch)
          updateHistory(isWhiteTurn, fromS, toS, 1 << plyR);

          shouldBreak = true;
        }
      }
    }
  }

  // Black found a position with score < alpha (strong position for black with black to move).
  // White won't choose to play this path, will instead play whatever path had score > alpha.
  // Score is an hence an upperbound (as black didn't finish the search).
  bool wasAlphaCutoff = shouldBreak && !isWhiteTurn;

  // Found position > beta (strong position for white with white to move).
  // Black won't choose to play this path, will instead play whatever path had score < beta.
  // Score is an hence an lowerbound (as white didn't finish the search).
  bool wasBetaCutoff  = shouldBreak && isWhiteTurn;

  int bestInGen = isWhiteTurn ? atomic_alpha : atomic_beta;
  //assert( alpha <= bestInGen && bestInGen <= beta );

  // TODO figure out why people want me to store refuting move (later searches maybe?)
  move_t suggestion = (wasAlphaCutoff || wasBetaCutoff) ?
      Board::NULL_MOVE : children[bestIndex].getLastMove();

  if (FLAGS_use_ttable && plyR >= 0) {
    char ttType = wasAlphaCutoff ? UPPER_BOUND :
        (wasBetaCutoff ? LOWER_BOUND : EXACT_BOUND);

    TTableEntry *entry = new TTableEntry{ttType, plyR /* depth */, bestInGen, suggestion};
    storeTT(b.getZobrist(), entry);
  }

  return make_pair(bestInGen, suggestion);
}


int Search::quiesce(const Board& b, int alpha, int beta) {
  //quiesceCounter += 1;
  // TODO more implementations here eventually.

  int score = b.heuristic();

  return min(beta, max(alpha, score));
}


int Search::getGameResultScore(board_s gameResult) {
  // Note: Maybe heuristic() would be a good return value but for now disallow.
  assert( gameResult != Board::RESULT_IN_PROGRESS );

  if (gameResult == Board::RESULT_TIE) {
    return 0;
  }
  if (gameResult == Board::RESULT_BLACK_WIN) {
    return -Board::SCORE_WIN;
  }
  if (gameResult == Board::RESULT_WHITE_WIN) {
    return Board::SCORE_WIN;
  }

}


void Search::perft(
    const Board& b,
    int ply,
    atomic<long> *count,
    atomic<long> *captures,
    atomic<int> *ep,
    atomic<int> *castles,
    atomic<int> *promotions,
    atomic<int> *mates) {
  if (ply == 0) {
    move_t move = b.getLastMove();
    board_s moveSpecial = get<6>(move);

    count->fetch_add(1);
//    board_hash_t test = zobrist;
//    assert( test == getZobrist_slow() );

    if (get<5>(move) != 0) { captures->fetch_add(1); }
    if (moveSpecial == Board::SPECIAL_EN_PASSANT) { ep->fetch_add(1); }
    if (moveSpecial == Board::SPECIAL_CASTLE) { castles->fetch_add(1); }
    if (moveSpecial == Board::SPECIAL_PROMOTION) { promotions->fetch_add(1); }

    return;
  }

  vector<Board> children = b.getLegalChildren();
  // TODO This incorrectly counts stalemates.
  if (children.size() == 0) { mates->fetch_add(1); }

  #pragma omp parallel for
  for (int ci = 0; ci < children.size(); ci++) {
    perft(children[ci], ply - 1, count, captures, ep, castles, promotions, mates);
  }
}

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "board.h"
#include "book.h"
#include "flags.h"
#include "search.h"
#include "ttable.h"
//Maybe needed in future
//#include "polyglot.h"
//#include "pst.h"

using namespace std;
using namespace book;
using namespace board;
using namespace search;
using namespace ttable;

//  Have to declare static variable here or something;
Search::Search(bool withTimeControl) {
  useTimeControl = withTimeControl;
  root = Board();

  setup();
}


Search::Search(Board rootB, bool withTimeControl) {
  useTimeControl = withTimeControl;
  root = rootB;

  setup();
}


void Search::setup() {
  nodeCounter = 0;
  ttCounter = 0;
  quiesceCounter = 0;

  // Has the right shape :)
  move_time_dist = gamma_distribution<double>(8.0, 0.2);
}


void Search::save() {
  // TODO make this a proper PGN save.
  int gameNumber = Book::saveMoves(moveNames);
  cout << "\tsaved game as " << gameNumber << endl;
}


void Search::load(int number) {
  moveNames.clear();
  moves.clear();

  vector<string> moveList;
  Book::loadMoves(number, &moveList);

  // Reset root board.
  root = Board();

  // Playback all the moves.
  for (string move : moveList) {
    makeAlgebraicMove(move);
  }
}


Board const Search::getRoot() {
  return root;
}


void Search::makeMove(move_t move) {
  string alg = root.algebraicNotation_slow(move);
  root.makeMove(move);

  moveNames.push_back(alg);
  moves.push_back(move);
}


bool Search::makeAlgebraicMove(string move) {
  bool valid = root.makeAlgebraicMove_slow(move);
  if (valid) {
    moveNames.push_back(move);
    moves.push_back(root.getLastMove());
  }

  return valid;
}


void Search::updateTime(long wTime, long bTime) {
  wMaxTime = max(wMaxTime, wTime);
  bMaxTime = max(bMaxTime, bTime);

  wCurrentTime = wTime;
  bCurrentTime = bTime;
}


long Search::getTimeForMove_millis() {
  long currentTime = root.getIsWhiteTurn() ? wCurrentTime : bCurrentTime;
  long remainingMoves = max((int) (60 - moves.size()), 30);
  long maxOkayTime = currentTime / remainingMoves;

  double tRand = move_time_dist(generator);
  long finalTime = tRand * maxOkayTime;
  return min(20000L, max(finalTime, 100L));
}


long Search::getCurrentTime_millis() {
  return chrono::duration_cast<chrono::milliseconds>(
      chrono::system_clock::now().time_since_epoch()).count();
}


/*****************************************************************************/
/* Code below is algorithmic, above is status                                */
/*****************************************************************************/

void Search::orderChildren(vector<Board> &children) {
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

void Search::stopAfterAllocatedTime(int allocatedTime) {
  long endTime = getCurrentTime_millis() + allocatedTime;
  while (!globalStop && getCurrentTime_millis() < endTime) {
    this_thread::sleep_for(chrono::milliseconds(10));
  }

  // TODO: Figure out how to prevent breaking before plyR = 2 finishes.
  globalStop = true;
}


// Public method that setups and calls helper method.
scored_move_t Search::findMove(int minPly, int minNodes, FindMoveStats *stats) {
  globalStop = false;
  searchStartTime = getCurrentTime_millis();
  long allocatedTime = useTimeControl ? getTimeForMove_millis() : 0;
  thread t1;

  if (useTimeControl) {
    if (FLAGS_verbosity >= 1) {
      cout << "findingAMove " << moves.size() << " moves in" << endl;
      cout << "\tcurrent fen: " << root.generateFen_slow() << endl;
      cout << "\tallocated " << allocatedTime << " millis " << endl;
      root.printBoard();
      cout << endl;
    }

    // TODO: Retrieve book lookup and stuff from old server code.
    // TODO: pull out simple cases?

    t1 = thread(&Search::stopAfterAllocatedTime, this, allocatedTime);
  }

  scored_move_t result = findMoveInner(minPly, minNodes, stats);

  long searchEndTime = getCurrentTime_millis();
  long duration = searchEndTime - searchStartTime;

  if (FLAGS_verbosity >= 2) {
    cout << "\tsearch took " << duration <<
            " (allocated " << allocatedTime << ")" << endl;
  }

  if (useTimeControl) {
    // if stopAfterAllocatedTime hasn't finished clue it to stop.
    globalStop = true;
    t1.join();
  }

  return result;
}


// Takes care of calling iterative deepening till outer thread
scored_move_t Search::findMoveInner(int minPly, int minNodes, FindMoveStats *stats) {
  nodeCounter = 0;
  ttCounter = 0;
  quiesceCounter = 0;

  clearTT();
  clearHistory();

  if (stats) {
    stats->plyR = 0;
    stats->nodes = 0;
  }

  // Check if game has a result
  board_s result = root.getGameResult_slow();
  if (result != Board::RESULT_IN_PROGRESS) {
    int score = Search::getGameResultScore(result, 0);
    return make_pair(score, Board::NULL_MOVE);
  }

  // Check if we only have one move (if so no real choice).
  // Really useful for anti chess where this happens often.
  auto c = root.getLegalChildren();
  if (c.size() == 1) {
    return make_pair(NAN, c[0].getLastMove());
  }

  // Update the global state.
  plySearchDepth = 2;

  // Checkmate this turn
  int maxScore = Search::SCORE_WIN + 101;

  scored_move_t scoredMove;
  int totalNodes = 0;
  while (true) {
    scored_move_t test = findMoveHelper(root, plySearchDepth, -maxScore, maxScore);
    if (globalStop || test.first == SCORE_INTERRUPT) {
      break;
    }

    scoredMove = test;
    totalNodes = nodeCounter + quiesceCounter;

    if (FLAGS_verbosity >= 2) {
      cout << "\tply: " << plySearchDepth << ", score: " << scoredMove.first
           << " (" << totalNodes << " nodes)" << endl;
    }

    if (abs(scoredMove.first) >= Search::SCORE_WIN || totalNodes > minNodes) {
      break;
    }

    plySearchDepth += 1;
  }

  // scoredMove.first == NAN when it's a forced move, otherwise the in search window.
  assert (-maxScore <= scoredMove.first && scoredMove.first <= maxScore);

  string name = root.algebraicNotation_slow(scoredMove.second);
  string ttableDebug = !FLAGS_use_ttable ?
    "" : ("(tt " + to_string(sizeTT()) + ", " + to_string(Search::ttCounter) + ")");

  if (stats) {
    stats->plyR = plySearchDepth;
    stats->nodes = totalNodes;
  }

  if (FLAGS_verbosity >= 2) {
    cout << "\t\tplyR " << plySearchDepth << "=> "
         << nodeCounter << " + " << quiesceCounter << " nodes "
         << ttableDebug
         << " => " << name << " (@ " << scoreString(scoredMove.first) << ")" << endl;
  }

  return scoredMove;
}


scored_move_t Search::findMoveHelper(const Board& b, char plyR, int alpha, int beta) {
  // TODO except at ROOT this doesn't need to return a move.
  // Figure out how to collect PV and change return.

  nodeCounter += 1;

  if (globalStop) {
    return make_pair(SCORE_INTERRUPT, Board::NULL_MOVE);
  }


  if (FLAGS_use_ttable) {
    TTableEntry* lookup = lookupTT(b.getZobrist());
    if (lookup != nullptr) {
      if (lookup->depth >= plyR) {
        ttCounter += 1;

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
    int score = getGameResultScore(status, plySearchDepth - plyR);

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

  #pragma omp parallel for if (!FLAGS_use_ttable)
  for (int ci = 0; ci < children.size(); ci++) {
    if (shouldBreak) {
      continue;
    }

    Board child = children[ci];
    auto suggest = findMoveHelper(child, plyR - 1, atomic_alpha, atomic_beta);
    int value = suggest.first;

    if (value == SCORE_INTERRUPT) {
      shouldBreak = true;
    }

    // History Heuristic not sure if it adds value.
    //auto lastMove = child.getLastMove();
    //int fromS = (get<0>(lastMove) << 3) + get<1>(lastMove);
    //int toS = (get<2>(lastMove) << 3) + get<3>(lastMove);

    if (isWhiteTurn) {
      if (value > atomic_alpha) {
        bestIndex = ci;
        atomic_alpha = value;
        if (atomic_alpha >= atomic_beta) {
          // Beta cut-off  (Opp won't pick this brach because we can do too well)
          //updateHistory(isWhiteTurn, fromS, toS, 1 << plyR);

          shouldBreak = true;
        }
      }
    } else {
      if (value < atomic_beta) {
        bestIndex = ci;
        atomic_beta = value;
        if (atomic_beta <= atomic_alpha) {
          // Alpha cut-off  (We have a strong defense so opp will play older better branch)
          //updateHistory(isWhiteTurn, fromS, toS, 1 << plyR);

          shouldBreak = true;
        }
      }
    }
  }

  if (globalStop) {
    return make_pair(SCORE_INTERRUPT, Board::NULL_MOVE);
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


int Search::getGameResultScore(board_s gameResult, int depth) {
  // Note: Maybe heuristic() would be a good return value but for now disallow.
  assert( gameResult != Board::RESULT_IN_PROGRESS );

  if (gameResult == Board::RESULT_TIE) {
    return 0;
  }

  int dTM = 50 - depth;
  int result = Search::SCORE_WIN + dTM;
  if (gameResult == Board::RESULT_BLACK_WIN) {
    return -result;
  }
  if (gameResult == Board::RESULT_WHITE_WIN) {
    return result;
  }
}


string Search::scoreString(int score) {
  if (abs(score) > Search::SCORE_WIN) {
    int depth = Search::SCORE_WIN + 100 - abs(score);
    return (score > 0 ? "#" : "#-") + to_string( (depth + 1) / 2);
  }
  return to_string(score / 100.0);
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

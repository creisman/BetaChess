#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "book.h"

using namespace std;
using namespace book;

const size_t Book::MAX_DEPTH = 5;
const string Book::ANTICHESS_FILE = "antichess-book.txt";

Book::Book() {
  time_t t = time(NULL);
  struct tm * local = localtime(&t);

  size_t seed = local->tm_hour;
  //cout << "Seeding Book with " << seed << endl;
  randomGenerator.seed(seed);
}

bool Book::load(void) {
  positionsLoaded = 0;

  // read some lines do some stuff build a tree.
  root.move = make_tuple(0, 0, 0, 0, 0, 0, 0);
  root.played = 0;
  root.wins = 0;
  root.losses = 0;
  root.children.clear();

  fstream fs(ANTICHESS_FILE, fstream::in);

  string line;
  vector<BetaChessBookEntry*> path;
  path.push_back(&root);
  while (fs.good()) {
    getline(fs, line);
    if (line.empty()) {
      continue;
    }

    positionsLoaded++;

    size_t depth = 0;
    while (line.size() && line.front() == ' ') {
      line.erase(0, 1); // erase first character.
      depth += 1;
    }

    //cout << "depth: " << depth << "\tline: \"" << line << "\"" << endl;

    int raw[10];
    istringstream strStream(line);
    for (int i = 0; i < 10; i++) {
      char comma;
      strStream >> raw[i];
      strStream >> comma;
      assert(comma == ',' || i == 9);
    }

    BetaChessBookEntry *entry = new BetaChessBookEntry();
    entry->move = make_tuple(raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6]);
    entry->played = raw[7];
    entry->wins = raw[8];
    entry->losses = raw[9];

    assert( depth <= path.size() );

    while (depth < path.size() - 1) {
      path.pop_back();
    }

    if (depth == 0) {
      root.played += entry->played;
    }

    assert (depth == path.size() - 1);
    (path.back()->children).push_back(entry);

    path.push_back(entry);
  }

 
  // Print book for debug purpose.
  //cout << "Loaded book with " << positionsLoaded << " positions" << endl;
  //printBook();

  return true;
}



bool writeHelper(ostream& fs, string prefix, BetaChessBookEntry *entry) {
  assert (entry != nullptr);
  move_t m = entry->move;
  fs << prefix;
  fs << (int)get<0>(m) << "," << (int)get<1>(m) << ","
       << (int)get<2>(m) << "," << (int)get<3>(m) << ","
       << (int)get<4>(m) << "," << (int)get<5>(m) << ","
       << (int)get<6>(m) << ",";
  fs << entry->played << ",";
  fs << entry->wins << ",";
  fs << entry->losses << endl;

  // TODO sort by child game count.
  for (auto child : entry->children) {
    writeHelper(fs, prefix + " ", child);
  }
}
  

bool Book::write(void) {
  string outFile = ANTICHESS_FILE + ".tmp";
  fstream fs(outFile, fstream::out);
  for (auto child : root.children) {
    writeHelper(fs, "", child);
  }
}


bool Book::updateResult(vector<move_t> moves, board_s result) {
  BetaChessBookEntry *entry = &root;
  Board b(true /* init */);

  cout << "End result was " << result << " (for white)" << endl;
  for (int i = 0; i < min(MAX_DEPTH, moves.size()); i++) {
    //b.makeMove(moves[i]);
    assert (b.getLastMove() == moves[i]);
    string moveName = b.algebraicNotation(b.getLastMove());

    BetaChessBookEntry *newEntry = recurseMove(entry, moves[i]);
    if (newEntry == nullptr) {
      cout << "\tAdding entry for move(" << i << "): " << moveName << endl;
      BetaChessBookEntry *newEntry = new BetaChessBookEntry();
      newEntry->move = moves[i];
      newEntry->played = 0;
      newEntry->wins = 0;
      newEntry->losses = 0;

      entry->children.push_back(newEntry);
    } else {
      cout << "\tUpdate (" << i << "): " << moveName << 
              "\t +" << newEntry->wins <<
                " -" << newEntry->losses << 
                " (from " << newEntry->played << ")" << endl;
    }
    entry = newEntry;

    // Update result.
    entry->played += 1;

    if (result == Board::RESULT_WHITE_WIN) {
      entry->wins += 1;
    } else if (result == Board::RESULT_BLACK_WIN) {
      entry->wins += 1;
    } else if (result == Board::RESULT_TIE) {
      // pass.
    } else {
      cerr << "Got invalid result: " << result << endl;
      assert (false); // Invalid result.
    }
  }
}

void Book::printBook() {
  printBook(&root, 0, 10);
}

void Book::printBook(BetaChessBookEntry *entry, int depth, int recurse) {
  if (entry != nullptr) {
    for (int i = 0; i < depth; i++) { cout << " "; }
    cout << stringRecord(entry)
        << " record " << entry->played << " +" << entry->wins << " -" << entry->losses 
        << " with " << entry->children.size() << " children" << endl;
    if (recurse > 0) {
      for (auto child : entry->children) {
        printBook(child, depth + 1, recurse - 1);
      }
    }
  }
}


string Book::stringRecord(BetaChessBookEntry *entry) {
  return "Games: " + to_string(entry->played) +
    " w: " + to_string(entry->wins) +
    " l: " + to_string(entry->losses);
}


string Book::stringMove(move_t move) {
  return to_string(get<0>(move)) + to_string(get<1>(move)) + " to " +
         to_string(get<2>(move)) + to_string(get<3>(move)) + " p: " +
         to_string(get<4>(move)) + " c: " +
         to_string(get<5>(move)) + " s: " +
         to_string(get<6>(move));
}

move_t* Book::multiArmBandit(vector<move_t> moves) {
  BetaChessBookEntry *entry = recurseMoves(&root, moves);
  if (entry == nullptr) {
    return nullptr;
  }

  vector<pair<double, move_t*>> sortedMoves;
  for (auto child : entry->children) {
    // See "How Not To Sort By Average Rating".
    double score = 0;

    int wins = child->wins;
    int losses = child->losses;
    int n = wins + losses;
    if (n > 0) {
      double z = 1.96;
      double zt = z*z / n;

      int goodResult = (moves.size() % 2 == 0) ? wins : losses; 
      double pHat = (1.0 * goodResult) / n;
      score = (pHat + zt/2 - z * sqrt( (pHat * (1 - pHat) + zt/4) / n )) / (1 + zt);
    }

    // TODO verify this with real data later.
    // TODO add move name.
    cout << "\t" << n << " = " << wins << " - " << losses << " with score: " << score << endl;
    sortedMoves.push_back(make_pair(score, &child->move));
  }

  if (sortedMoves.empty()) {
    return nullptr;
  }

  sort(sortedMoves.begin(), sortedMoves.end());
  reverse(sortedMoves.begin(), sortedMoves.end());

  double bestWinRate = sortedMoves[0].first;

  // Small change to choose randomly "Explore"
  if (bestWinRate < 0.3 || randomGenerator() % 5 == 0) {
    cout << "\tMAB is exploring" << endl;
    return sortedMoves[randomGenerator() % sortedMoves.size()].second;
  }

  // "Exploit"
  return sortedMoves[0].second;
}


BetaChessBookEntry* Book::recurseMove(BetaChessBookEntry *entry, move_t moves) {
  //for (auto child : entry->children) {
  //  if (child->move == move) {
  //    return child;
  //  }
  //}
  //cout << "couldn't find move: " << stringMove(move) << endl;
  return nullptr;
}


BetaChessBookEntry* Book::recurseMoves(BetaChessBookEntry *entry, vector<move_t> moves) {
  BetaChessBookEntry *result = entry;
  for (move_t move : moves) {
    result = recurseMove(result, move);
    if (result == nullptr) {
      return result;
    }
  }
  return result;
}


/*
int main(void) {
  cout << "In Book Main!" << endl;

  Book book;
  book.load();
  book.printBook();

  vector<move_t> moves;
  moves.push_back(make_tuple(1,6,2,6,1,0,0));
  book.incrementPlayed(moves);

  book.write();
}
// */

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
#include <map>

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
  bookMap.clear();

  // read some lines do some stuff build a map.
  fstream fs(ANTICHESS_FILE, fstream::in);

  string line;
  while (fs.good()) {
    getline(fs, line);
    if (line.empty()) {
      continue;
    }

    istringstream input(line);
    string part[4];
    for (int i = 0; i < 4; i++) {
      assert( getline( input, part[i], ',') );
    }
    // TODO assert about input being fully consumed.

    BetaChessBookEntry *entry = new BetaChessBookEntry();
    entry->hash = stoull(part[0], nullptr, 16);
    entry->played = stoul(part[1]);
    entry->wins = stoul(part[2]);
    entry->losses = stoul(part[3]);

    assert(bookMap.count(entry->hash) == 0);
    bookMap[entry->hash] = entry;
  }

  // Print book for debug purpose.
  //cout << "Loaded book with " << bookMap.size() << " positions" << endl;
  //printBook();

  return true;
}



bool writeHelper(ostream& fs, BetaChessBookEntry *entry) {
  assert (entry != nullptr);
  fs << hex << entry->hash << dec << ",";
  fs << entry->played << ",";
  fs << entry->wins << ",";
  fs << entry->losses << endl;
}


bool Book::write(void) {
  // TODO sort by hash or something.
  string outFile = ANTICHESS_FILE + ".tmp";
  fstream fs(outFile, fstream::out);
  for (auto entryPair : bookMap) {
    writeHelper(fs, entryPair.second);
  }
}


bool Book::updateResult(vector<string> moves, board_s result) {
  cout << "End result was " << (int) result << " (for white)" << endl;

  Board b;

  // Update starting position.
  BetaChessBookEntry *entry = findOrCreateEntry(b.getZobrist());
  updateEntry(entry, result);

  for (int i = 0; i < min(MAX_DEPTH, moves.size()); i++) {
    // Play a move.
    Board test = b.copy();
    assert( test.makeAlgebraicMove_slow(moves[i]) );
    assert( moves[i] == b.algebraicNotation_slow(test.getLastMove()) );
    b = test;

    // Find or create an entry for this board hash.
    entry = findOrCreateEntry(b.getZobrist());
    if (entry->played == 0) {
      cout << "\tAdding entry for move(" << i << "): " << moves[i] << endl;
    } else {
      cout << "\tUpdate (" << i << "): " << moves[i] << "\t" << stringRecord(entry) << endl;
    }

    // Update the entry with game result.
    updateEntry(entry, result);
  }
}


BetaChessBookEntry* Book::findOrCreateEntry(board_hash_t hash) {
  auto lookup = bookMap.find(hash);
  if (lookup != bookMap.end()) {
    return lookup->second;
  } else {
    BetaChessBookEntry *entry = new BetaChessBookEntry();
    entry->hash = hash;
    entry->played = entry->wins = entry->losses = 0;
    bookMap[entry->hash] = entry;
    return entry;
  }
}


void Book::updateEntry(BetaChessBookEntry *entry, board_s result) {
  assert( entry != nullptr );

  entry->played += 1;

  if (result == Board::RESULT_WHITE_WIN) {
    entry->wins += 1;
  } else if (result == Board::RESULT_BLACK_WIN) {
    entry->losses += 1;
  } else if (result == Board::RESULT_TIE) {
    // pass.
  } else {
    cerr << "Got invalid result: " << result << endl;
    assert (false); // Invalid result.
  }
}

void Book::printBook() {
  Board b;
  printBook(b, "START", 0, 10);
}

void Book::printBook(Board &b, string moveName, int depth, int recurse) {
  auto lookup = bookMap.find(b.getZobrist());
  if (lookup != bookMap.end()) {
    BetaChessBookEntry *entry = lookup->second;
    string start = "";
    start.resize(depth, ' ');

    start += moveName;

    // Pad out depth / move data.
    start.resize(20, ' ');
    cout << start << stringRecord(entry) << endl;
    // Figure out how to count children again.
    if (recurse > 0) {
      for (auto c : b.getLegalChildren()) {
        // Consider sorting by # played or success.
        string newMoveName = b.algebraicNotation_slow(c.getLastMove());
        printBook(c, newMoveName, depth + 1, recurse - 1);
      }
    }
  }
}


string Book::stringRecord(BetaChessBookEntry *entry) {
  return "+" + to_string(entry->wins) +
    " -" + to_string(entry->losses) +
    " from " + to_string(entry->played);
}


string Book::multiArmBandit(Board &b) {
  auto lookup = bookMap.find(b.getZobrist());
  if (lookup == bookMap.end()) {
    return "";
  }
  cout << "\tFound book position for: "
         << hex << b.getZobrist() << dec << endl;

  BetaChessBookEntry *entry = lookup->second;
  assert(entry != nullptr);

  vector<pair<double, string>> sortedMoves;
  for (auto c : b.getLegalChildren()) {
    lookup = bookMap.find(c.getZobrist());
    if (lookup == bookMap.end()) {
      continue;
    }
    BetaChessBookEntry *child = lookup->second;
    string moveName = b.algebraicNotation_slow(c.getLastMove());

    // See "How Not To Sort By Average Rating".
    double score = 0;

    // TODO account for percent of ties or something.
    int wins = child->wins;
    int losses = child->losses;
    int n = wins + losses;
    if (n > 0) {
      double z = 1.96;
      double zt = z*z / n;

      int goodResult = b.getIsWhiteTurn() ? wins : losses;
      double tieMult = b.getIsWhiteTurn() ? 0.4 : 0.6;
      double pHat = (1.0 * goodResult) / n;
      score = (pHat + zt/2 - z * sqrt( (pHat * (1 - pHat) + zt/4) / n )) / (1 + zt);
    }

    // TODO verify this with real data later.
    cout << "\t" << n << " = " << wins << " - " << losses << " with score: " << score << endl;
    sortedMoves.push_back(make_pair(score, moveName));
  }

  if (sortedMoves.empty()) {
    return "";
  }

  sort(sortedMoves.begin(), sortedMoves.end());
  reverse(sortedMoves.begin(), sortedMoves.end());

  double bestWinRate = sortedMoves[0].first;

  // Small change to choose randomly "Explore"
  if (bestWinRate < 0.3 || randomGenerator() % 5 == 0) {
    cout << "\tMAB is exploring" << endl;
    if (randomGenerator() % 2 == 0 && sortedMoves.size() < 3) {
      return "";
    }
    return sortedMoves[randomGenerator() % sortedMoves.size()].second;
  }

  // "Exploit"
  return sortedMoves[0].second;
}


/*
int main(void) {
  cout << "In Book Main!" << endl;

  Book book;
  book.load();
  cout << "\tLoaded" << endl << endl;

  book.printBook();
  cout << endl << endl;

  vector<string> moves = { "Na3", "b5", "Nxb5", "a6", "Nxc7", "Qxc7" };
  book.updateResult(moves, Board::RESULT_WHITE_WIN);

  book.write();
}
// */

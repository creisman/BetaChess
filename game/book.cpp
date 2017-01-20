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


bool Book::updatePlayed(vector<move_t> moves) {
  // TODO recurse and get BetaChessBookEntry
  return true;
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
  BetaChessBookEntry *entry = recurse(moves);
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
      double pHat = (1.0 * wins) / n;
      score = (pHat + zt/2 - z * sqrt( (pHat * (1 - pHat) + zt/4) / n )) / (1 + zt);
    }

    // TODO verify this with real data later.
    //cout << "\t" << n << " = " << wins << " - " << losses << " with score: " << score << endl;
    sortedMoves.push_back(make_pair(score, &child->move));
  }

  if (sortedMoves.empty()) {
    return nullptr;
  }

  sort(sortedMoves.begin(), sortedMoves.end());
  reverse(sortedMoves.begin(), sortedMoves.end());

  // Small change to choose randomly "Explore"
  if (randomGenerator() % 5 == 0) {
    return sortedMoves[randomGenerator() % sortedMoves.size()].second;
  }

  // "Exploit"
  return sortedMoves[0].second;
}


BetaChessBookEntry* Book::recurse(vector<move_t> moves) {
  cout << " Recursing to depth: " << moves.size() << endl;
  BetaChessBookEntry *entry = &root;
  for (move_t move : moves) {
    bool found = false;
    for (auto child : entry->children) {
      if (child->move == move) {
        entry = child;
        found = true;
        break;
      }
    }
    if (!found) {
      // TODO add move or repr of move.
      cout << "couldn't find move: " << endl;
      return nullptr;
    }
  }
  return entry;
}


int main(void) {
  cout << "In Book Main!" << endl;

  Book book;
  book.load();
  book.printBook();
  book.write();
}

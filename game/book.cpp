#include <cassert>
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
}

bool Book::load(void) {
  positionsLoaded = 0;

  // read some lines do some stuff build a tree.
  root.move = make_tuple(0, 0, 0, 0, 0, 0, 0);
  root.played = 10;
  root.wins = 2;
  root.losses = 1;
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
      int part;
      char comma;
      strStream >> part;
      strStream >> comma;
      assert(comma == ',' || i == 9);

      raw[i] = part;
    }

    BetaChessBookEntry entry;
    entry.move = make_tuple(raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6]);
    entry.played = raw[7];
    entry.wins = raw[8];
    entry.losses = raw[9];

    assert( depth <= path.size() );

    while (depth < path.size() - 1) {
      path.pop_back();
    }

    assert (depth == path.size() - 1);
    path.back()->children.push_back(entry);
    path.push_back(&entry);
  }

  cout << "Loaded book with " << positionsLoaded << " positions" << endl;
  return true;
}


bool Book::write(void) {
  // TODO write file out.
  return true;
}


bool Book::updatePlayed(vector<move_t> moves) {
  // TODO recurse and get BetaChessBookEntry
  return true;
}


void Book::printBook(vector<move_t> moves) {
  BetaChessBookEntry *entry = recurse(moves);
  if (entry != nullptr) {
    cout << stringRecord(entry) << " with " << entry->children.size() << " children" << endl;
    for (int i = 0; i < entry->children.size(); i++) {
      cout << "\t" << i << ": " << stringMove(entry->children[i].move) << endl;
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
  // TODO init random with clock hour (so we play same moves in chuncks).
  BetaChessBookEntry *entry = recurse(moves);
  if (entry == nullptr) {
    return nullptr;
  }

  // TODO make this a class variable.
  mt19937 generator(909);

  // TODO actually impliment MAB.
  int games = 0;
  for (auto child = entry->children.begin(); child != entry->children.end(); child++) {
    games += child->played;
  }

  if (games == 0) {
    return nullptr;
  }

  // NOTE this has slight bias but I don't care.
  int gameSelected = generator() % games;
  for (auto child = entry->children.begin(); child != entry->children.end(); child++) {
    gameSelected -= child->played;
    if (gameSelected < 0) {
      return &child->move;
    }
  }

  assert( false );
}


BetaChessBookEntry* Book::recurse(vector<move_t> moves) {
  BetaChessBookEntry *entry = &root;
  for (move_t move : moves) {
    bool found = false;
    for (auto child = entry->children.begin(); child != entry->children.end(); child++) {
      if (child->move == move) {
        entry = &(*child);
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


/*
int main(void) {
  cout << "In Book Main!" << endl;

  Book book;
  book.load();

  vector<move_t> moves;

  book.printBook(moves);
}
*/

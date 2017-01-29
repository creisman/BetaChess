#include <atomic>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "polyglot.h"
#include "board.h"

using namespace std;
using namespace board;


const string Board::PIECE_SYMBOL = "?pnbrqk";

const map<board_s, movements_t> Board::MOVEMENTS = {
  {KNIGHT, {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}}},
  {BISHOP, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}},
  {ROOK, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}},
  {QUEEN, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}}},
  {KING, {{0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1, -1}}},
};

const map<board_s, int> Board::PIECE_VALUE = {
  {KING, 100},
  {QUEEN, 9 },
  {ROOK, 5 },
  {BISHOP, 3 },
  {KNIGHT, 3 },
  {PAWN, 1},
};


const map<board_s, int> Board::ANTICHESS_PIECE_VALUE = {
  {KING, 5},
  {QUEEN, 9 },
  {ROOK, 2 },
  {BISHOP, 2 },
  {KNIGHT, 4 },
  {PAWN, 1},
};


Board::Board(bool initState) {
  if (initState) {
      resetBoard();
  }
}

Board::Board(string fen) {
  // Fen is "easy" they said

  lastMove = make_tuple(0, 0, 0, 0, 0, 0, 0);

  memset(&state, '\0', sizeof(state));

  int y = 7;
  int x = 0;
  int fi;
  for (fi = 0; fi < fen.size(); fi++) {
    if (y == 0 && x == 8) {
      break;
    };

    char c = fen[fi];

    if (c == '/') {
      assert(x == 8);
      y -= 1;
      x = 0;
    } else if (c >= '1' && c <= '8') {
      x += c - '0';
    } else {
      // c should be in PIECE_SYMBOL
      int color = isupper(c) ? WHITE : BLACK;
      size_t piece = PIECE_SYMBOL.find(tolower(c));

      assert(piece >= PAWN && piece <= KING);
      board_s insert = color * piece;
      state[y][x] = insert;
      x += 1;
    }
  }

  // Next is who to play.
  assert(fen[fi] == ' ');
  fi++;
  assert(fen[fi] == 'b' || fen[fi] == 'w');
  ply = (fen[fi] == 'w') ? 0 : 1;
  isWhiteTurn = ply % 2 == 0;
  fi++;

  // Next is castling
  assert(fen[fi] == ' ');
  fi++;
  castleStatus = 0;
  while (fen[fi] != ' ') {
    if (fen[fi] == 'K') { castleStatus |= WHITE_OO; }; // whiteOO = true
    if (fen[fi] == 'Q') { castleStatus |= WHITE_OOO; }; // whiteOOO = true
    if (fen[fi] == 'k') { castleStatus |= BLACK_OO; }; // blackOO = true
    if (fen[fi] == 'q') { castleStatus |= BLACK_OOO; }; // blackOOO = true
    fi++;
  }

  // TODO Next is En passant
  // TODO Next is halfmove clock
  // TODO Next is fullmove clock

  // TODO matestatus
  materialDiff = getPiecesValue_slow();
  zobrist = getZobrist_slow();
}


void Board::resetBoard(void) {
  ply = 0;
  isWhiteTurn = true;
  materialDiff = 0;

  // whiteOO = whiteOOO = true;
  // blackOO = blackOOO = true;
  castleStatus = 15;

  memset(&state, '\0', sizeof(state));

  // TODO figure out what this should be.
  lastMove = make_tuple(0, 0, 0, 0, 0, 0, 0);

  for (int i = 0; i < 8; i++) {
    state[1][i] = PAWN;
    state[6][i] = -PAWN;
  }

  state[0][0] = ROOK;
  state[0][1] = KNIGHT;
  state[0][2] = BISHOP;
  state[0][3] = QUEEN;
  state[0][4] = KING;
  state[0][5] = BISHOP;
  state[0][6] = KNIGHT;
  state[0][7] = ROOK;

  state[7][0] = -ROOK;
  state[7][1] = -KNIGHT;
  state[7][2] = -BISHOP;
  state[7][3] = -QUEEN;
  state[7][4] = -KING;
  state[7][5] = -BISHOP;
  state[7][6] = -KNIGHT;
  state[7][7] = -ROOK;

  materialDiff = getPiecesValue_slow();
  zobrist = getZobrist_slow();
}


Board Board::copy() {
  // Call "copy" constructor.
  Board copy = *this;
  return copy;
}


string Board::boardStr(void) {
  string rep = "";
  for (int row = 7; row >= 0; row--) {
    rep += "|";
    for (int col = 0; col < 8; col++) {
      board_s piece = state[row][col];
      board_s absPiece = (piece >= 0) ? piece : -piece;
      char symbol = (piece != 0) ? PIECE_SYMBOL[absPiece] : '.';
      if (piece > 0) {
        symbol = toupper(symbol);
      }
      rep += symbol;
      }
    rep += "|\n";
  }

  return rep;
}

void Board::printBoard(void) {
  cout << boardStr() << endl;
}

uint64_t Board::getZobrist(void) {
  return zobrist;
}

move_t Board::getLastMove(void) {
  return lastMove;
}

vector<Board> Board::getChildren(void) {
  bool hasCapture = false;
  vector<Board> all_moves;

  board_s pawnDirection = isWhiteTurn ? 1 : -1;
  board_s selfColor = isWhiteTurn ? WHITE : BLACK;
  board_s oppColor = isWhiteTurn ? BLACK : WHITE;

  pair<bool, board_s> moveTest;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      board_s piece = state[y][x];
      if (piece == 0 || isWhiteTurn != isWhitePiece(piece)) {
        continue;
      }
      board_s absPiece = abs(piece);

      if (absPiece == PAWN) {
        // pawn capture
        for (board_s lr = -1; lr <= 1; lr += 2) {
          moveTest = attemptMove(y + pawnDirection, x + lr);
          if (moveTest.first && moveTest.second == oppColor) {
            // Pawn Capture (plus potential promotion)
            promoHelper(&all_moves, isWhiteTurn, selfColor, x, y, x + lr, y + pawnDirection);
            hasCapture = true;
          }
        }

        if (IS_ANTICHESS && hasCapture) {
          continue;
        }

        moveTest = attemptMove(y + pawnDirection, x);
        // pawn move: if next space is empty.
        if (moveTest.first && moveTest.second == 0) {
          // Normal move forward && promo
          promoHelper(&all_moves, isWhiteTurn, selfColor, x, y, x, y + pawnDirection);

          // double move (only if nothing in the way for single move)
          if ((isWhiteTurn && y == 1) || (!isWhiteTurn && y == 6)) {
            moveTest = attemptMove(y + 2 * pawnDirection, x);
            if (moveTest.first && moveTest.second == 0) {
              Board c = copy();
              c.makeMove(y,x,    y + 2 * pawnDirection, x);
              all_moves.push_back( c );
            }
          }
        }
      } else if (absPiece == KNIGHT || absPiece == KING) {
        // "jumpy" pieces = KNIGHT, KING
        for (auto iter = MOVEMENTS.at(absPiece).begin();
                  iter != MOVEMENTS.at(absPiece).end();
                  iter++) {
          moveTest = attemptMove(y + iter->first, x + iter->second);
          if (moveTest.first && moveTest.second != selfColor) {
            bool isCapture = moveTest.second != 0;
            hasCapture |= isCapture;
            if (IS_ANTICHESS && !isCapture && hasCapture) {
              continue;
            }

            Board c = copy();
            c.makeMove(y,x,   y + iter->first, x + iter->second);
            all_moves.push_back( c );
          }
        }
      } else {
        // slidy pieces = BISHOPS, ROOKS, QUEENS
        assert( absPiece == BISHOP || absPiece == ROOK || absPiece == QUEEN );
        for (auto iter = MOVEMENTS.at(absPiece).begin();
                  iter != MOVEMENTS.at(absPiece).end();
                  iter++) {
          board_s deltaY = iter->first;
          board_s deltaX = iter->second;
          board_s newY = y;
          board_s newX = x;
          while (true) {
            newY += deltaY;
            newX += deltaX;

            moveTest = attemptMove(newY, newX);

            if (!moveTest.first || moveTest.second == selfColor) {
              break;
            }

            bool isCapture = moveTest.second != 0;
            hasCapture |= isCapture;
            if (IS_ANTICHESS && !isCapture && hasCapture) {
              continue;
            }

            Board c = copy();
            c.makeMove(y, x,   newY, newX);
            all_moves.push_back( c );

            if (moveTest.second == oppColor) {
              break;
            }
          }
        }
      }
    }
  }

  // Castling
  if (!hasCapture || !IS_ANTICHESS) {
    // TODO: This is likely broken for Antichess (but not CRITICALLY?)

    int y = isWhiteTurn ? 0 : 7;
    int x = 4;
    if (state[y][x] == selfColor * KING) {
      bool canOO = castleStatus & (isWhiteTurn ? WHITE_OO : BLACK_OO);
      bool canOOO = castleStatus & (isWhiteTurn ? WHITE_OOO : BLACK_OOO);
      // OOO
      if (canOOO && (state[y][0] == selfColor * ROOK)) {
        // Check empty squares.
        if (state[y][1] == 0 && state[y][2] == 0 && state[y][3] == 0) {
          // chek for attack on [4] [3] and [2]
          if ((checkAttack_medium(isWhiteTurn, y, 4) == 0) &&
              (checkAttack_medium(isWhiteTurn, y, 3) == 0) &&
              (checkAttack_medium(isWhiteTurn, y, 2) == 0)) {
            Board c = copy();
            c.makeMove(y, 4,   y, 2, SPECIAL_CASTLE); // Record king over two as the move.
            all_moves.push_back( c );
          }
        }
      }
      if (canOO && (state[y][7] == selfColor * ROOK)) {
        // Check empty squares.
        if (state[y][5] == 0 && state[y][6] == 0) {
          // chek for attack on [4] [5] and [6]
          if ((checkAttack_medium(isWhiteTurn, y, 4) == 0) &&
              (checkAttack_medium(isWhiteTurn, y, 5) == 0) &&
              (checkAttack_medium(isWhiteTurn, y, 6) == 0)) {
            Board c = copy();
            c.makeMove(y, 4,   y, 6, SPECIAL_CASTLE); // Record king over two as the move.
            all_moves.push_back( c );
          }
        }
        // Have to check for attack and empty squares.
      }
    }
  }

  // En passant
  // Pawn moving two spaces forward! (lastMove = (a, b), (c, d) moving, removing)
  if (get<4>(lastMove) == oppColor * PAWN && abs(get<0>(lastMove) - get<2>(lastMove)) == 2) {
    assert( get<1>(lastMove) == get<3>(lastMove) );
    // pawn moved to c so en passant can only happen from c - pawnDirection;
    board_s lastY = get<2>(lastMove);
    board_s lastX = get<3>(lastMove);

    for (board_s lr = -1; lr <= 1; lr += 2) {
      board_s testX = lastX + lr;

      // Capturing pawn is adjacent after double move.
      if (0 <= testX && testX <= 7 && state[lastY][testX] == selfColor * PAWN) {
        Board c = copy();
        // Move our pawn
        c.makeMove(lastY, testX, lastY + pawnDirection, lastX, SPECIAL_EN_PASSANT);
        all_moves.push_back( c );
      }
    }
  }
  return all_moves;
}

vector<Board> Board::getLegalChildren(void) {
  if (IS_ANTICHESS) {
    // TODO this is the simple version, could be partially moved into getChildren.
    vector<Board> all_moves = getChildren();

    if (all_moves.size() == 0) {
      return all_moves;
    }

    bool hasCapture = get<5>(all_moves.back().getLastMove()) != 0;
    auto test = all_moves.begin();
    for (; test != all_moves.end(); test++) {
      bool isCapture = get<5>(test->getLastMove()) != 0;
      if (isCapture) {
        break;
      }
      if (hasCapture) {
        all_moves.erase(test);
        test--;
      }
    }

    // Verify remaining items are all captures.
    for (; test != all_moves.end(); test++) {
      bool isCapture = get<5>(test->getLastMove()) != 0;
      assert( isCapture );
    }

    return all_moves;
  }


  board_s selfColor = isWhiteTurn ? WHITE : BLACK;
  board_s selfKing = selfColor * KING;
  // A guess where king will be (or nearby).
  board_s kingY = -1;
  board_s kingX;

  for (int y = 0; kingY == -1 && y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      board_s piece = state[y][x];
      if (piece == selfKing) {
        kingY = y;
        kingX = x;
        break;
      }
    }
  }

  assert( onBoard(kingY, kingX) );

  // TODO lots of optimizations
  //    was square under double attack => had to move
  //    was square under knight attack => had to destroy knight or move
  //    was attack from adjacent square => had to move or destroy
  //    was square under slide attack =>
  //      if king didn't move
  //        last move must be in way of single attack.

  vector<Board> all_moves = getChildren();
  for (auto test = all_moves.begin(); test != all_moves.end(); test++) {
    board_s testY = kingY;
    board_s testX = kingX;

    // Check if king is still in same place.
    if (test->state[kingY][kingX] != selfKing) {
      // king must have moved (lookup where it went).
      testY = get<2>(test->getLastMove());
      testX = get<3>(test->getLastMove());
      assert( onBoard(testY, testX) );
    }

    assert( test->state[testY][testX] == selfKing );
    if (test->checkAttack_medium(isWhiteTurn, testY, testX) != 0) {
      all_moves.erase(test);
      test--;
    }
  }
  return all_moves;
}

// inline
void Board::promoHelper(
  vector<Board> *all_moves,
  bool isWhiteTurn,
  board_s selfColor,
  board_s x,
  board_s y,
  board_s x2,
  board_s y2) {
  if ((isWhiteTurn && y2 == 7) || (!isWhiteTurn && y2 == 0)) {
    // promotion && underpromotion
    board_s lastPromoPiece = IS_ANTICHESS ? KING : QUEEN;
    for (board_s newPiece = KNIGHT; newPiece <= lastPromoPiece; newPiece++) {
      // "promote" then move piece (this means history shows Queen moving to back row not a pawn)
      Board c = copy();
      c.state[y][x] = selfColor * newPiece;
      c.makeMove(y,x,    y2, x2, SPECIAL_PROMOTION);
      all_moves->push_back(c);
    }
  } else {
    // Normal move to square.
    Board c = copy();
    c.makeMove(y,x,    y2, x2);
    all_moves->push_back(c);
  }
}


// TODO a duplicate version that returns location of attack, count of attack
board_s Board::checkAttack_medium(bool byBlack, board_s a, board_s b) {
  // TODO consider pre calculating for each square in getChildrenMove of parent.
  // returns piece or 0 for no

  board_s selfColor = byBlack ? WHITE : BLACK;
  board_s oppKnight = byBlack ? -KNIGHT: KNIGHT;
  board_s selfPawnDirection = byBlack ? 1 : -1;

  // Check if knight is attacking square.
  for (auto iter = MOVEMENTS.at(KNIGHT).begin();
            iter != MOVEMENTS.at(KNIGHT).end();
            iter++) {
    board_s attackingKnight = getPiece(a + iter->first, b + iter->second);
    if (attackingKnight ==  oppKnight) {
      return attackingKnight;
    }
  }

  pair<bool, board_s> moveTest;
  // check if slidy piece is attacking square.
  for (auto iter = MOVEMENTS.at(QUEEN).begin();
            iter != MOVEMENTS.at(QUEEN).end();
            iter++) {
    board_s deltaY = iter->first;
    board_s deltaX = iter->second;
    board_s newY = a;
    board_s newX = b;
    while (true) {
      newY += deltaY;
      newX += deltaX;

      moveTest = attemptMove(newY, newX);
      if (!moveTest.first || moveTest.second == selfColor) {
        break;
      }

      if (moveTest.second != 0) {
        const board_s oppPiece = state[newY][newX];
        const board_s pieceType = abs(oppPiece);
        // check if oppPiece can deliver this type of attack
        if (pieceType == QUEEN) {
          return oppPiece;
        }

        board_s distance = abs(a - newY) + abs(b - newX);

        bool isStraight = iter->first == 0 || iter->second == 0;
        if (isStraight) {
          if (distance == 1 && pieceType == KING) {
            return oppPiece;
          }
          if (pieceType == ROOK) {
            return oppPiece;
          }
          break;
        }

        bool isDiagonal = iter->first != 0 && iter->second != 0;
        assert( isDiagonal );

        if (pieceType == BISHOP) {
          return oppPiece;
        }

        if (distance == 2) {
          // KING or PAWN might be attacker
          if (pieceType == KING) {
            return oppPiece;
          }

          // vector from king to piece so inverted pawn direction.
          if (deltaY == selfPawnDirection && pieceType == PAWN) {
            return oppPiece;
          }
        }

        // Hit a piece, nothing does x-ray.
        break;
      }
    }
  }
  return 0;
}

pair<bool, board_s> Board::attemptMove(board_s a, board_s b) {
  // prep for moving piece to state[a][b]
  // returns on board, piece on [a][b]

  if (onBoard(a, b)) {
    board_s destPiece = state[a][b];
    if (destPiece) {
      return make_pair(true, peaceSign(destPiece));
    }
    return make_pair(true, 0);
  }
  return make_pair(false, 0);
}

board_s Board::getPiece(board_s a, board_s b) {
  return onBoard(a, b) ? state[a][b] : 0;
}

void Board::makeMove(move_t move) {
  board_s a = get<0>(move);
  board_s b = get<1>(move);
  board_s c = get<2>(move);
  board_s d = get<3>(move);
  board_s moving  = get<4>(move);
  board_s capture = get<5>(move);
  unsigned char special = get<6>(move);

  if (special != SPECIAL_EN_PASSANT) {
    assert (state[c][d] == capture);
  }

  state[a][b] = moving;
  makeMove(a, b, c, d, special);
  // This is the special magic for promotions.

  assert(this->getLastMove() == move);
}

void Board::makeMove(board_s a, board_s b, board_s c, board_s d, unsigned char special) {
  if (special == SPECIAL_CASTLE) {
    if (d == 2) {
      assert(abs(state[a][0]) == ROOK);
      state[a][3] = state[a][0];
      state[a][0] = 0;
    } else {
      assert(d == 6);
      assert(abs(state[a][7]) == ROOK);
      state[a][5] = state[a][7];
      state[a][7] = 0;
    }
  }

  makeMove(a, b, c, d);
  get<6>(lastMove) = special;

  if (special == SPECIAL_EN_PASSANT) {
    get<5>(lastMove) = state[a][d];
    updateMaterialDiff(state[a][d]);
    assert(abs(state[a][d]) == PAWN);
    state[a][d] = 0;
  }
}

void Board::makeMove(board_s a, board_s b, board_s c, board_s d) {
  board_s moving = state[a][b];
  bool isWhite = isWhitePiece(moving);
  board_s removed = state[c][d];

  assert( isWhite == isWhiteTurn );
  if (removed != 0) {
    assert( isWhitePiece(removed) != isWhiteTurn );
  }

  state[a][b] = 0;
  state[c][d] = moving;

  // update turn state
  ply++;
  isWhiteTurn = !isWhiteTurn;
  lastMove = make_tuple(a, b, c, d, moving, removed, 0);
  if (removed != 0) {
    updateMaterialDiff(removed);
  }

  // update castling.
  if (isWhite) {
    // if king moves (king can't be captured so if it leaves square it moves)
    if (a == 0 && b == 4 && moving == KING) { castleStatus &= 3; } // whiteOO = 0, whiteOOO = 0

    // TODO correctly apply zobrist rules here.

    // rook moving back doesn't count.
    if (c == 0 && d == 0 && moving == ROOK) { castleStatus &= 7; } // whiteOO = 0
    if (c == 0 && d == 7 && moving == ROOK) { castleStatus &= 11; } // whiteOOO =0
  } else {
    // if king moves (king can't be captured so if it leaves square it moves)
    if (a == 7 && b == 4 && moving == -KING) { castleStatus &= 12; } // blackOO = 0, blackOOO = 0

    // TODO correctly apply zobrist rules here.

    // rook moving back doesn't count.
    if (c == 7 && d == 0 && moving == -ROOK) { castleStatus &= 13; } // blackOO = 0
    if (c == 7 && d == 7 && moving == -ROOK) { castleStatus &= 14; } // blackOOO = 0;
  }
}

board_s Board::getGameResult_slow(void) {
  // TODO: Tie a lot of moves since pawn move.

  if (IS_ANTICHESS) {
    bool hasMove = !getLegalChildren().empty();

    // To win, don't have a valid move.
    if (hasMove) {
      // TODO check for draw conditions (insufficent material, no move, ...).
      return RESULT_IN_PROGRESS;
    }

    // TODO check stalemate.
    return isWhiteTurn ? RESULT_WHITE_WIN : RESULT_BLACK_WIN;
  }

  pair<board_s, board_s> kingPos =
      findPiece_slow(isWhiteTurn ? KING : -KING);
  assert( onBoard(get<0>(kingPos), get<1>(kingPos)) );

  bool inCheck = checkAttack_medium(
      !isWhiteTurn /* byBlack */,
      get<0>(kingPos),
      get<1>(kingPos)) == 0;

  // This could be improved (maybe making this _medium) by instead checking move_exists?
  vector<Board> children = getLegalChildren();
  bool hasChildren = !children.empty();


  if (hasChildren) {
      return RESULT_IN_PROGRESS;
  }

  // Loss conditions: no moves + in check.
  if (!hasChildren && inCheck) {
    return isWhiteTurn ? RESULT_WHITE_WIN : RESULT_BLACK_WIN;
  }

  // Tie (stalemate) conditions: no moves + not in check.
  if (!hasChildren && !inCheck) {
    return RESULT_TIE;
  }

  assert(false);
}


string Board::algebraicNotation(move_t child_move) {
  // e4 = white king pawn double out.
  // e is file.
  // 4 is rank.

  bool mult = false;
  bool sameFile = false;
  bool sameRank = false;

  // NOTE(seth): It appears disambigous is only looking at "valid" moves.
  vector<Board> children = getLegalChildren();
  for (Board c : children) {
    // same piece, same destination
    if ((get<2>(child_move) == get<2>(c.lastMove)) &&
        (get<3>(child_move) == get<3>(c.lastMove)) &&
        (get<4>(child_move) == get<4>(c.lastMove))) {
      bool equalFile = get<1>(child_move) == get<1>(c.lastMove);
      bool equalRank = get<0>(child_move) == get<0>(c.lastMove);

      if (equalFile && equalRank) {
        // the move itself.
        continue;
      }
      mult = true;
      sameFile |= equalFile;
      sameRank |= equalRank;
    }
  }

  string capture = get<5>(child_move) == 0 ? "" : "x";

  board_s piece = abs(get<4>(child_move));
  string pieceName = string(1, toupper(PIECE_SYMBOL[piece]));
  if (piece == PAWN) {
    pieceName = "";
  }

  string dest = squareName(get<2>(child_move), get<3>(child_move));

  bool fileDisambigs = mult && (!sameFile || sameFile && sameRank);
  string disambiguate = (fileDisambigs ? fileName(get<1>(child_move)) : "") +
                        (sameFile      ? rankName(get<0>(child_move)) : "");

  // TODO consider adding check/mate (+/#) status.

  unsigned char special = get<6>(child_move);
  if (special == SPECIAL_PROMOTION) {
    // Piece handly records what we promoted to!
    // But it's not a recorded as a pawn move so add capture logic again.
    if (!capture.empty()) {
      return fileName(get<1>(child_move)) + capture + dest +  "=" + pieceName;
    }
    return dest + "=" + pieceName;
  }
  if (special == SPECIAL_CASTLE) {
    return (get<3>(child_move) == 2) ? "O-O-O" : "O-O";
  }

  // Pawn captures get file added.
  if (piece == PAWN && !capture.empty()) {
    // A special (generic) case of disambiguate.
    // Can't be disambigous once we know file.
    pieceName = fileName(get<1>(child_move));
    return pieceName + capture + dest;
  }

  return pieceName + disambiguate + capture + dest;
}


string Board::coordinateNotation(move_t move) {
  // This partially (via inference) supports castling, ep
  // and has explicit promotion.

  string fromTo = squareName(get<0>(move), get<1>(move)) + " - " +
                  squareName(get<2>(move), get<3>(move));

  if (get<6>(move) == SPECIAL_PROMOTION) {
    string promotedTo = string(1, toupper(PIECE_SYMBOL[abs(get<4>(move))]));
    return fromTo + "(" + promotedTo + ")";
  }
  return fromTo;
}


string Board::squareName(board_s a, board_s b) {
  assert( onBoard(a, b) );
  return fileName(b) + rankName(a);
}


string Board::rankName(board_s a) {
  return to_string(1 + a);
}


string Board::fileName(board_s b) {
  char file = 'a' + b;
  return string(1, file);
}


board_s Board::getPieceValue(board_s piece) {
  assert(piece != 0);
  board_s absPiece = abs(piece);
  if (IS_ANTICHESS) {
    // Pieces have negative value. This might be a bad idea but it simplifies right now.
    return  -peaceSign(piece) * ANTICHESS_PIECE_VALUE.at(absPiece);
  } else {
    return  peaceSign(piece) * PIECE_VALUE.at(absPiece);
  }
}

bool Board::isWhitePiece(board_s piece) {
  return piece > 0;
}

board_s Board::peaceSign(board_s piece) {
//  return (piece == 0) ? 0 : (piece > 0) ? WHITE : BLACK;
  return (0 < piece) - (piece < 0);
}

bool Board::onBoard(board_s a, board_s b) {
  return 0 <= a && a <= 7 && 0 <= b && b <= 7;
}


void Board::updateMaterialDiff(board_s removed) {
  assert(removed != 0);
  materialDiff -= getPieceValue(removed);
}

int Board::getPiecesValue_slow(void) {
  int pieceValue = 0;
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      board_s piece = state[r][c];
      if (piece != 0) {
        pieceValue += getPieceValue(piece);
      }
    }
  }
  return pieceValue;
}


void Board::updateZobristPiece(board_s a, board_s b, board_s piece) {
  short kindOfPiece = 2 * (abs(piece) - 1)  + isWhitePiece(piece);
  short index = 64 * kindOfPiece + 8 * a + b; // a = rank, b = file
  assert (0 <= index && index < 768);
  zobrist ^= POLYGLOT_RANDOM[index];
}


uint64_t Board::getZobrist_slow(void) {
  zobrist = 0;
  for (int r = 0; r < 8; r++) {
    for (int f = 0; f < 8; f++) {
      board_s piece = state[r][f];
      if (piece != 0) {
        updateZobristPiece(r, f, piece);
      }
    }
  }

  zobrist ^= ((castleStatus & WHITE_OO)  > 0) * POLYGLOT_RANDOM[768 + 0];
  zobrist ^= ((castleStatus & WHITE_OOO) > 0) * POLYGLOT_RANDOM[768 + 1];
  zobrist ^= ((castleStatus & BLACK_OO)  > 0) * POLYGLOT_RANDOM[768 + 2];
  zobrist ^= ((castleStatus & BLACK_OOO) > 0) * POLYGLOT_RANDOM[768 + 3];

  // TODO enpassant.

  zobrist ^= (isWhiteTurn > 0) * POLYGLOT_RANDOM[780];
  return zobrist;
}


double Board::heuristic() {
  //int pieceValue = getPiecesValue_slow();
  //assert( pieceValue == materialDiff );
  int pieceValue = materialDiff;

  if (IS_ANTICHESS) {
    // TODO cache between boards.

    // Was lastmove a capture?
    int heuristicSign = isWhiteTurn ? 1 : -1;
    double haveTempo = heuristicSign * 6.0 * (get<5>(lastMove) != 0);

    // TODO check if we also have to capture.

    return haveTempo + pieceValue;
  }

  /*
  200(K-K')
  + 9(Q-Q')
  + 5(R-R')
  + 3(B-B' + N-N')
  + 1(P-P')
  - 0.5(D-D' + S-S' + I-I')
  + 0.1(M-M')
  */

  //int whiteMobility = 0
  //int blackMobility = 0

  if (pieceValue > 50) {
    // black is missing king.
    materialDiff = WHITE_WIN;
  } else if (pieceValue < -50) {
    materialDiff = BLACK_WIN;
  }

  //sumValueWhite = sum(pieceValue[piece[0]] for piece in self.whitePieces)
  //sumValueBlack = sum(pieceValue[-piece[0]] for piece in self.blackPieces)

  //mobilityBonus = 0.1 * (whiteMobility - blackMobility)
  //# TODO Determine doubled, blocked, and isolated pawns

  //return sumValueWhite - sumValueBlack + mobilityBonus
  return pieceValue;
}


// Public method that setups and calls helper method.
atomic<int> Board::dbgCounter(0);
scored_move_t Board::findMove(int minNodes) {
  Board::dbgCounter = 0;

  // Check if game has a result
  board_s result = getGameResult_slow();
  if (result != RESULT_IN_PROGRESS) {
    cout << "Game Result: " << (int) result << endl;
    move_t emptyMove = make_tuple(0, 0, 0, 0, 0, 0, 0);
    return make_pair(result, emptyMove);
  }

  // Check if we only have one move (if so no real choice).
  auto c = getLegalChildren();
  if (c.size() == 1) {
    return make_pair(NAN, c[0].getLastMove());
  }


  int plyR = 4;
  scored_move_t scoredMove;
  while (true) {
    scoredMove = findMoveHelper(plyR, -1000.0, 1000);
    if (abs(scoredMove.first) > 400 || dbgCounter > minNodes) {
      break;
    }
    plyR += 1;
  }

  // scoredMove.first == NAN when it's a forced move.

  cout << "\t\tplyR " << plyR << "=> " << Board::dbgCounter << " nodes" << endl;

  return scoredMove;
}


scored_move_t Board::findMoveHelper(int plyR, double alpha, double beta) {
  Board::dbgCounter += 1;
  if (plyR == 0) {
    return make_pair(heuristic(), lastMove);
  }
  double bestInGen = isWhiteTurn ? -1000.0 : 1000.0 ;
  move_t suggestion;
  vector<Board> children = getLegalChildren();

  if (children.empty()) {
    // TODO callGameResultStatus
    // Node is a winner!
    double score = isWhiteTurn ? 500 : -500;
    return make_pair(score, lastMove);
  }

  atomic<double> atomic_alpha(alpha);
  atomic<double> atomic_beta(beta);
  atomic<bool> shouldBreak(false);
  #pragma omp parallel for
  for (int ci = 0; ci < children.size(); ci++) {
    if (shouldBreak) {
      continue;
    }

    auto suggest = children[ci].findMoveHelper(plyR - 1, atomic_alpha, atomic_beta);
    double value = suggest.first;

    if (isWhiteTurn) {
     if (value > bestInGen) {
        suggestion = children[ci].getLastMove();
        bestInGen = value;
        atomic_alpha = max(atomic_alpha.load(), bestInGen);
        if (atomic_beta <= atomic_alpha) {
          // Beta cut-off  (Opp won't pick this brach because we can do too well)
          shouldBreak = true;
          //break;
        }
      }
    } else {
     if (value < bestInGen) {
        suggestion = children[ci].getLastMove();
        bestInGen = value;
        atomic_beta = min(atomic_beta.load(), bestInGen);
        if (atomic_beta <= atomic_alpha) {
          // Alpha cut-off  (We have a strong defense so opp will play older better branch)
          shouldBreak = true;
          //break;
        }
      }
    }
  }
  return make_pair(bestInGen, suggestion);
};


pair<board_s, board_s> Board::findPiece_slow(board_s piece) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (state[y][x] == piece) {
        return make_pair(y, x);
      }
    }
  }
  return make_pair(-1, -1);
}


void Board::perft(
    int ply,
    atomic<int> *count,
    atomic<int> *captures,
    atomic<int> *ep,
    atomic<int> *castles,
    atomic<int> *promotions,
    atomic<int> *mates) {
  if (ply == 0) {
    move_t move = getLastMove();
    board_s moveSpecial = get<6>(move);

    count->fetch_add(1);

    if (get<5>(move) != 0) { captures->fetch_add(1); }
    if (moveSpecial == SPECIAL_EN_PASSANT) { ep->fetch_add(1); }
    if (moveSpecial == SPECIAL_CASTLE) { castles->fetch_add(1); }
    if (moveSpecial == SPECIAL_PROMOTION) { promotions->fetch_add(1); }

    return;
  }

  vector<Board> children = getLegalChildren();
  // TODO This incorrectly counts stalemates.
  if (children.size() == 0) { mates->fetch_add(1); }

  #pragma omp parallel for
  for (int ci = 0; ci < children.size(); ci++) {
    children[ci].perft(ply - 1, count, captures, ep, castles, promotions, mates);
  }
}

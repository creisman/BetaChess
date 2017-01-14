#include <cassert>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

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
  {KING, 200},
  {QUEEN, 9 },
  {ROOK, 5 },
  {BISHOP, 3 },
  {KNIGHT, 3 },
  {PAWN, 1},
};


Board::Board(bool initState) {
  if (initState) {
      resetBoard();
  }
}

Board::Board(string fen) {
  // Fen is "easy" they said

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
  mateStatus = -2;

}


Board Board::copy() {
  // Call "copy" constructor.
  Board copy = *this;
  return copy;
}


void Board::resetBoard(void) {
  ply = 0;
  isWhiteTurn = true;
  mateStatus = -2;

  // whiteOO = whiteOOO = true;
  // blackOO = blackOOO = true;
  castleStatus = 15;

  memset(&state, '\0', sizeof(state));

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
}

string Board::boardStr(void) {
  string rep = "----------\n";
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
  rep += "----------\n";

  return rep;
}

void Board::printBoard(void) {
  cout << boardStr() << endl;
}

move_t Board::getLastMove(void) {
  return lastMove;
}

board_s Board::getLastMoveSpecial(void) {
  return lastMoveSpecial;
}

vector<Board> Board::getChildren(void) {
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
            promoHelper(&all_moves, isWhiteTurn, selfColor, pawnDirection, x, y, x + lr);
          }
        }

        moveTest = attemptMove(y + pawnDirection, x);
        // pawn move: if next space is empty.
        if (moveTest.first && moveTest.second == 0) {
          // Normal move forward && promo    
          promoHelper(&all_moves, isWhiteTurn, selfColor, pawnDirection, x, y, x);

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
  {
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
          if ((checkAttack(isWhiteTurn, y, 4) == 0) &&
              (checkAttack(isWhiteTurn, y, 3) == 0) &&
              (checkAttack(isWhiteTurn, y, 2) == 0)) {
            Board c = copy();
            c.state[y][3] = c.state[y][0]; // Rook over three.
            c.state[y][0] = 0;
            c.makeMove(y, 4,   y, 2); // Record king over two as the move.
            c.lastMoveSpecial = SPECIAL_CASTLE;
            all_moves.push_back( c );
          }
        }
      }
      if (canOO && (state[y][7] == selfColor * ROOK)) {
        // Check empty squares.
        if (state[y][5] == 0 && state[y][6] == 0) {
          // chek for attack on [4] [5] and [6]
          if ((checkAttack(isWhiteTurn, y, 4) == 0) &&
              (checkAttack(isWhiteTurn, y, 5) == 0) &&
              (checkAttack(isWhiteTurn, y, 6) == 0)) {
            Board c = copy();
            c.state[y][5] = c.state[y][7]; // Rook over three.
            c.state[y][7] = 0;
            c.makeMove(y, 4,   y, 6); // Record king over two as the move.
            c.lastMoveSpecial = SPECIAL_CASTLE;
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
        c.makeMove(lastY, testX, lastY + pawnDirection, lastX);
        // Remove the pawn (beceause we didn't move onto it).
        c.state[lastY][lastX] = 0;
        get<5>(c.lastMove) = oppColor * PAWN;
        c.lastMoveSpecial = SPECIAL_EN_PASSANT;
        all_moves.push_back( c );
      }
    } 
  } 
  return all_moves;
}

vector<Board> Board::getLegalChildren(void) {

  board_s selfColor = isWhiteTurn ? WHITE : BLACK;
  board_s selfKing = selfColor * KING;
  // A guess where king will be (or nearby).
  board_s kingY = -1, kingX;

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
  vector<Board> filtered;
  auto test = all_moves.begin();
  int testCount = 0;
  for (; test != all_moves.end(); test++) {
    //cout << "hi : " << testCount++ << " rem: " << all_moves.size() << endl;

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
    if (test->checkAttack(isWhiteTurn, testY, testX) != 0) {
      //cout << (int) testY << ", " << (int) testX << "  Not allowed " << 
      //  (int) test->checkAttack(isWhiteTurn, testX, testY) << endl;
      //test->printBoard();
      //all_moves.erase(test);
      //test--;
      continue;
    }

    //cout << "allowed this move" << endl;
    //test->printBoard();
    filtered.push_back(*test);
  }

  // TODO scary that "R    RK " was marked NOT under attack.

  //cout << "input " << all_moves.size() << " length " << endl;
  //cout << "returned " << filtered.size() << " length " << endl;

  return filtered;
}

// inline
void Board::promoHelper(
  vector<Board> *all_moves,
  bool isWhiteTurn, 
  board_s selfColor,
  board_s pawnDirection,
  board_s x,
  board_s y,
  board_s x2) {
  if ((isWhiteTurn && y == 7) || (!isWhiteTurn && y == 1)) {
    // promotion && underpromotion
    for (board_s newPiece = KNIGHT; newPiece <= QUEEN; newPiece++) {
      // "promote" then move piece (TODO how does this affect history?)
      Board c = copy();
      c.state[y][x] = selfColor * newPiece;
      c.makeMove(y,x,    y + pawnDirection, x2);
      c.lastMoveSpecial = SPECIAL_PROMOTION;
      all_moves->push_back(c);
    }
  } else {
    // Normal move to square.
    Board c = copy();
    c.makeMove(y,x,    y + pawnDirection, x2);
    all_moves->push_back(c);
  } 
}        


// TODO a duplicate version that returns location of attack, count of attack
board_s Board::checkAttack(bool forWhite, board_s a, board_s b) {
  // TODO consider pre calculating for each square in getChildrenMove of parent.
  // returns piece or 0 for no

  board_s selfColor = forWhite ? WHITE : BLACK;
  board_s oppKnight = forWhite ? -KNIGHT: KNIGHT;
  board_s selfPawnDirection = forWhite ? 1 : -1;

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

// TODO inline
bool Board::onBoard(board_s a, board_s b) {
  return 0 <= a && a <= 7 && 0 <= b && b <= 7;
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

// TODO inline?
board_s Board::getPiece(board_s a, board_s b) {
  // TODO test using &&
  return onBoard(a, b) ? state[a][b] : 0;
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
  lastMove = make_tuple(a, b, c, d, moving, removed);
  lastMoveSpecial = 0;

  // update castling.
  if (isWhite) {
    // if king moves (king can't be captured so if it leaves square it moves)
    if (a == 0 && b == 4 && moving == KING) { castleStatus &= 3; } // whiteOO = 0, whiteOOO = 0

    // rook moving back doesn't count.
    if (c == 0 && d == 0 && moving == ROOK) { castleStatus &= 7; } // whiteOO = 0
    if (c == 0 && d == 7 && moving == ROOK) { castleStatus &= 11; } // whiteOOO =0
  } else { 
    // if king moves (king can't be captured so if it leaves square it moves)
    if (a == 7 && b == 4 && moving == -KING) { castleStatus &= 12; } // blackOO = 0, blackOOO = 0

    // rook moving back doesn't count.
    if (c == 7 && d == 0 && moving == -ROOK) { castleStatus &= 13; } // blackOO = 0
    if (c == 7 && d == 7 && moving == -ROOK) { castleStatus &= 14; } // blackOOO = 0;
  }
}

bool Board::isWhitePiece(board_s piece) {
  return piece > 0;
}

board_s Board::peaceSign(board_s piece) {
  // TODO is this in cmath?
  if (piece == 0) {
    return 0;
  }
  return  (piece > 0) ? WHITE : BLACK;
}

string Board::squareNamePair(move_t move) {
  assert(false);
  //return "abcdefgh"[yx[1]] + str(yx[0] + 1)
  return "a4";
}

double Board::heuristic() {
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

  int pieceValue = 0;
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      board_s piece = state[r][c];
      if (piece != 0) {
        // TODO(seth) to be made faster later.
        pieceValue += peaceSign(piece) * PIECE_VALUE.at(abs(piece));
      }
    }
  }

  if (pieceValue > 100) {
    // black is missing king.
    mateStatus = 1;
  } else if (pieceValue < -100) {
    mateStatus = -1;
  } else {
    mateStatus = 0;
  }

  //sumValueWhite = sum(pieceValue[piece[0]] for piece in self.whitePieces)
  //sumValueBlack = sum(pieceValue[-piece[0]] for piece in self.blackPieces)

  //mobilityBonus = 0.1 * (whiteMobility - blackMobility)
  //# TODO Determine doubled, blocked, and isolated pawns

  //return sumValueWhite - sumValueBlack + mobilityBonus
  return pieceValue;
}

// -1 Black victory, 0 no mate, 1 White victory
board_s Board::mateResult(void) {
//    TODO fix this up.
//    if (mateStatus == -2) {
//      heuristic();
//    }
//    assert ( mateStatus != -2 );
//    return mateStatus;
}

void Board::perft(int ply,
    atomic<int> *count,
    atomic<int> *captures,
    atomic<int> *ep,
    atomic<int> *castles,
    atomic<int> *mates) {

  if (ply == 0) {
    move_t move = getLastMove();
    board_s moveSpecial = getLastMoveSpecial();

    count->fetch_add(1);

    if (get<5>(move) != 0) { captures->fetch_add(1); }
    if (moveSpecial == SPECIAL_EN_PASSANT) { ep->fetch_add(1); }
    if (moveSpecial == SPECIAL_CASTLE) { castles->fetch_add(1); }

    return;
  }

  vector<Board> children = getLegalChildren();
  if (children.size() == 0) { mates->fetch_add(1); }

  //#pragma omp parallel for
  for (int ci = 0; ci < children.size(); ci++) {
    children[ci].perft(ply - 1, count, captures, ep, castles, mates);
  }
}

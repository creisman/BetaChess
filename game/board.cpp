#include <atomic>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <sstream>
#include <tuple>
#include <utility>
#include <unordered_map>
#include <vector>

#include "tbb/concurrent_hash_map.h"

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

Board::Board(bool initState) {
  if (initState) {
      resetBoard();
  }
}

Board::Board(string fen) {
  // Fen is "easy" they said

  // Make list of space seperated tokens.
  stringstream ss(fen);
  istream_iterator<string> begin(ss);
  istream_iterator<string> end;
  vector<string> parts(begin, end);

  assert (parts.size() == 6);

  lastMove = make_tuple(0, 0, 0, 0, 0, 0, 0);
  memset(&state, '\0', sizeof(state));
  int y = 7;
  int x = 0;
  int fi;
  string boardPart = parts[0];
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
  assert(parts[1] == "b" || parts[1] == "w");
  isWhiteTurn = parts[1] == "w";

  // Next is castling
  castleStatus = 0;
  for (char castle : parts[2]) {
    if (castle == 'K') { castleStatus |= WHITE_OO; }; // whiteOO = true
    if (castle == 'Q') { castleStatus |= WHITE_OOO; }; // whiteOOO = true
    if (castle == 'k') { castleStatus |= BLACK_OO; }; // blackOO = true
    if (castle == 'q') { castleStatus |= BLACK_OOO; }; // blackOOO = true
  }

  // En passant "target square" (square behind the pawn)
  if (parts[3] != "-") {
    assert( (isWhiteTurn && parts[3][1] == '3') ||
             (!isWhiteTurn && parts[3][2] == '6') );

    board_s file = parts[3][0] - 'a';
    board_s rank = parts[3][1] - '0';
    board_s direction = isWhiteTurn ? 1 : -1;

    assert( onBoard(rank, file) );

    assert( abs(state[rank + direction][file]) == PAWN );
    assert( state[rank][file] == 0 );
    assert( state[rank - direction][file] == 0 );

    lastMove = make_tuple(rank - direction, file,
                          rank + direction, file,
                          direction * PAWN, 0,
                          SPECIAL_EN_PASSANT);
  };

  // Number of moves since pawn push or capture.
  halfMoves = stoul(parts[4]);

  // Number of moves into the game (FEN records number of full turns).
  // TODO verify off by one is correct.
  gameMoves = 2 * (stoul(parts[5]) - 1) + !isWhiteTurn;

  //materialDiff = getPiecesValue_slow();
  zobrist = getZobrist_slow();
}


void Board::resetBoard(void) {
  gameMoves = 0;
  halfMoves = 0;
  isWhiteTurn = true;
  //materialDiff = 0;

  castleStatus = WHITE_OO | WHITE_OOO | BLACK_OO | BLACK_OOO;

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

  //materialDiff = getPiecesValue_slow();
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

string Board::generateFen_slow(void) {
  string rep = "";
  for (int row = 7; row >= 0; row--) {
    int spaces = 0;
    for (int col = 0; col < 8; col++) {
      board_s piece = state[row][col];
      if (piece == 0) {
        spaces += 1;
        continue;
      }
      if (spaces > 0) {
        rep += to_string(spaces);
        spaces = 0;
      }

      board_s absPiece = (piece >= 0) ? piece : -piece;
      char symbol = (piece != 0) ? PIECE_SYMBOL[absPiece] : '.';
      if (piece > 0) {
        symbol = toupper(symbol);
      }
      rep += symbol;
    }
    if (spaces > 0) {
      rep += to_string(spaces);
    }
    if (row > 0) {
      rep += "/";
    }
  }
  rep += " ";

  // Who's turn it is.
  rep += isWhiteTurn ? "w" : "b";
  rep += " ";

  // Castling status.
  if (castleStatus == 0) {
    rep += "- ";
  } else {
    rep += (castleStatus & WHITE_OO) ? "K" : "";
    rep += (castleStatus & WHITE_OOO) ? "Q" : "";
    rep += (castleStatus & BLACK_OO) ? "k" : "";
    rep += (castleStatus & BLACK_OOO) ? "q" : "";
    rep += " ";
  }

  // TODO en passant.
  rep += "- ";

  // Half moves clock.
  rep += to_string(halfMoves) + " ";

  // Whole moves.
  rep += to_string(1 + gameMoves / 2);
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
          }
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

  assert(state[y][x] == selfColor * PAWN);
  if ((isWhiteTurn && y2 == 7) || (!isWhiteTurn && y2 == 0)) {
    board_s movingPawn = state[y][x];
    // promotion && underpromotion
    for (board_s newPiece = KNIGHT; newPiece <= QUEEN; newPiece++) {
      // "promote" then move piece (this means history shows Queen moving to back row not a pawn)
      Board c = copy();

      // We replace the pawn with a newPiece.
      board_s signedPiece = selfColor * newPiece;
      c.state[y][x] = signedPiece;

      //c.materialDiff += getPieceValue(signedPiece) - getPieceValue(movingPawn);
      c.updateZobristPiece(y, x, movingPawn);
      c.updateZobristPiece(y, x, signedPiece);

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
  unsigned char special = get<6>(move);

  if (special != SPECIAL_EN_PASSANT) {
    board_s capture = get<5>(move);
    assert (state[c][d] == capture);
  }

  if (special == SPECIAL_PROMOTION) {
    board_s moving  = get<4>(move);
    state[a][b] = moving;
    updateZobristPiece(a, b, peaceSign(moving) * PAWN);
    updateZobristPiece(a, b, moving);
    // This is the special magic for promotions.
  }
  makeMove(a, b, c, d, special);

  assert(getLastMove() == move);
}

void Board::makeMove(board_s a, board_s b, board_s c, board_s d, unsigned char special) {
  if (special == SPECIAL_CASTLE) {
    board_s ourKing = state[a][4];
    board_s ourRook = peaceSign(state[a][4]) * ROOK;

    if (d == 2) {
      assert( state[a][0] == ourRook );
      state[a][3] = state[a][0];
      state[a][0] = 0;

      updateZobristPiece(a, 0, ourRook);
      updateZobristPiece(a, 3, ourRook);
    } else if (d == 6) {
      assert( state[a][7] == ourRook );
      state[a][5] = state[a][7];
      state[a][7] = 0;

      updateZobristPiece(a, 5, ourRook);
      updateZobristPiece(a, 7, ourRook);
    } else {
      assert (false); // unknown destination;
    }
  }

  makeMove(a, b, c, d);
  get<6>(lastMove) = special;

  if (special == SPECIAL_EN_PASSANT) {
    board_s theirPawn = state[a][d];
    assert(abs(theirPawn) == PAWN);

    state[a][d] = 0;
    // Note that we captured a pawn.
    get<5>(lastMove) = theirPawn; 
    updateMaterialDiff(theirPawn);
    updateZobristPiece(a, d, theirPawn);
    halfMoves = 0;
  }
}

void Board::makeMove(board_s a, board_s b, board_s c, board_s d) {
  board_s moving = state[a][b];
  bool isWhiteP = isWhitePiece(moving);
  board_s removed = state[c][d];

  assert( isWhiteP == isWhiteTurn );
  if (removed != 0) {
    assert( isWhitePiece(removed) != isWhiteTurn );
  }

  state[a][b] = 0;
  state[c][d] = moving;
  updateZobristPiece(a, b, moving);
  updateZobristPiece(c, d, moving);

  // update castling.
  if (isWhiteTurn) {
    bool kingMove = (a == 0 && b == 4 && moving == KING);
    bool longRookMove  = (a == 0 && b == 0 && moving == ROOK);
    bool shortRookMove = (a == 0 && b == 7 && moving == ROOK);

    if ((kingMove || longRookMove) && (castleStatus & WHITE_OOO)) {
      castleStatus ^= WHITE_OOO;
      updateZobristCastle(WHITE_OOO);
    }
    if ((kingMove || shortRookMove) && (castleStatus & WHITE_OO)) {
      castleStatus ^= WHITE_OO;
      updateZobristCastle(WHITE_OO);
    }

    bool blackLongRookCapture  = (c == 7 && d == 0 && removed == -ROOK);
    bool blackShortRookCapture = (c == 7 && d == 7 && removed == -ROOK);
    if (blackLongRookCapture && (castleStatus & BLACK_OOO)) {
      castleStatus ^= BLACK_OOO;
      updateZobristCastle(BLACK_OOO);
    }
    if (blackShortRookCapture && (castleStatus & BLACK_OO)) {
      castleStatus ^= BLACK_OO;
      updateZobristCastle(BLACK_OO);
    }
  } else { 
    bool kingMove = (a == 7 && b == 4 && moving == -KING);
    bool longRookMove  = (a == 7 && b == 0 && moving == -ROOK);
    bool shortRookMove = (a == 7 && b == 7 && moving == -ROOK);

    if ((kingMove || longRookMove) && (castleStatus & BLACK_OOO)) {
      castleStatus ^= BLACK_OOO;
      updateZobristCastle(BLACK_OOO);
    }
    if ((kingMove || shortRookMove) && (castleStatus & BLACK_OO)) {
      castleStatus ^= BLACK_OO;
      updateZobristCastle(BLACK_OO);
    }

    bool whiteLongRookCapture  = (c == 0 && d == 0 && removed == ROOK);
    bool whiteShortRookCapture = (c == 0 && d == 7 && removed == ROOK);
    if (whiteLongRookCapture && (castleStatus & WHITE_OOO)) {
      castleStatus ^= WHITE_OOO;
      updateZobristCastle(WHITE_OOO);
    }
    if (whiteShortRookCapture && (castleStatus & WHITE_OO)) {
      castleStatus ^= WHITE_OO;
      updateZobristCastle(WHITE_OO);
    }
  }

  // update turn state
  gameMoves++;
  halfMoves++;
  isWhiteTurn = !isWhiteTurn;
  updateZobristTurn(true); //Toggle turn.

  lastMove = make_tuple(a, b, c, d, moving, removed, 0);
  if (removed != 0) {
    updateMaterialDiff(removed);
    updateZobristPiece(c, d, removed);
  }

  if (abs(moving) == PAWN || removed != 0) { 
    halfMoves = 0;
  }
}


bool Board::makeAlgebraicMove_slow(string move) {
  // This is like slow^2.
  // For each child, go another lookup over each child (to find if it's disambigous)

  for (Board c : getLegalChildren()) {
    if (algebraicNotation_slow(c.getLastMove()) == move) {
      makeMove(c.getLastMove());
      return true;
    }
  }
  return false;
}


string Board::algebraicNotation_slow(move_t child_move) {
  // e4 = white king pawn double out.
  // e is file.
  // 4 is rank.

  bool mult = false;
  bool sameFile = false;
  bool sameRank = false;

  Board child_board = copy();
  child_board.makeMove(child_move);

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
        assert(c.getZobrist() == child_board.getZobrist());
        continue;
      }
      mult = true;
      sameFile |= equalFile;
      sameRank |= equalRank;
    }
  }

  pair<board_s, board_s> oppKingPos =
      child_board.findPiece_slow(isWhiteTurn ? -KING : KING);
  assert( child_board.onBoard(get<0>(oppKingPos), get<1>(oppKingPos)) );

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

  // NOTE that + and # are omitted.
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
  //materialDiff -= getPieceValue(removed);
}

void Board::updateZobristPiece(board_s a, board_s b, board_s piece) {
  short kindOfPiece = 2 * (abs(piece) - 1)  + isWhitePiece(piece);
  short index = 64 * kindOfPiece + 8 * a + b; // a = rank, b = file
  assert (0 <= index && index < 768);
  zobrist ^= POLYGLOT_RANDOM[index];
}

void Board::updateZobristTurn(bool isWTurn) {
  zobrist ^= (isWTurn == true) * POLYGLOT_RANDOM[780];
}

void Board::updateZobristCastle(char castleStatus) {
  zobrist ^= ((castleStatus & WHITE_OO)  > 0) * POLYGLOT_RANDOM[768 + 0];
  zobrist ^= ((castleStatus & WHITE_OOO) > 0) * POLYGLOT_RANDOM[768 + 1];
  zobrist ^= ((castleStatus & BLACK_OO)  > 0) * POLYGLOT_RANDOM[768 + 2];
  zobrist ^= ((castleStatus & BLACK_OOO) > 0) * POLYGLOT_RANDOM[768 + 3];
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

  // TODO enpassant.
  updateZobristTurn(isWhiteTurn);
  updateZobristCastle(castleStatus);

  return zobrist;
}


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

typedef tbb::concurrent_hash_map<uint64_t, uint64_t> perfter;

unordered_map<uint64_t, uint64_t> perftLookup[10];
perfter lookupTBB[10];

uint64_t Board::perftMoveOnly(int ply) {
  if (ply == 0) {
    return 1;
  }

  vector<Board> children = getLegalChildren();
  uint64_t key = getZobrist();
  //auto lookup = perftLookup[ply].find(key);
  //if (lookup != perftLookup[ply].end()) {
  //  return lookup->second;
  //}

  perfter::accessor a;
  if (lookupTBB[ply].find(a, key)) {
    uint64_t count = a->second;
    return count;
  } 
  a.release();

  atomic<uint64_t> count(0);
  if (ply == 1) {
    count = children.size(); 
  } else {
    #pragma omp parallel for
    for (int ci = 0; ci < children.size(); ci++) {
      count += children[ci].perftMoveOnly(ply - 1);
    }
  }

  if (ply > 1) {
    perfter::accessor b;
    lookupTBB[ply].insert(b, key);
    b->second = count;
    b.release();
  //  perftLookup[ply][key] = count;
  }
  return count;
}


void Board::perft(
    int ply,
    atomic<long> *count,
    atomic<long> *captures,
    atomic<int> *ep,
    atomic<int> *castles,
    atomic<int> *promotions,
    atomic<int> *mates) {
  if (ply == 0) {
    move_t move = getLastMove();
    board_s moveSpecial = get<6>(move);

    count->fetch_add(1);
//    uint64_t test = zobrist;
//    assert( test == getZobrist_slow() );

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

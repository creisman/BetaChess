#include <algorithm>
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
#include <vector>

#include "board.h"
#include "flags.h"
#include "polyglot.h"
#include "pst.h"
#include "ttable.h"

using namespace board;
using namespace std;
using namespace ttable;


const string Board::PIECE_SYMBOL = "?pnbrqk";

const move_t Board::NULL_MOVE = make_tuple(0, 0, 0, 0, 0, 0, 0);

const map<board_s, movements_t> Board::MOVEMENTS = {
  {KNIGHT, {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}}},
  {BISHOP, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}},
  {ROOK, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}},
  {QUEEN, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}}},
  {KING, {{0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1, -1}}},
};


Board::Board(void) {
  resetBoard();
}


Board::Board(string fen) {
  // Fen is "easy" they said

  // Make list of space seperated tokens.
  stringstream ss(fen);
  istream_iterator<string> begin(ss);
  istream_iterator<string> end;
  vector<string> parts(begin, end);
  assert (parts.size() == 6);

  lastMove = Board::NULL_MOVE;
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

  // Special don't validate settings.
  material = 0;
  totalMaterial = 0;
  position = 0;
  zobrist = 0;
  recalculateEvaluations_slow();
  recalculateZobrist_slow();
}


void Board::resetBoard(void) {
  gameMoves = 0;
  halfMoves = 0;
  isWhiteTurn = true;

  castleStatus = WHITE_OO | WHITE_OOO | BLACK_OO | BLACK_OOO;

  memset(&state, '\0', sizeof(state));

  lastMove = Board::NULL_MOVE;

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

  // Special don't validate settings.
  material = 0;
  totalMaterial = 0;
  position = 0;
  zobrist = 0;
  recalculateEvaluations_slow();
  recalculateZobrist_slow();
}


Board Board::copy() const {
  // Call "copy" constructor.
  Board copy = *this;
  return copy;
}


string Board::boardStr(void) const {
  string rep = "";
  for (int row = 7; row >= 0; row--) {
    rep += "|";
    for (int col = 0; col < 8; col++) {
      board_s piece = state[row][col];
      board_s absPiece = (piece >= 0) ? piece : -piece;
      assert( 0 <= absPiece && absPiece <= 6);
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



string Board::generateFen_slow(void) const {
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


void Board::printBoard(void) const {
  cout << boardStr() << endl;
}


bool Board::getIsWhiteTurn(void) const {
  return isWhiteTurn;
}


board_hash_t Board::getZobrist(void) const {
  return zobrist;
}


move_t Board::getLastMove(void) const {
  return lastMove;
}


vector<Board> Board::getChildrenInternal_slow(void) const {
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
            promoHelper(&all_moves, selfColor, x, y, x + lr, y + pawnDirection);
            hasCapture = true;
          }
        }

        moveTest = attemptMove(y + pawnDirection, x);
        // pawn move: if next space is empty.
        if (moveTest.first && moveTest.second == 0) {
          // Normal move forward && promo
          promoHelper(&all_moves, selfColor, x, y, x, y + pawnDirection);

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


vector<Board> Board::getLegalChildren(void) const {
  vector<Board> all_moves = getChildrenInternal_slow();
  if (all_moves.size() == 0) {
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


void Board::promoHelper(
  vector<Board> *all_moves,
  board_s selfColor,
  board_s x,
  board_s y,
  board_s x2,
  board_s y2) const {

  assert(state[y][x] == selfColor * PAWN);
  if ((isWhiteTurn && y2 == 7) || (!isWhiteTurn && y2 == 0)) {
    board_s movingPawn = state[y][x];
    // promotion && underpromotion
    board_s lastPromoPiece = QUEEN;
    for (board_s newPiece = lastPromoPiece; newPiece >= KNIGHT; newPiece--) {
      // "promote" then move piece (this means history shows Queen moving to back row not a pawn)
      Board c = copy();

      // We replace the pawn with a newPiece.
      board_s signedPiece = selfColor * newPiece;
      c.state[y][x] = signedPiece;

      c.updatePiece(y, x, movingPawn, false /* movingTo */);
      c.updatePiece(y, x, signedPiece, true /* movingTo */);

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


board_s Board::checkAttack_medium(bool byBlack, board_s a, board_s b) const {
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


pair<bool, board_s> Board::attemptMove(board_s a, board_s b) const {
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


board_s Board::getPiece(board_s a, board_s b) const {
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
    board_s disappearingPawn = peaceSign(moving) * PAWN;
    assert(state[a][b] ==  disappearingPawn);
    state[a][b] = moving;

    updatePiece(a, b, disappearingPawn, false /* movingTo */);
    updatePiece(a, b, moving, true /* movingTo */);
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

      updatePiece(a, 0, ourRook, false /* movingTo */);
      updatePiece(a, 3, ourRook, true /* movingTo */);
    } else if (d == 6) {
      assert( state[a][7] == ourRook );
      state[a][5] = state[a][7];
      state[a][7] = 0;

      updatePiece(a, 7, ourRook, false /* movingTo */);
      updatePiece(a, 5, ourRook, true /* movingTo */);
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
    updatePiece(a, d, theirPawn, false /* movingTo */);
    halfMoves = 0;
  }
}


void Board::makeMove(board_s a, board_s b, board_s c, board_s d) {
  board_s moving = state[a][b];
  board_s removed = state[c][d];

  assert( moving != 0 );

  assert( isWhitePiece(moving) == isWhiteTurn );
  if (removed != 0) {
    assert( isWhitePiece(removed) != isWhiteTurn );
  }

  state[a][b] = 0;
  state[c][d] = moving;

  updatePiece(a, b, moving, false /* movingTo */);
  updatePiece(c, d, moving, true /* movingTo */);

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
  updateZobristEnPassant(lastMove); // Toggle off last move.

  lastMove = make_tuple(a, b, c, d, moving, removed, 0);
  updateZobristEnPassant(lastMove); // Toggle on this move.

  if (removed != 0) {
    updatePiece(c, d, removed, false /* movingTo */);
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


string Board::algebraicNotation_slow(move_t child_move) const {
  // e4 = white king pawn double out.
  // e is file.
  // 4 is rank.

  if (child_move == NULL_MOVE) {
    return "NULL_MOVE";
  }

  bool mult = false;
  bool sameFile = false;
  bool sameRank = false;

  Board child_board = copy();
  child_board.makeMove(child_move);

  // NOTE(seth): It appears disambigous is only looking at "valid" moves.
  vector<Board> children = getLegalChildren();
  for (Board c : children) {
    // same piece, same destination, same special.
    if ((get<2>(child_move) == get<2>(c.lastMove)) &&
        (get<3>(child_move) == get<3>(c.lastMove)) &&
        (get<4>(child_move) == get<4>(c.lastMove)) &&
        (get<6>(child_move) == get<6>(c.lastMove))) {
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


  bool isCheck, isMate;
  pair<board_s, board_s> oppKingPos =
      child_board.findPiece_slow(isWhiteTurn ? -KING : KING);
  assert( child_board.onBoard(get<0>(oppKingPos), get<1>(oppKingPos)) );

  // Assume We are currently white.
  // After our move check if blackKing is under attack by white (not byBlack).
  isCheck = child_board.checkAttack_medium(
      !isWhiteTurn /* byBlack */,
      get<0>(oppKingPos),
      get<1>(oppKingPos)) != 0;

  // Note assumes self move can't result in mate.
  isMate = isCheck &&
      ((isWhiteTurn ? RESULT_WHITE_WIN : RESULT_BLACK_WIN)
          == child_board.getGameResult_slow());

  string check = isMate ? "#" : (isCheck ? "+" : "");

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
    if (capture.size() > 0) {
      return fileName(get<1>(child_move)) + capture + dest +  "=" + pieceName + check;
    }
    return dest + "=" + pieceName + check;
  }
  if (special == SPECIAL_CASTLE) {
    return ((get<3>(child_move) == 2) ? "O-O-O" : "O-O") + check;
  }

  // Pawn captures get file added.
  if (piece == PAWN && capture.size() > 0) {
    // A special (generic) case of disambiguate.
    // Can't be disambigous once we know file.
    pieceName = fileName(get<1>(child_move));
    return pieceName + capture + dest + check;
  }

  return pieceName + disambiguate + capture + dest + check;
}


string Board::lastMoveName_slow(void) const {
  return coordinateNotation(lastMove);
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


int Board::getPieceValue(board_s piece) {
  assert(piece != 0);
  board_s absPiece = abs(piece);
  assert(0 <= absPiece && absPiece <= 6);
  return peaceSign(piece) * PST_PIECE_VALUE[absPiece];
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


int Board::getPSTValue(board_s a, board_s b, board_s piece) const {
  char index;
  if (peaceSign(piece) == WHITE) {
    index = 8 * a     + b; // a = rank, b = file
  } else {
    // Rotate the board for black (not just rank (y) mirror).
    index = 8 * (7-a) + (7-b); // a = rank, b = file
  }
  assert (0 <= index && index < 64);

  board_s absPiece = abs(piece);
  int signMult = peaceSign(piece);

  // TODO optimize game phase / materialRatio somehow.
  //float materialRatio = (totalMaterial - 2 * getPieceValue(KING)) / PST_PIECE_VALUE_SUM;
  //float materialRatio = 0.5;
  //assert (0 <= materialRatio && materialRatio <= 1.0);

  int val;
  if        (absPiece == PAWN) {
    val = PST_PAWN_MG[index];

  } else if (absPiece == KNIGHT) {
    val = PST_KNIGHT_MG[index];

  } else if (absPiece == BISHOP) {
    val = PST_BISHOP_MG[index];

  } else if (absPiece == ROOK) {
    val = PST_ROOK_MG[index];

  } else if (absPiece == QUEEN) {
    val = PST_QUEEN_MG[index];

  } else if (absPiece == KING) {
    val = PST_KING_MG[index];

  } else {
    assert( false );
  }

  return signMult * val;
}


void Board::updatePiece(board_s a, board_s b, board_s piece, bool movingTo) {
  assert(piece != 0);

  int mult = (movingTo ? 1 : -1);
  int value = getPieceValue(piece);

  material += mult * value;
  totalMaterial += mult * abs(value);

  int pst = getPSTValue(a, b, piece);
  position += mult * pst;

  updateZobristPiece(a, b, piece);
}


void Board::recalculateEvaluations_slow(void) {
  int oldMaterial = material;
  int oldTotalMaterial = totalMaterial;
  int oldPosition = position;
  board_hash_t oldZobrist = zobrist;

  material = 0;
  totalMaterial = 0;
  position = 0;
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      board_s piece = state[r][c];
      if (piece != 0) {
        updatePiece(r, c, piece, true /* movingTo */);

        // Note: have to undo updateZobrist.
        updateZobristPiece(r, c, piece);
      }
    }
  }

  assert( oldMaterial == 0      | oldMaterial == material           );
  assert( oldTotalMaterial == 0 | oldTotalMaterial == totalMaterial );
  assert( oldPosition == 0      | oldPosition == position           );
  assert( oldZobrist == zobrist );
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


void Board::updateZobristEnPassant(move_t &move) {
  // We don't follow the Polyglot standard and choose to always include the
  // enpassant after a double pawn push.
  if (abs(get<4>(move)) == PAWN && abs(get<0>(move) - get<2>(move)) == 2) {
    zobrist ^= POLYGLOT_RANDOM[772 + get<1>(move)];
  }
}


void Board::recalculateZobrist_slow(void) {
  board_hash_t oldZobrist = zobrist;

  zobrist = 0;
  for (int r = 0; r < 8; r++) {
    for (int f = 0; f < 8; f++) {
      board_s piece = state[r][f];
      if (piece != 0) {
        updateZobristPiece(r, f, piece);
      }
    }
  }

  updateZobristTurn(isWhiteTurn);
  updateZobristCastle(castleStatus);
  updateZobristEnPassant(lastMove);

  assert( oldZobrist == 0 || zobrist == oldZobrist );
}


int Board::heuristic() const {
  // NOTE: Used to test that updatePiece is doing incremental updates correctly.
  // tuple<short, short> marco = make_tuple(material, position);
  // getPiecesValue_slow();
  // tuple<short, short> polo = make_tuple(material, position);
  //assert( marco == polo );

  // TODO testing ttable.
  int evaluation = material + position;
  return evaluation;
}


pair<board_s, board_s> Board::findPiece_slow(board_s piece) const {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (state[y][x] == piece) {
        return make_pair(y, x);
      }
    }
  }
  return make_pair(-1, -1);
}


board_s Board::getGameResult_slow(void) const {
  // TODO: Add 50 move rule and repeated position.
  // TODO: cache gameresult from heuristic and here.

  pair<board_s, board_s> kingPos =
      findPiece_slow(isWhiteTurn ? KING : -KING);
  assert( onBoard(get<0>(kingPos), get<1>(kingPos)) );

  bool inCheck = checkAttack_medium(
      isWhiteTurn /* byBlack */,
      get<0>(kingPos),
      get<1>(kingPos)) != 0;

  // This could be improved (maybe making this _medium) by instead checking move_exists?
  vector<Board> children = getLegalChildren();
  bool hasChildren = !children.empty();

  if (hasChildren) {
      // TODO check for draw conditions (insufficent material, ...)
      return RESULT_IN_PROGRESS;
  }

  // Loss conditions: no moves + in check.
  if (!hasChildren && inCheck) {
    return isWhiteTurn ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
  }

  // Tie (stalemate) conditions: no moves + not in check.
  if (!hasChildren && !inCheck) {
    return RESULT_TIE;
  }

  assert(false);
}

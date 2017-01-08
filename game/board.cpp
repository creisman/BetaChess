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

Board::Board(bool initState) {
  if (initState) {
      resetBoard();
  }
}


Board Board::copy() {
  Board copy = Board(false);
  copy.setState(ply, state);
  return copy;
}


void Board::setState(int plyP, board_t stateP) {
    ply = plyP;
    isWhiteTurn = (plyP % 2) == 0;
    memcpy(&state, &stateP, sizeof(state));

    // Do I need to copy of hasKing status?
}


void Board::resetBoard(void) {
  ply = 0;
  isWhiteTurn = true;
  mateStatus = 0;

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
  string rep = "";
  for (int row = 7; row >= 0; row--) {
    for (int col = 0; col < 8; col++) {
      board_s piece = state[row][col];
      board_s absPiece = (piece >= 0) ? piece : -piece;
      char symbol = (piece != 0) ? PIECE_SYMBOL[absPiece] : '.';
      if (piece > 0) {
        symbol = toupper(symbol);
      }
      rep += symbol;
      } 
    cout << endl;
    rep += "\n";
  }

  return rep;
}

void Board::printBoard(void) {
  cout << boardStr() << endl;
}


vector<pair<move_t, Board>> Board::getChildren(void) {
  vector<pair<move_t, Board>> all_moves;
 
  // Because we are doing anti-chess first.
  vector<pair<move_t, Board>> non_captures;
  vector<pair<move_t, Board>> captures;

  board_s pawnDirection = isWhiteTurn ? 1 : -1;
  board_s oppColor = isWhiteTurn ? BLACK : WHITE;

  pair<bool, board_s> moveTest;
  move_t move;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      board_s piece = state[y][x];
      if (isWhiteTurn != isWhitePiece(piece)) {
        continue;
      }

      // TODO CASTLING, CHECKS (CHECKMATES)

      if (piece == PAWN || piece == -PAWN) {
        // TODO: enpassant, and promotion

        moveTest = attemptMove(y + pawnDirection, x);
        // if next space is empty.
        if (captures.size() == 0 && moveTest.first && moveTest.second == 0) {
          Board c = copy();
          move = c.makeMove(y,x,    y + pawnDirection, x);
          //non_captures.push_back( make_pair(move, c) )
          all_moves.push_back( make_pair(move, c) );

          // double move (only if nothing in the way for single move)
          if ((isWhiteTurn && y == 1) || (!isWhiteTurn && y == 6)) {
            moveTest = attemptMove(y + 2 * pawnDirection, x);
            if (moveTest.first && moveTest.second == 0) {
              Board c = copy();
              move = c.makeMove(y,x,    y + 2 * pawnDirection, x);
              //non_captures.append( (move, c) )
              all_moves.push_back( make_pair(move, c) );
            }
          }
        }

        // pawn capture
        for (board_s lr = -1; lr <= 1; lr += 2) {
          moveTest = attemptMove(y + pawnDirection, x + lr);
          if (moveTest.first && moveTest.second == oppColor) {
            Board c = copy();
            move = c.makeMove(y,x,    y + pawnDirection, x + lr);
            all_moves.push_back( make_pair(move, c) );
          }
        }
      } /*else if (abs(piece) == KNIGHT || abs(piece) == KING) {
        // "jumpy" pieces = KNIGHT, KING
        for deltaY, deltaX in Board.MOVEMENTS[pieceType]:
          onBoard, destColor = self.attemptMove(y + deltaY, x + deltaX)
          if onBoard and destColor != color:
            if len(captures) > 0 and destColor == None:
              continue

            c = self.copy()
            move = c.makeMove(y,x,   y + deltaY, x + deltaX)

            if destColor == None:
              non_captures.append( (move, c) )
            else
              captures.append( (move, c) )
      } else {
        // slidy pieces = BISHOPS, ROOKS, QUEENS
        for deltaY, deltaX in Board.MOVEMENTS[pieceType]:
          newY = y
          newX = x
          while True:
            newY += deltaY
            newX += deltaX
            onBoard, destColor = self.attemptMove(newY, newX)
            if not onBoard or destColor == color:
              break

            c = self.copy()
            move = c.makeMove(y, x,   newY, newX)
            yield (move, c)
      }
    */
    }
  }
  return all_moves;
}


pair<bool, board_s> Board::attemptMove(board_s a, board_s b) {
  // prep for moving piece to state[a][b]
  // returns on board, piece on [a][b]

  if (0 <= a <= 7 && 0 <= b <= 7) {
    board_s destPiece = state[a][b];
    if (destPiece) {
      return make_pair(true, peaceSign(destPiece));
    }
    return make_pair(true, 0);
  }
  return make_pair(false, 0);
}


move_t Board::makeMove(board_s a, board_s b, board_s c, board_s d) {
  board_s moving = state[a][b];
  bool isWhite = isWhitePiece(moving);
  board_s removed = state[c][d];

  // TODO figure out asserts.
  //assert(isWhite == isWhiteTurn );
  //if (removed) {
  //  assert( isWhitePiece(remove) != isWhiteTurn, "test" );
  //}

  state[a][b] = 0;
  state[c][d] = moving;

  // update turn state
  ply++;
  isWhiteTurn = !isWhiteTurn;

  return make_tuple(a, b, c, d, moving, removed);
}

bool Board::isWhitePiece(board_s piece) {
  return piece > 0;
}

board_s Board::peaceSign(board_s piece) {
  // TODO is this in cmath?
  if (piece == 0) {
    return 0;
  }
  return  (piece > 0) ? 1 : -1;
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
  /*
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      board_s piece = state[r][c];
      if (piece) {
        // TODO(seth) to be made faster later.
        pieceValue += peaceSign(piece) * PIECE_VALUE.at(piece);
      }
    }
  }
  */

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
    if (mateStatus == -2) {
      heuristic();
    }
    assert ( mateStatus != -2 );
    return mateStatus;
}

void Board::perft(int depth, int* count, int* captures, int* mates) {
  if (depth == 0) {
    *count += 1;

    if (mateResult() != 0) {
      *mates += 1;
    }
    return;
  }

  vector<pair<move_t, Board>> children = getChildren();
  for (auto c = children.begin(); c != children.end(); c++) {
    c->second.perft(depth - 1, count, captures, mates);
  }
}

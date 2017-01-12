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
  /*
  whiteOO = whiteOOO = false;
  blackOO = blackOOO = false;
  while (fen[fi] != ' ') {
    if (fen[fi] == 'K') { whiteOO = true; }
    if (fen[fi] == 'Q') { whiteOOO = true; }
    if (fen[fi] == 'k') { blackOO = true; }
    if (fen[fi] == 'q') { blackOOO = true; }
    fi++;
  }  
  */
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


vector<pair<move_t, Board>> Board::getChildren(void) {
  vector<pair<move_t, Board>> all_moves;
 
  // Because we are doing anti-chess first.
  //vector<pair<move_t, Board>> non_captures;
  //vector<pair<move_t, Board>> captures;

  board_s pawnDirection = isWhiteTurn ? 1 : -1;
  board_s selfColor = isWhiteTurn ? WHITE : BLACK;
  board_s oppColor = isWhiteTurn ? BLACK : WHITE;

  pair<bool, board_s> moveTest;
  move_t move;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      board_s piece = state[y][x];
      if (piece == 0 || isWhiteTurn != isWhitePiece(piece)) {
        continue;
      }

      // TODO CASTLING, CHECKS (CHECKMATES)

      if (abs(piece) == PAWN) {
        // pawn capture
        for (board_s lr = -1; lr <= 1; lr += 2) {
          moveTest = attemptMove(y + pawnDirection, x + lr);
          if (moveTest.first && moveTest.second == oppColor) {
            Board c = copy();
            move = c.makeMove(y,x,    y + pawnDirection, x + lr);
            all_moves.push_back( make_pair(move, c) );
          }
        }

        // TODO: enpassant, and promotion

        moveTest = attemptMove(y + pawnDirection, x);
        // pawn move: if next space is empty.
        if (moveTest.first && moveTest.second == 0) {
          // Antichess
          //if (captures.size() > 0) { continue; }

          Board c = copy();
          move = c.makeMove(y,x,    y + pawnDirection, x);
          all_moves.push_back( make_pair(move, c) );

          // double move (only if nothing in the way for single move)
          if ((isWhiteTurn && y == 1) || (!isWhiteTurn && y == 6)) {
            moveTest = attemptMove(y + 2 * pawnDirection, x);
            if (moveTest.first && moveTest.second == 0) {
              Board c = copy();
              move = c.makeMove(y,x,    y + 2 * pawnDirection, x);
              all_moves.push_back( make_pair(move, c) );
            }
          }
        }
      } else if (abs(piece) == KNIGHT || abs(piece) == KING) {
        // "jumpy" pieces = KNIGHT, KING
        for (auto iter = MOVEMENTS.at(abs(piece)).begin();
                  iter != MOVEMENTS.at(abs(piece)).end();
                  iter++) {
          moveTest = attemptMove(y + iter->first, x + iter->second);
          if (moveTest.first && moveTest.second != selfColor) {
            // AntiChess 
            //if (captures.size() > 0) { continue; }

            Board c = copy();
            move = c.makeMove(y,x,   y + iter->first, x + iter->second);
            all_moves.push_back( make_pair(move, c) );

            // AntiChess
            //if (destColor == oppColor) {
            //  non_captures.append( (move, c) )
            //else
            //  captures.append( (move, c) )
          }
        }
      } else {
        // slidy pieces = BISHOPS, ROOKS, QUEENS
        assert( abs(piece) == BISHOP || abs(piece) == ROOK || abs(piece) == QUEEN );
        for (auto iter = MOVEMENTS.at(abs(piece)).begin();
                  iter != MOVEMENTS.at(abs(piece)).end();
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

            // TODO antiChess 

            Board c = copy();
            move = c.makeMove(y, x,   newY, newX);
            all_moves.push_back( make_pair(move, c) );

            if (moveTest.second == oppColor) {
              break;
            }
          }
        }
      }
    }
  }
  return all_moves;
}


pair<bool, board_s> Board::attemptMove(board_s a, board_s b) {
  // prep for moving piece to state[a][b]
  // returns on board, piece on [a][b]

  if (0 <= a && a <= 7 && 0 <= b && b <= 7) {
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

  assert( isWhite == isWhiteTurn );
  if (removed != 0) {
    assert( isWhitePiece(removed) != isWhiteTurn );
  }

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
    if (mateStatus == -2) {
      heuristic();
    }
    assert ( mateStatus != -2 );
    return mateStatus;
}

void Board::perft(int ply, atomic<int> *count, atomic<int> *captures, atomic<int> *mates) {
  if (ply == 0) {
    count->fetch_add(1);

    //printBoard();

    if (mateResult() != 0) {
      mates->fetch_add(1);
    }
    return;
  }

  vector<pair<move_t, Board>> children = getChildren();
  //#pragma omp parallel for
  for (int ci = 0; ci < children.size(); ci++) {
    if (get<5>(children[ci].first) != 0) {
      captures->fetch_add(1);
    }

    children[ci].second.perft(ply - 1, count, captures, mates);
  }
}

#ifndef BOARD_H
#define BOARD_H

#include <atomic>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

#define BOARD_SIZE 8

namespace board {
  // NOTE(seth): It appears that my RPi assumes unsigned char by default so call it out specifically.
  typedef signed char board_s;

  // (a, b) to (c, d), piece that moving, piece that was captured removing
  typedef tuple<board_s, board_s, board_s, board_s, board_s, board_s> move_t;
  typedef vector<pair<board_s, board_s>> movements_t;
  typedef board_s board_t[BOARD_SIZE][BOARD_SIZE];

  class Board {
    public:
      static const board_s WHITE = 1;
      static const board_s BLACK = -1;

      static const board_s PAWN = 1;
      static const board_s KNIGHT = 2;
      static const board_s BISHOP = 3;
      static const board_s ROOK = 4;
      static const board_s QUEEN = 5;
      static const board_s KING = 6;

      static const string PIECE_SYMBOL;
      static const map<board_s, movements_t> MOVEMENTS;
      static const map<board_s, int> PIECE_VALUE;

      // Constructor (including the hidden copy constructor).
      Board(bool initState);
      Board(string fen);

      Board copy(void);

      void resetBoard(void);

      vector<pair<move_t, Board>> getChildren(void);

      double heuristic(void);
      board_s mateResult(void);

      string boardStr(void);
      void printBoard(void);

      void perft(int ply, atomic<int> *count,
          atomic<int> *captures, atomic<int> *castles, atomic<int> *mates);

    private:
      void promoHelper(vector<pair<move_t, Board>> *all_moves,
          bool isWhiteTurn, board_s selfColor, board_s pawnDirection, board_s x, board_s y, board_s x2);
      board_s checkAttack(board_s a, board_s b);
      board_s getPiece(board_s a, board_s b);
      pair<bool, board_s> attemptMove(board_s a, board_s b);
      move_t makeMove(board_s a, board_s b, board_s c, board_s d);

      static bool isWhitePiece(board_s piece);
      static board_s peaceSign(board_s piece);

      // TODO investigate usage of this.
      static string squareNamePair(move_t move);

      // 2 + 1 + 64 + 1   +   4 + 4  = 76 bytes.
      short ply;
      bool isWhiteTurn;
      board_t state;
      board_s mateStatus;

      move_t lastMove;
      bool whiteOO;
      bool whiteOOO;
      bool blackOO;
      bool blackOOO;
  };
}
#endif // BOARD_H

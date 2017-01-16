#ifndef BOARD_H
#define BOARD_H

#include <atomic>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

#define BOARD_SIZE 8
#define IS_ANTICHESS false

namespace board {
  // NOTE(seth): It appears that my RPi assumes unsigned char by default so call it out specifically.
  typedef signed char board_s;

  // (a, b) to (c, d), piece that moving, piece that was captured removing
  typedef tuple<board_s, board_s, board_s, board_s, board_s, board_s> move_t;

  // score concatonated to end of move_t
  typedef pair<double, move_t> scored_move_t;

  typedef vector<pair<board_s, board_s>> movements_t;

  typedef board_s board_t[BOARD_SIZE][BOARD_SIZE];


  // Board Class
  class Board {
    public:
      // NOTE(sethtroisi): Converting to constexpr slows -O2 by ~10%.
      // NOTE(sethtroisi): Requires -O2 to compile (maybe move to cpp?
      static const board_s WHITE = 1;
      static const board_s BLACK = -1;

      // Pieces.
      static const board_s PAWN = 1;
      static const board_s KNIGHT = 2;
      static const board_s BISHOP = 3;
      static const board_s ROOK = 4;
      static const board_s QUEEN = 5;
      static const board_s KING = 6;

      // Castle status.
      static const board_s WHITE_OO = 8;
      static const board_s WHITE_OOO = 4;
      static const board_s BLACK_OO = 2;
      static const board_s BLACK_OOO = 1;

      // Last Move Special
      static const board_s SPECIAL_CASTLE = 1;
      static const board_s SPECIAL_EN_PASSANT = 2;
      static const board_s SPECIAL_PROMOTION = 3;


      static const string PIECE_SYMBOL;
      static const map<board_s, movements_t> MOVEMENTS;
      static const map<board_s, int> PIECE_VALUE;
      static const map<board_s, int> ANTICHESS_PIECE_VALUE;

      // Constructor (including the hidden copy constructor).
      Board(bool initState);
      Board(string fen);

      Board copy(void);

      void resetBoard(void);

      move_t getLastMove(void);
      board_s getLastMoveSpecial(void);
      vector<Board> getChildren(void);
      vector<Board> getLegalChildren(void);

      double heuristic(void);

      string boardStr(void);
      void printBoard(void);

      scored_move_t findMove(int ply);
      void makeMove(board_s a, board_s b, board_s c, board_s d);

      void perft(int ply,
          atomic<int> *count,
          atomic<int> *captures,
          atomic<int> *ep,
          atomic<int> *castles,
          atomic<int> *promotions,
          atomic<int> *mates);

      // Algebraic notation of legal move from this board.
      string algebraicNotation(move_t child_move, board_s child_move_special);

      // ~Coordinate notation.
      static string coordinateNotation(move_t child_move);

      // NOTE(seth): a is y 0-7, b is x 0-7
      static string squareName(board_s a, board_s b);
      static string rankName(board_s a);
      static string fileName(board_s b);

    private:
      void promoHelper(vector<Board> *all_moves,
          bool isWhiteTurn, board_s selfColor, board_s pawnDirection, board_s x, board_s y, board_s x2);

      // 1-arg version is public.
      scored_move_t findMove(int ply, double alpha, double beta);

      board_s checkAttack(bool fromWhite, board_s a, board_s b);
      board_s getPiece(board_s a, board_s b);
      pair<bool, board_s> attemptMove(board_s a, board_s b);

      // Helper methods.
      static bool isWhitePiece(board_s piece);
      static board_s peaceSign(board_s piece);
      static bool onBoard(board_s a, board_s b);

      // Used for counting moves in findMove
      static int dbgCounter;


      // 2 + 1 + 64 + 1   +   6+1  = 76 bytes.
      short ply;
      bool isWhiteTurn;
      board_t state;
      board_s mateStatus;

      move_t lastMove;
      char lastMoveSpecial; // used for castle, en passant, check, checkmate.

      // whiteOO, whiteOOO, blackOO, blackOOO
      char castleStatus;
  };
}
#endif // BOARD_H

#ifndef BOARD_H
#define BOARD_H

#include <atomic>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "flags.h"

using namespace std;


namespace board {
  // NOTE(seth): It appears that my RPi assumes unsigned char by default so call it out specifically.
  typedef signed char board_s;

  typedef uint64_t board_hash_t;

  // (a, b) to (c, d), piece that moving, piece that was captured removing, special_status
  typedef tuple<board_s, board_s, board_s, board_s, board_s, board_s, unsigned char> move_t;

  struct FindMoveStats {
    int plyR;
    int nodes;
  };

  // score concatonated to end of move_t
  typedef pair<int, move_t> scored_move_t;

  typedef vector<pair<board_s, board_s>> movements_t;

  typedef board_s board_t[8][8];


  // Board Class
  class Board {
    public:
      // NOTE(sethtroisi): Converting to constexpr slows -O2 by ~10%.
      // NOTE(sethtroisi): Requires -O2 to compile (maybe move to cpp?
      static const board_s WHITE = 1;
      static const board_s BLACK = -1;

      // Pieces
      static const board_s PAWN = 1;
      static const board_s KNIGHT = 2;
      static const board_s BISHOP = 3;
      static const board_s ROOK = 4;
      static const board_s QUEEN = 5;
      static const board_s KING = 6;

      // Castle status
      static const board_s WHITE_OO = 8;
      static const board_s WHITE_OOO = 4;
      static const board_s BLACK_OO = 2;
      static const board_s BLACK_OOO = 1;

      // Last Move Special
      static const unsigned char SPECIAL_CASTLE = 1;
      static const unsigned char SPECIAL_EN_PASSANT = 2;
      static const unsigned char SPECIAL_PROMOTION = 3;

      // Game result
      static const board_s RESULT_IN_PROGRESS = 100;
      static const board_s RESULT_TIE         = 101;
      static const board_s RESULT_WHITE_WIN   = 102;
      static const board_s RESULT_BLACK_WIN   = 103;

      // Game status
      static const board_s STATUS_IS_CHECK  = 10;
      static const board_s STATUS_NOT_CHECK = 11;

      static const move_t NULL_MOVE;
      static const string PIECE_SYMBOL;
      static const map<board_s, movements_t> MOVEMENTS;

      // Constructors
      Board(void);
      Board(string fen);

      void resetBoard(void);

      Board copy(void) const;

      string boardStr(void) const;
      string generateFen_slow(void) const;
      void printBoard(void) const;

      bool getIsWhiteTurn(void) const;
      board_hash_t getZobrist(void) const;

      move_t getLastMove(void) const;
      vector<Board> getLegalChildren(void) const;

      void makeMove(move_t move);
      bool makeAlgebraicMove_slow(string move);

      // Algebraic notation of legal move from this board.
      string algebraicNotation_slow(move_t child_move) const;
      string lastMoveName_slow() const;

      // ~Coordinate notation.
      static string coordinateNotation(move_t move);

      // NOTE(seth): a is y (0-7), b is x (0-7)
      static string squareName(board_s a, board_s b);
      static string rankName(board_s a);
      static string fileName(board_s b);

      // VisibileForTesting (safe to call but slow and silly).
      void recalculateEvaluations_slow(void);
      void recalculateZobrist_slow(void);

      int heuristic(void) const;

      // see RESULT_{BLACK_WIN,WHITE_WIN,TIE,IN_PROGRESS}
      board_s getGameResult_slow(void) const;

      // Should be private used in transition.
      static int getPieceValue(board_s piece);

    private:
      vector<Board> getChildrenInternal_slow(void) const;
      board_s checkAttack_medium(bool byBlack, board_s a, board_s b) const;

      pair<bool, board_s> attemptMove(board_s a, board_s b) const;
      board_s getPiece(board_s a, board_s b) const;

      void makeMove(board_s a, board_s b, board_s c, board_s d);
      void makeMove(board_s a, board_s b, board_s c, board_s d, unsigned char special);

      // Helper methods.
      static bool isWhitePiece(board_s piece);
      static board_s peaceSign(board_s piece);
      static bool onBoard(board_s a, board_s b);
      int getPSTValue(board_s a, board_s b, board_s piece) const;

      void updatePiece(board_s a, board_s b, board_s piece, bool movingTo);

      void updateZobristPiece(board_s a, board_s b, board_s piece);
      void updateZobristTurn(bool isWTurn);
      void updateZobristCastle(char castleStatus);
      void updateZobristEnPassant(move_t &move);

      void promoHelper(
          vector<Board> *all_moves,
          board_s selfColor,
          board_s pawnDirection,
          board_s x,
          board_s y,
          board_s x2) const;

      // Behavior is not defined if multiple pieces exist.
      pair<board_s, board_s> findPiece_slow(board_s piece) const;

      // Size per instance ~= 2 + 2 + 7 + 1 + 64 + 4 + 4 + 4 + 1 + 1 + 8 = 98 bytes.

      // (full move count * 2 + isBlack)
      short gameMoves;
      // Used to count 50 move rule.
      short halfMoves;

      move_t lastMove;

      bool isWhiteTurn;

      board_t state;

      // Evaluations, measured in centipawns (100th of a pawn)
      //  +42 is tiny advantage for white, +842 is a white a queen up, -310 is a minor up for black.
      // Sum of material. (- for black, + for white)
      int material;
      // Sum of all material on board.
      int totalMaterial;

      // Piece square value table lookup.
      int position;

      // whiteOO, whiteOOO, blackOO, blackOOO
      char castleStatus;
      board_hash_t zobrist;
  };
}
#endif // BOARD_H

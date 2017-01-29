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

  // (a, b) to (c, d), piece that moving, piece that was captured removing, special_status
  typedef tuple<board_s, board_s, board_s, board_s, board_s, board_s, unsigned char> move_t;

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

      // Material diff special
      static const board_s  BLACK_WIN = -100;
      static const board_s  WHITE_WIN = 100;

      // Game result
      static const board_s RESULT_TIE = -2; // TODO reconsider value later
      static const board_s RESULT_BLACK_WIN = BLACK_WIN;
      static const board_s RESULT_WHITE_WIN = WHITE_WIN;
      static const board_s RESULT_IN_PROGRESS = 0;


      static const string PIECE_SYMBOL;
      static const map<board_s, movements_t> MOVEMENTS;
      static const map<board_s, int> PIECE_VALUE;
      static const map<board_s, int> ANTICHESS_PIECE_VALUE;

      // Constructor (including the hidden copy constructor).
      Board(bool initState);
      Board(string fen);

      void resetBoard(void);

      Board copy(void);

      string boardStr(void);
      void printBoard(void);

      uint64_t getZobrist(void);

      move_t getLastMove(void);
      vector<Board> getChildren(void);
      vector<Board> getLegalChildren(void);

      void makeMove(move_t move);

      // Algebraic notation of legal move from this board.
      string algebraicNotation(move_t child_move);

      // ~Coordinate notation.
      static string coordinateNotation(move_t move);

      // NOTE(seth): a is y 0-7, b is x 0-7
      static string squareName(board_s a, board_s b);
      static string rankName(board_s a);
      static string fileName(board_s b);

      double heuristic(void);

      scored_move_t findMove(int minNodes);

      void perft(
          int ply,
          atomic<int> *count,
          atomic<int> *captures,
          atomic<int> *ep,
          atomic<int> *castles,
          atomic<int> *promotions,
          atomic<int> *mates);

      // see RESULT_{BLACK_WIN,WHITE_WIN,TIE,IN_PROGRESS}
      board_s getGameResult_slow(void);

    private:
      board_s checkAttack_medium(bool byBlack, board_s a, board_s b);

      pair<bool, board_s> attemptMove(board_s a, board_s b);
      board_s getPiece(board_s a, board_s b);

      void makeMove(board_s a, board_s b, board_s c, board_s d);
      void makeMove(board_s a, board_s b, board_s c, board_s d, unsigned char special);

      // Helper methods.
      static board_s getPieceValue(board_s piece);
      static bool isWhitePiece(board_s piece);
      static board_s peaceSign(board_s piece);
      static bool onBoard(board_s a, board_s b);

      void updateMaterialDiff(board_s removed);
      // Note the added s in Piece(s), this sums all pieces (using 8x8 search)
      int getPiecesValue_slow(void);

      void updateZobristPiece(board_s a, board_s b, board_s piece);
      uint64_t getZobrist_slow(void);


      void promoHelper(
          vector<Board> *all_moves,
          bool isWhiteTurn,
          board_s selfColor,
          board_s pawnDirection,
          board_s x,
          board_s y,
          board_s x2);

      // 1-arg version is public.
      scored_move_t findMoveHelper(int ply, double alpha, double beta);

      // Behavior is not defined if multiple pieces exist.
      pair<board_s, board_s> findPiece_slow(board_s piece);

      // Used for counting moves in findMove
      static atomic<int> dbgCounter;


      // 4 + 2 + 1 + 64 + 1   +   6+1  = 80 bytes.
      uint64_t zobrist;
      short ply;
      bool isWhiteTurn;
      board_t state;
      board_s materialDiff;

      move_t lastMove;

      // whiteOO, whiteOOO, blackOO, blackOOO
      char castleStatus;
  };
}
#endif // BOARD_H

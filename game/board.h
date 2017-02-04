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

      static const string PIECE_SYMBOL;
      static const map<board_s, movements_t> MOVEMENTS;

      // Constructors
      Board(bool initState);
      Board(string fen);
      // Included to avoid bool/string ambiguity.
      Board(char const* s) : Board(string(s)) {};

      void resetBoard(void);

      Board copy(void);

      string boardStr(void);
      string generateFen_slow(void);
      void printBoard(void);

      uint64_t getZobrist(void);
      uint64_t getZobrist2(void);

      move_t getLastMove(void);
      vector<Board> getChildren(void);
      vector<Board> getLegalChildren(void);

      void makeMove(move_t move);
      bool makeAlgebraicMove_slow(string move);

      // Algebraic notation of legal move from this board.
      string algebraicNotation_slow(move_t child_move);

      // ~Coordinate notation.
      static string coordinateNotation(move_t move);

      // NOTE(seth): a is y 0-7, b is x 0-7
      static string squareName(board_s a, board_s b);
      static string rankName(board_s a);
      static string fileName(board_s b);

      uint64_t perftMoveOnly(int ply);

      void perft(
          int ply,
          atomic<long> *count,
          atomic<long> *captures,
          atomic<int> *ep,
          atomic<int> *castles,
          atomic<int> *promotions,
          atomic<int> *mates);

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
      //int getPiecesValue_slow(void);

      void updateZobristPiece(board_s a, board_s b, board_s piece);
      void updateZobristTurn(bool isWTurn);
      void updateZobristCastle(char castleStatus);
      void updateZobristEnPassant(move_t &move);
      uint64_t getZobrist_slow(void);
      uint64_t getZobrist2_slow(void);

      void promoHelper(
          vector<Board> *all_moves,
          bool isWhiteTurn,
          board_s selfColor,
          board_s pawnDirection,
          board_s x,
          board_s y,
          board_s x2);

      // Behavior is not defined if multiple pieces exist.
      pair<board_s, board_s> findPiece_slow(board_s piece);

      // Size per instance ~= 2 + 2 + 1 + 64 + 1 + 1 + 7 + 1 + 4 = 82 bytes.

      // (full move count * 2 + isBlack)
      short gameMoves;
      // Used to count 50 move rule.
      short halfMoves;

      bool isWhiteTurn;
      board_t state;
      //board_s materialDiff;
      move_t lastMove;

      // whiteOO, whiteOOO, blackOO, blackOOO
      char castleStatus;
      uint64_t zobrist;
      uint64_t zobrist2;
  };
}
#endif // BOARD_H

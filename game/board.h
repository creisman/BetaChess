#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

#define BOARD_SIZE 8


// NOTE(seth): It appears that my RPi assumes unsigned char by default so call it out specifically.
typedef signed char board_s;

// (a, b) to (c, d), piece that moving, piece that was captured removing
typedef tuple<board_s, board_s, board_s, board_s, board_s, board_s> move_t;
typedef board_s board_t[BOARD_SIZE][BOARD_SIZE];

class Board {

  static const board_s WHITE = 1;
  static const board_s BLACK = -1;

  static const board_s PAWN = 1;
  static const board_s KNIGHT = 2;
  static const board_s BISHOP = 3;
  static const board_s ROOK = 4;
  static const board_s QUEEN = 5;
  static const board_s KING = 6;

  static constexpr const char* PIECE_SYMBOL = "?pnbrqk";

  // TODO investigate making this static?
  const map<board_s, vector<pair<board_s, board_s>>> MOVEMENTS = {
      {KNIGHT, {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -1}, {-1, 2}, {-1, -2}}},
      {BISHOP, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}},
      {ROOK, {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}},
      {QUEEN, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}}},
      {KING, {{0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1, -1}}},
  };

  const map<board_s, int> PIECE_VALUE = {
      {KING, 200},
      {QUEEN, 9 },
      {ROOK, 5 },
      {BISHOP, 3 },
      {KNIGHT, 3 },
  };

  public:
    Board(bool initState);

    vector<pair<move_t, Board>> getChildren(void);

    void setState(int plyP, board_t stateP);
    void resetBoard(void);

    string boardStr(void);
    void printBoard(void);

    Board copy(void);
    double heuristic(void);
    board_s mateResult(void);

    void perft(int ply, int *count, int *captures, int* mates);

  private:
    pair<bool, board_s> attemptMove(board_s a, board_s b);
    move_t makeMove(board_s a, board_s b, board_s c, board_s d);

    static bool isWhitePiece(board_s piece);
    static board_s peaceSign(board_s piece);

    // TODO investigate usage of this.
    static string squareNamePair(move_t move);

    int ply;
    bool isWhiteTurn;
    board_t state;
    int mateStatus = -2;
};

#endif // BOARD_H

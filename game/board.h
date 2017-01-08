#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <utility>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

#define BOARD_SIZE 8

typedef tuple<char, char, char, char, char, char> move_t;
typedef char board_t[BOARD_SIZE][BOARD_SIZE];

class Board {
  // a, b, c, d,  moving, removing


  const char WHITE = 1;
  const char BLACK = -1;

  const char PAWN = 1;
  const char KNIGHT = 2;
  const char BISHOP = 3;
  const char ROOK = 4;
  const char QUEEN = 5;
  const char KING = 6;

  const map<char, vector<pair<char, char>>> MOVEMENTS = {
      {KNIGHT, {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}}},
      //{KNIGHT : ((1,-2), (2,-1), (2,1), (1,2), (-1,2), (-2,1), (-2,-1), (-1,-2)),
      //{BISHOP : ((1,-1), (1, 1), (-1,1), (-1,-1)),
      //{ROOK : ((0,-1), (1, 0), (0,1), (-1,0)),
      //{QUEEN : ((1,-1), (1, 1), (-1,1), (-1,-1), (0,-1), (1, 0), (0,1), (-1,0)),
      //{KING : ((0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0), (-1, -1))
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
    char mateResult(void);

  private:
    pair<bool, char> attemptMove(char a, char b);
    move_t makeMove(char a, char b, char c, char d);

    static bool isWhitePiece(char piece);

    // TODO investigate usage of this.
    static string squareNamePair(move_t move);
    static bool test(void);

    int ply;
    bool isWhiteTurn;
    board_t state;
};

#endif // BOARD_H

#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <utility>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

#define BOARD_SIZE 8

class Board {
  // a, b, c, d,  moving, removing
  typedef tuple<char, char, char, char, char, char> move_t;
  typedef char board_t[BOARD_SIZE][BOARD_SIZE];


  const char WHITE = 1
  const char BLACK = -1

  const char PAWN = 1
  const char KNIGHT = 2
  const char BISHOP = 3
  const char ROOK = 4
  const char QUEEN = 5
  const char KING = 6

  map<char, vector<pair<char, char>> MOVEMENTS = {
      {KNIGHT, {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}}},
      //{KNIGHT : ((1,-2), (2,-1), (2,1), (1,2), (-1,2), (-2,1), (-2,-1), (-1,-2)),
      //{BISHOP : ((1,-1), (1, 1), (-1,1), (-1,-1)),
      //{ROOK : ((0,-1), (1, 0), (0,1), (-1,0)),
      //{QUEEN : ((1,-1), (1, 1), (-1,1), (-1,-1), (0,-1), (1, 0), (0,1), (-1,0)),
      //{KING : ((0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0), (-1, -1))
    }

  public:
    Board(bool initState);
    void setState(int turn, board_t state);
    void resetBoard(void);

    string boardStr(void);
    void printBoard(void);


    vector<pair<move_t, Board>> getChildren(void);

    pair<bool, char> attemptMove(char a, char b);
    move_t makeMove(char a, char b, char c, char d):

    void pieceColor(char piece);

    // TODO investigate usage of this.
    string squareNamePair(move_t move);

    Board copy(void);
    double heuristic(void);
    char mateResult(void);

    bool test(void);

  private:
    int ply;
    bool isWhite;
    board_t state;

#endif // BOARD_H

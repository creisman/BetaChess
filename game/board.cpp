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
}


void Board::resetBoard(void) {
  ply = 0;
  isWhiteTurn = true;

  memset(&state, sizeof(state), 0);

  for (int i = 0; i < 8; i++) {
    state[1][i] = PAWN;
    state[6][i] = -PAWN;
  }

  state[0][0] = state[0][7] = ROOK;
  state[0][1] = state[0][6] = KNIGHT;
  state[0][2] = state[0][5] = BISHOP;
  state[0][3] = QUEEN;
  state[0][4] = KING;

  state[7][0] = state[7][7] = -ROOK;
  state[7][1] = state[7][6] = -KNIGHT;
  state[7][2] = state[7][5] = -BISHOP;
  state[7][3] = -QUEEN;
  state[7][4] = -KING;
}

string Board::boardStr(void) {
  string rep = "";
  for (int row = 7; row >= 0; row--) {
    for (int col = 0; col < 8; col++) {
      char piece = state[row][col];
      char symbol = (piece != 0) ? PIECE_SYMBOL[abs(piece)] : '.';
      if (piece > 0) {
        symbol = toupper(symbol);
      }
      rep += symbol;
      } 
    rep += "\n";
  }

  return rep;
}

void Board::printBoard(void) {
  cout << boardStr() << endl;
}


vector<pair<move_t, Board>> getChildren(void) {
    // Because we are doing anti-chess first.
    vector<pair<move_t, Board>> non_captures;
    vector<pair<move_t, Board>> captures;

    /*
    for piece, (y,x) in pieces:
      pieceType = abs(piece)
      color = Board.pieceColor(piece)

      # CASTLING, CHECKS (CHECKMATES)

      if pieceType == Board.PAWN:
        # TODO: enpassant, and promotion

        onBoard, destColor = self.attemptMove(y + color, x)
        # if next space is empty.
        if len(captures) == 0 and onBoard and destColor == None:
          c = self.copy()
          move = c.makeMove(y,x,    y + color, x)
          non_captures.append( (move, c) )

          # double move (only if nothing in the way for single move)
          if (self.turn == Board.WHITE and y == 1) or \
             (self.turn == Board.BLACK and y == 6):

            onBoard, destColor = self.attemptMove(y + 2 * color, x)
            if onBoard and destColor == None:
              c = self.copy()
              move = c.makeMove(y,x,    y + 2 * color, x)
              non_captures.append( (move, c) )

        # pawn capture
        for lr in (-1, 1):
          onBoard, destColor = self.attemptMove(y + color, x + lr)
          if onBoard and destColor == self.opp:
            c = self.copy()
            move = c.makeMove(y,x,    y + color, x + lr)
            captures.append( (move, c) )

      elif pieceType in (Board.KNIGHT, Board.KING):
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

      else:
        # slidy pieces = BISHOPS, ROOKS, QUEENS
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
  */
  return captures;
}


pair<bool, bool> attemptMove(char a, char b) {
  // prep for moving piece to state[a][b]
  // returns on board, piece on [a][b]

  if (0 <= a <= 7 && 0 <= b <= 7) {
    char destPiece = state[a][b];
    if (destPiece) {
      return make_pair(true, isWhitePiece(destPiece));
    }
  }
  return make_pair(false, false);
}


move_t Board::makeMove(char a, char b, char c, char d) {
  char moving = state[a][b];
  bool isWhite = isWhitePiece(moving);
  char removed = state[c][d];

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

// TODO change signature;
bool Board::isWhitePiece(char piece) {
  //return WHITE if piece > 0 else Board.BLACK
  return true;
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
      char piece = state[r][c];
      if (piece) {
        // TODO(seth) to be made faster later.
        pieceValue += ((x < 0) ? -1: 1) * PIECE_VALUE[piece];
      }
    }
  }

  //sumValueWhite = sum(pieceValue[piece[0]] for piece in self.whitePieces)
  //sumValueBlack = sum(pieceValue[-piece[0]] for piece in self.blackPieces)

  //mobilityBonus = 0.1 * (whiteMobility - blackMobility)
  //# TODO Determine doubled, blocked, and isolated pawns

  //return sumValueWhite - sumValueBlack + mobilityBonus
  return pieceValue;
}

  // -1 Black victory, 0 no mate, 1 White victory
char Board::mateResult(void) {
    //whiteK = len([1 for piece in self.whitePieces if piece[0] == Board.KING])
    //blackK = len([1 for piece in self.blackPieces if piece[0] == -Board.KING])
    //return whiteK - blackK
    return 0;
}

bool Board::test() {
  int count = 0;
  int caps = 0;
  // capsDict = defaultdict(int)

  //patterns = defaultdict(int)

  Board b(true /* init */);
  /*
  for mb, c in b.getChildren():
    for mc, d in c.getChildren():
      for md, e in d.getChildren():
  #      for me, f in e.getChildren():
  #        for mf, g in f.getChildren():

            count += 1
            //firstMove[(mb[2], Board.squareNamePair(mb[1]))] += 1
            //patterns[(Board.squareNamePair(mc[1]),
            //          Board.squareNamePair(md[1]))] += -1 if mb[1][1] == 0 else 1

            test = caps
  #        caps += (mb[-1] != None)
  #        caps += (mc[-1] != None)
  #        caps += (md[-1] != None)
  #        caps += (me[-1] != None)

  #        if test != caps:
  #          capsDict[(md[-2], md[-1])] += 1
  #          e.printBoard()



  #print (capsDict)
  #for k,v in sorted(firstMove.items()):
  #  print (k,v)

  '''
  print ()
  for k,v in sorted(patterns.items()):
    if v != 0:
      print (k,v)
  '''
  print (count, caps)
  */

  return true;
}

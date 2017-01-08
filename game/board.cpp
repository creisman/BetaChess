#include <cassert>
#include <iostream>
#include <map>
#include <string>
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
    // TODO memcopy
    //self.state = [row[:] for row in state]
}


void Board::resetBoard(void) {
  ply = 0;
  isWhiteTurn = true;

  // TODO memcopy
  //self.state = [[None for i in range(8)] for j in range(8)]
  /*
  for i in range(8):
    self.state[1][i] = Board.PAWN
    self.whitePieces.append( (Board.PAWN, (1, i)) )

    self.state[6][i] = -Board.PAWN
    self.blackPieces.append( (-Board.PAWN, (6, i)) )

  backRow = [Board.ROOK, Board.KNIGHT, Board.BISHOP, Board.QUEEN, Board.KING, Board.BISHOP, Board.KNIGHT, Board.ROOK]
  for i, piece in enumerate(backRow):
    self.state[0][i] = piece
    self.whitePieces.append( (piece, (0, i)) )

    self.state[7][i] = -piece
    self.blackPieces.append( (-piece, (7, i)) )
  */
}

string Board::boardStr(void) {
  string rep = "";
  //for row in range(7, -1, -1):
  //  for piece in self.state[row]:
  //    symbol = "?pkbrqk"[abs(piece)] if piece else "."
  //    if piece and piece > 0:
  //      symbol = symbol.upper()

  //    rep += symbol
  //  rep += "\n"
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

pair<bool, char> attemptMove(char a, char b) {
  // prep for moving piece to state[a][b]
  // returns on board, piece on [a][b]
  /*
  if 0 <= a <= 7 and 0 <= b <= 7:
    destPiece = self.state[a][b]
    return True, Board.pieceColor(destPiece) if destPiece else None
  else:
    return False, None
  */
  return make_pair(false, 0);
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
  return "a4";
//    return "abcdefgh"[yx[1]] + str(yx[0] + 1)
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

    /*
    whiteMobility = 0
    blackMobility = 0
    '''
    #TODO enable when fast enough to handle extra recursion
    originalTurn = self.turn
    self.turn = Board.WHITE
    whiteMobility = 0 # len(list(self.getChildren()))
    self.turn = Board.BLACK
    blackMobility = 0 # len(list(self.getChildren()))
    self.turn = originalTurn
    #'''
    #print("white: " + str(whiteMobility) + " black: " + str(blackMobility))

    pieceValue = {
      Board.KING : 200,
      Board.QUEEN : 9,
      Board.ROOK : 5,
      Board.BISHOP: 3,
      Board.KNIGHT: 3,
      Board.PAWN: 1,
    }

    sumValueWhite = sum(pieceValue[piece[0]] for piece in self.whitePieces)
    sumValueBlack = sum(pieceValue[-piece[0]] for piece in self.blackPieces)

    mobilityBonus = 0.1 * (whiteMobility - blackMobility)
    # TODO Determine doubled, blocked, and isolated pawns

    return sumValueWhite - sumValueBlack + mobilityBonus
    */
    return 0.0;
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

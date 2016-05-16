class Board:
  WHITE = 1
  BLACK = -1

  PAWN = 1
  KNIGHT = 2
  BISHOP = 3
  ROOK = 4
  QUEEN = 5
  KING = 6

  MOVEMENTS = {
      KNIGHT : ((1,-2), (2,-1), (2,1), (1,2), (-1,2), (-2,1), (-2,-1), (-1,-2)),
      BISHOP : ((1,1), (1, -1)),
      ROOK : ((1,0), (0, 1)),
      QUEEN : ((1,0), (0, 1), (1,1), (1, -1)),
      KING : ((0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0), (-1, -1))
    }


  def __init__(self):
    self.resetBoard()


  def setState(self, turn, state, white, black):
    self.turn = turn
    self.state = [[p for p in row] for row in state]
    self.whitePieces = [p for p in white]
    self.blackPieces = [p for p in black]


  def resetBoard(self):
    self.turn = Board.WHITE
    self.state = [[None for i in range(8)] for j in range(8)]
    self.whitePieces = []
    self.blackPieces = []

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
      
  def boardStr(self):
    rep = ""
    for row in range(7, -1, -1):
      for piece in self.state[row]:
        symbol = "?pkbrqk"[abs(piece)] if piece else "."
        if piece and piece > 0:
          symbol = symbol.upper()
        
        rep += symbol
      rep += "\n"
    return rep

  def printBoard(self):
    print (self.boardStr())
#    print (self.whitePieces)
#    print (self.blackPieces)


  def getChildren(self):
    # returns tuple(tuple(move, BOARD), ...)
    pieces = self.whitePieces if self.turn == Board.WHITE else self.blackPieces

    for piece, (y,x) in pieces:
      pieceType = abs(piece)
      color = Board.pieceColor(piece)

      # if pawn move can't capture
      if pieceType == Board.PAWN:
        # TODO: double first move, enpassant, and promotion
        
        onBoard, destColor = self.attemptMove(y + color, x)
        if onBoard and destColor == None:
          c = self.copy()
          c.makeMove(y,x,    y + color, x)
          yield c

        for lr in (-1, 1):
          onBoard, destColor = self.attemptMove(y + color, x + lr)
          if onBoard and destColor != color:
            c = self.copy()
            c.makeMove(y,x,    y + color, x + lr)
            yield c

      if pieceType in (Board.KNIGHT, Board.KING):
        for deltaY, deltaX in Board.MOVEMENTS[pieceType]:
          onBoard, destColor = self.attemptMove(y + deltaY, x + deltaX)
          if onBoard and destColor != color:
            c = self.copy()
            c.makeMove(y, x,   y + deltaY, x + deltaX)
            yield c


  def attemptMove(self, a,b):
    # prep for moving piece to state[c][d]
    # returns on board, piece on [c][d]
    if 0 <= a <= 7 and 0 <= b <= 7:
      destPiece = self.state[a][b]
      
      return True, Board.pieceColor(destPiece) if destPiece else None
    else:
      return False, None

  def makeMove(self, a,b, c,d):
    moving = self.state[a][b]
    color = Board.pieceColor(moving)
    removed = self.state[c][d]

    assert color == self.turn
    if removed:
      assert color != Board.pieceColor(removed)

    self.state[a][b] = None
    self.state[c][d] = moving
    self.turn = Board.BLACK if self.turn == self.WHITE else Board.WHITE

    if color == Board.WHITE:
      self.whitePieces.remove((moving, (a, b)))
      self.whitePieces.append((moving, (c, d)))
      if removed:
        self.blackPieces.remove((removed, (c, d)))
    else:
      self.blackPieces.remove((moving, (a, b)))
      self.blackPieces.append((moving, (c, d)))
      if removed:
        self.whitePieces.remove((removed, (c, d)))


  def pieceColor(piece):
    return Board.WHITE if piece > 0 else Board.BLACK


  def copy(self):
    copy = Board()
    copy.setState(self.turn, self.state, self.whitePieces, self.blackPieces)
    return copy

  def heuristic(self):
    '''
    200(K-K')
    + 9(Q-Q')
    + 5(R-R')
    + 3(B-B' + N-N')
    + 1(P-P')
    - 0.5(D-D' + S-S' + I-I')
    + 0.1(M-M')
    '''

    whiteMobility = 0
    blackMobility = 0
    #'''
    #TODO enable when fast enough to handle extra recursion
    originalTurn = self.turn
    self.turn = Board.WHITE
    whiteMobility = 0 # len(list(self.getChildren()))
    self.turn = Board.BLACK
    blackMobility = 0 # len(list(self.getChildren()))
    self.turn = originalTurn
    #'''
    print("white: " + str(whiteMobility) + " black: " + str(blackMobility))
    return 200 * self.getPieceDifference(Board.KING) +\
      9 * self.getPieceDifference(Board.QUEEN) +\
      5 * self.getPieceDifference(Board.ROOK) +\
      3 * self.getPieceDifference(Board.BISHOP) +\
      3 * self.getPieceDifference(Board.KNIGHT) +\
      1 * self.getPieceDifference(Board.PAWN) +\
      .1 * (whiteMobility - blackMobility)
      # TODO Determine doubled, blocked, and isolated pawns

  def getPieceCount(self, findPiece, side):
    if side == Board.WHITE:
      pieces = self.whitePieces
    else:
      pieces = self.blackPieces
      findPiece = findPiece * -1

    count = 0;
    for (piece, pos) in pieces:
      if piece == findPiece:
        count += 1
    return count

  def getPieceDifference(self, findPiece):
    return self.getPieceCount(findPiece, Board.WHITE) - self.getPieceCount(findPiece, Board.BLACK)


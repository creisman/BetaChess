import time
from board import *

class Game:
  DEPTH = 4

  def __init__(self):
    self.board = Board()
    self.scores = {}

  def getNextMove(self):
    isWhiteMove = self.board.turn == Board.WHITE

    bestMove = None
    bestScore = float("-inf") if isWhiteMove else float("inf")

    newScores = {}
    move, score, totalEvaled = self.recurse(newScores, self.DEPTH, isWhiteMove, self.board)

    print("TotalEvaled: {}".format(totalEvaled))
    print("Score: {} for move {}".format(score, move))
    return move, score

  def recurse(self, newScores, depthRemaining, isWhiteMove, board):
    boardStr = board.boardStr()

    isMate = board.isMate()
    if isMate != 0:
      return None, 100 * isMate, 1

    if depthRemaining == 0:
      return None, board.heuristic(), 1

    totalEvaled = 0
    bestMove = None
    bestScore = float("-inf") if isWhiteMove else float("inf")

    for move, nextBoard in board.getChildren():
      nextMove, score, numEvaled = self.recurse(newScores, depthRemaining - 1, not isWhiteMove, nextBoard)
      totalEvaled += numEvaled

      if (isWhiteMove and score > bestScore) or (not isWhiteMove and score < bestScore):
        bestScore = score
        bestMove = move

    return bestMove, bestScore, totalEvaled

  def makeMove(self, move):
    self.board.makeMove(move[0][0], move[0][1], move[1][0], move[1][1])
    self.board.printBoard()

if __name__ == "__main__":
  game = Game()
  for ply in range(25):
    T0 = time.time()
    move, score = game.getNextMove()
    T1 = time.time()
    print ("\t{:.3f}".format(T1 - T0))

    game.makeMove(move)



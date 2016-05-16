from board import *

class Game:
  DEPTH = 2

  def __init__(self):
    self.board = Board()
    self.scores = {}

  def getNextMove(self):
    newScores = {}
    numStates = self.recurse(newScores, self.DEPTH, self.board)
    print("NumStates: " + str(numStates))
    whiteMove = self.board.turn = Board.WHITE
    bestMove = None
    bestScore = float("-inf") if whiteMove else float("inf")
    for state in newScores.keys():
      (score, move) = newScores[state]
      if (whiteMove and score > bestScore) or (not whiteMove and score < bestScore):
        bestScore = score
        bestMove = move
    return bestScore

  def recurse(self, newScores, depthRemaining, board, move=None):
    if depthRemaining == 0:
      newScores[board.boardStr()] = (board.heuristic(), move)
      return 1

    nextStates = board.getChildren()
    numStates = 0
    for nextBoard in nextStates:
      numStates += self.recurse(newScores, depthRemaining - 1, nextBoard)
    return numStates

if __name__ == "__main__":
  game = Game()
  print(game.getNextMove())


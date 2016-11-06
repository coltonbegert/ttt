import numpy as np
from winning_state import winning_state

class Board:
    def __init__(self):
        self._board = np.zeros((9,9), dtype='uint8')
        self._miniwins = np.zeros((3,3), dtype='uint8')
        self._next_board = None

    def move(self, row, col, player):
        (br, mr), (bc, mc) = divmod(row,3), divmod(col,3)
        assert self._next_board is None or (br,bc) == self._next_board, "Must play in the active board"
        assert self._board[row, col] == 0, "That cell is taken"
        assert self._miniwins[br, bc] == 0, "That board already has a winner"
        self._board[row, col] = player

        miniboard = self._get_miniboard(br, bc)
        if winning_state(miniboard, player):
            self._miniwins = player
            self._next_board = None
        else:

            self._next_board = mr, mc

    def _get_miniboard(self, br, bc):
        return self._board[br*3:(br+1)*3, bc*3:(bc+1)*3]

    def get_valid(self):
        if self._next_board is None:
            return [(r,c) for r,c in board_iter(9) if self._board[r,c] == 0]
        else:
            br, bc = self._next_board
            return [(br*3+r,bc*3+c) for r,c in board_iter(3) if self._board[r,c] == 0]

    def __repr__(self):
        s = '\n'
        for i in range(9):
            for j in range(9):
                s += [' ','X','O'][self._board[i,j]]
                if j % 3 == 2:
                    s += ' '
                elif j < 8:
                    s += '|'
            s += '\n'
            if i % 3 == 2:
                s += '\n'
            else:
                s += '-+-+- ' * 3 + '\n'
        return s

def board_iter(size):
    for r in range(size):
        for c in range(size):
            yield r,c
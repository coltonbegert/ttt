import numpy as np
from winning_state import winning_state

class Board:
    def __init__(self):
        self._board = np.zeros((9,9), dtype='uint8')
        self._miniwins = np.zeros((3,3), dtype='uint8')
        self._next_board = None
        self._last_move = None
        self.winner = None
        self.player = 1
        self.turns_left = 81

    def clone(self):
        B = Board()
        B._board = self._board.copy()
        B._miniwins = self._miniwins.copy()
        B._next_board = self._next_board
        B.winner = self.winner
        B.player = self.player
        B.turns_left = self.turns_left
        B._last_move = self._last_move
        return B

    def move(self, row, col, player=None):
        if player is None:
            player = self.player
        (br, mr), (bc, mc) = divmod(row,3), divmod(col,3)

        assert self._next_board is None or (br,bc) == self._next_board, "Must play in the active board"
        assert self._board[row, col] == 0, "That cell is taken"
        assert self._miniwins[br, bc] == 0, "That board already has a winner"

        self._last_move = row, col

        self._board[row, col] = player
        self.turns_left -= 1

        # Check if the miniboard has a win
        miniboard = self._get_miniboard(br, bc)
        miniwin = winning_state(miniboard, player)
        if miniwin:
            self._miniwins[br, bc] = miniwin
            if self._miniwins[mr, mc] == 0:
                self._next_board = mr, mc
            else:
                self._next_board = None
            # Check if the main board has a win
            big_win = winning_state(self._miniwins, player)
            if big_win == player:
                self.winner = player
            # Big game tie!
            elif big_win > 0:
                self.winner = 0
                return 0
        else:
            # Make sure that board doesn't have a winner
            if self._miniwins[mr,mc] == 0:
                self._next_board = mr, mc
            else:
                self._next_board = None

            if self.turns_left == 0:
                self.winner = 0

        # Swap players
        self.player = 3 - self.player

        return self.winner

    def _get_miniboard(self, br, bc):
        return self._board[br*3:(br+1)*3, bc*3:(bc+1)*3]

    def get_valid(self):
        if self._next_board is None:
            return [(r,c) for r,c in board_iter(9) if self._board[r,c] == 0 and self._miniwins[r//3,c//3] == 0]
        else:
            br, bc = self._next_board
            return [(br*3+r,bc*3+c) for r,c in board_iter(3) if self._board[br*3+r,bc*3+c] == 0]

    def __repr__(self, lastmove=lambda s: s, miniwin=lambda s, w: s, active=lambda s: s):
        s = '\n '
        for i in range(9):
            for j in range(9):
                if (i,j) == self._last_move:
                    s += lastmove([' ','X','O'][self._board[i,j]])
                else:
                    s += [' ','X','O'][self._board[i,j]]
                if j % 3 == 2:
                    s += ' '
                elif j < 8:
                    if self._miniwins[i//3,j//3]:
                        s += miniwin('|', self._miniwins[i//3,j//3])
                    else:
                        if self._next_board == (i//3,j//3):
                            s += active('|')
                        else:
                            s += '|'
            s += '\n '
            if i % 3 == 2:
                s += '\n '
            else:
                for k in range(3):
                    if self._miniwins[i//3,k]:
                        s += miniwin('-+-+- ', self._miniwins[i//3,k])
                    else:
                        if self._next_board == (i//3,k):
                            s += active('-+-+- ')
                        else:
                            s += '-+-+- '
                s += '\n '
        return s

    def pprint(self, player=None):
        """ Pretty representation """
        lastmove = lambda s: '\033[1;33;50m{}\033[0;0;0m'.format(s)
        if player is not None:
            miniwin = lambda s,w: '\033[0;{};50m{}\033[0;0;0m'.format(31+(w==player),s)
        else:
            miniwin = lambda s,w: '\033[0;{};50m{}\033[0;0;0m'.format([37,36,35,30][w],s)
        active = lambda s: '\033[1;33;50m{}\033[0;0;0m'.format(s)
        print(self.__repr__(lastmove=lastmove, miniwin=miniwin, active=active))

def board_iter(size):
    for r in range(size):
        for c in range(size):
            yield r,c

def pack(*args):
    return bytes(args)


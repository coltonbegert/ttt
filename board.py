import numpy as np
from winning_state import winning_state

class Board:
    def __init__(self):
        self._board = np.zeros((9,9), dtype='uint8')
        self._miniwins = np.zeros((3,3), dtype='uint8')
        self._next_board = None
        self.winner = None
        self.player = 1
        self.turns_left = 81

    def move(self, row, col, player):
        (br, mr), (bc, mc) = divmod(row,3), divmod(col,3)

        assert self._next_board is None or (br,bc) == self._next_board, "Must play in the active board"
        assert self._board[row, col] == 0, "That cell is taken"
        assert self._miniwins[br, bc] == 0, "That board already has a winner"

        self._board[row, col] = player
        self.turns_left -= 1

        # Check if the miniboard has a win
        miniboard = self._get_miniboard(br, bc)
        miniwin = winning_state(miniboard, player)
        if miniwin:
            self._miniwins[br, bc] = miniwin
            self._next_board = None
            # Check if the main board has a win
            if winning_state(self._miniwins, player):
                self.winner = player
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

def play():
    # Allows humans to play
    B = Board()
    while B.winner is None:
        print("Player", B.player)
        print(B)
        if B._next_board is None:
            br = get_input("Board row: ")
            bc = get_input("Board col: ")
        else:
            br, bc = B._next_board

        print("Playing in board {}, {}".format(br, bc))

        mr = get_input("Row: ")
        mc = get_input("Col: ")

        r = br*3 + mr
        c = bc*3 + mc

        try:
            B.move(r,c, B.player)
        except AssertionError as e:
            print("##############")
            print(e)
            print("##############")
    print("\n\n#########")
    print("Player", B.winner, "wins!")
    print("#########")

def get_input(prompt):
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print("##############")
            print("Invalid input")
            print("##############")

if __name__ == '__main__':
    play()
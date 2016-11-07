from random import choice
from board import Board

# Plays a random game of UTTT and puts the moves list in a text file

B = Board()
with open('moves.dat', 'w') as f:
    while B.winner is None:
        print("Player", B.player)
        print(B)

        r,c = choice(B.get_valid())
        f.write("(r,c)\n")
        print("played ({},{})".format(r,c))
        B.move(r,c, B.player)

print("Winner: Player", B.winner)
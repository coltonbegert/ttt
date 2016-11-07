from random import choice
from board import Board

# Plays a random game of UTTT and puts the moves list in a text file
def sample(outfile='moves.dat'):
    B = Board()
    with open(outfile, 'w') as f:
        while B.winner is None:
            print("Player", B.player)
            print(B)

            print("Choosing from:")
            print(B.get_valid())
            r,c = choice(B.get_valid())
            f.write("(r,c)\n")
            print("played ({},{})".format(r,c))
            B.move(r,c, B.player)

    print("Winner: Player", B.winner)
    return B.winner

if __name__ == '__main__':
    sample()
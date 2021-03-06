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
            f.write("({},{})\n".format(r,c))
            print("played ({},{})".format(r,c))
            B.move(r,c, B.player)

    if B.winner:
        print("Winner: Player", B.winner)
    else:
        print("Game is a tie!")
    return B.winner

if __name__ == '__main__':
    sample()

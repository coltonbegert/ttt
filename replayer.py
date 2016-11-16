from board import Board
from time import sleep
import sys

# Replays a game of UTTT loaded from an input file
def replay(infile, pretty_print=False, show_options=False):
    B = Board()
    moves = []
    i = 0
    with open(infile) as f:
        print("Reading file...")
        for line in f:
            line = line.strip()
            if not line: continue
            if line[0] == '#': continue
            _,sr,_,sc,*_ = line
            r = int(sr)
            c = int(sc)
            moves.append( (r,c) )

        print("Press Enter to continue")
        print("Press Control + C to go back")
        print("Press Control + D to quit")
        while True:
            if i == len(moves):
                print("Control + D to quit")
            else:
                r,c = moves[i]
                B.move(r,c, B.player)
                print("Player", B.player)
            if not pretty_print:
                print(B)
            else:
                B.pprint()

            if show_options: print(B.get_valid())

            print("played ({},{})".format(r,c))

            try:
                opt = input("\n>>> ").lower()
            except EOFError:
                break
            except KeyboardInterrupt:
                opt = 'b'
                print('\r>>>     ')
            if opt == 'b':
                i = max(i-1, 0)
                B = Board()
                for move in moves[:i]:
                    B.move(*move)
            else:
                i = min(i+1,len(moves))
    for move in moves[i+1:]:
        B.move(*move)
    B.pprint()
    if B.winner:
        print("Winner: Player", B.winner)
    else:
        print("Game is a tie!")

    return B.winner

if __name__ == '__main__':
    fname = 'moves.dat'
    if len(sys.argv) == 2:
        fname = sys.argv[-1]
    replay(fname, True, False)

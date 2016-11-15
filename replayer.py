from board import Board
from time import sleep

# Replays a game of UTTT loaded from an input file
def replay(infile, pretty_print=False, step_time=0, show_options=False):
    B = Board()
    with open(infile) as f:
        for line in f:
            line = line.strip()
            if not line: continue
            if line[0] == '#': continue
            _,sr,_,sc,*_ = line
            r = int(sr)
            c = int(sc)

            print("Player", B.player)
            if not pretty_print:
                print(B)
            else:
                B.pprint()

            if show_options: print(B.get_valid())

            print("played ({},{})".format(r,c))
            B.move(r,c, B.player)

            sleep(step_time)

    print(B)
    if B.winner:
        print("Winner: Player", B.winner)
    else:
        print("Game is a tie!")

    return B.winner

if __name__ == '__main__':
    replay('moves.dat', True, 2, False)

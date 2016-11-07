from board import Board

# Replays a game of UTTT loaded from an input file
def replay(infile='moves.dat'):
    B = Board()
    with open(infile) as f:
        for line in f:
            _,sr,_,sc,*_ = line
            r = int(sr)
            c = int(sc)

            print("Player", B.player)
            print(B)
            print(B.get_valid())
            print("played ({},{})".format(r,c))
            B.move(r,c, B.player)

    print(B)
    if B.winner:
        print("Winner: Player", B.winner)
    else:
        print("Game is a tie!")

    return B.winner

if __name__ == '__main__':
    replay()

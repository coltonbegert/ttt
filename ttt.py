from valid_move import valid_move
from print_board import print_board
from winning_state import winning_state



def main(argv):
    game = True

    while game:
        print_board()

        user_input = raw_input("Enter a move: ")
        while !valid_move(user_input):
            user_input = raw_input("Enter a valid move: ")

        if winning_state():
            game = False

    print_board()


if __name__ == '__main__':
    main(sys.argv)

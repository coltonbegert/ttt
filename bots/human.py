from bots.base_bot import BaseBot

class Bot(BaseBot):
    def request(self):
        print("Player", self.board.player)

        while True:
            B = self.board.clone()
            B.pprint()
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
                continue

            confirm = '?'
            while confirm not in {'y','n'}:
                B.pprint()
                confirm = input("Is this what you want? (Y/n) ").lower()
                if not confirm: confirm = 'y'
            if confirm == 'n':
                continue

            print("Waiting for a response from your opponent...\n")
            return r,c

def get_input(prompt):
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print("##############")
            print("Invalid input")
            print("##############")


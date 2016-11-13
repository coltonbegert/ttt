from bots.base_bot import BaseBot

class Bot(BaseBot):
    def request(self):
        print("Player", self.board.player)

        while True:
            B = self.board.clone()
            print(B)
            if B._nextboard is None:
                br = get_input("Board row: ")
                bc = get_input("Board col: ")
            else:
                br, bc = B._nextboard

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

            return r,c

def get_input(prompt):
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print("##############")
            print("Invalid input")
            print("##############")


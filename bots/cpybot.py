from bots.base_bot import BaseBot

class Bot(BaseBot):
    """
    Bot for C extension files.

    Your C code needs to #include cpybot.h
    as the last import.
    """
    def setup(self, *args):
        print(args)
        self.cbot = get_cbot(args[0])
        self.cbot.setup(*args[1:])

    def start(self):
        """ Called after the connection is made """
        self.cbot.start()

    def stop(self):
        """ Called after the game is over """
        self.cbot.stop()

    def on_update(self, last_player, last_move):
        """ Called after a move is made """
        import fast_mcts
        self.cbot.update(last_player, last_move)

    def update_board(self, board):
        """ Called on a board wipe signal (NOT IMPLEMENTED) """
        pass

    def request(self):
        """ Called when the bot is expected to make a turn """
        return self.cbot.request()

def get_cbot(name):
    if name == 'mcts':
        import c_mcts
        return c_mcts
    elif name == 'fast':
        import fast_mcts
        return fast_mcts
    else:
        raise ValueError("Unknown cpy bot '{}'".format(name))

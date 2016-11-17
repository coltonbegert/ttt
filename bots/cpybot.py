from bots.base_bot import BaseBot

class Bot(BaseBot):
    """
    Bot for C extension files.

    Your C code needs to #include cpybot.h
    as the last import.
    """
    def setup(self, *args):
        import fast_mcts
        fast_mcts.setup(*args)

    def start(self):
        """ Called after the connection is made """
        import fast_mcts
        fast_mcts.start()

    def stop(self):
        """ Called after the game is over """
        import fast_mcts
        fast_mcts.stop()

    def on_update(self, last_player, last_move):
        """ Called after a move is made """
        import fast_mcts
        fast_mcts.update(last_player, last_move)

    def update_board(self, board):
        """ Called on a board wipe signal (NOT IMPLEMENTED) """
        import fast_mcts
        pass

    def request(self):
        """ Called when the bot is expected to make a turn """
        import fast_mcts
        return fast_mcts.request()

class BaseBot:
    """
    Base for all bots.
    You must override request.
    If your bot has setup actions, you should override the start method, not init.
    If your bot has update actions, you should override the on_update method
    If your bot does not keep a running copy of the board, you should override update_board (NOT IMPLEMENTED)
    If you must override the default behaviour, you should use a super call
    """

    """
    Subclasses should override some of the below methods
    """
    def setup(self, *args):
        """ Called after initialization """
        pass

    def start(self):
        """ Called after the connection is made """
        pass

    def stop(self):
        """ Called after the game is over """
        pass

    def on_update(self, last_player, last_move):
        """ Called after a move is made """
        pass

    def update_board(self, board):
        """ Called on a board wipe signal (NOT IMPLEMENTED) """
        pass

    def request(self):
        """ Called when the bot is expected to make a turn """
        raise NotImplementedError("Subclasses must override 'request()'")

    """
    Subclasses should not override these methods
    """
    def __init__(self, board, player, *args):
        self.board = board
        self.player = player
        self.setup(*args)

    def update(self, last_player, last_move):
        self.board.move(*last_move)
        self.on_update(last_player, last_move)

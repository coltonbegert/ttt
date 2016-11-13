class BaseBot:
    """
    Base for all bots.
    You must override request.
    If your bot has setup actions, you should override the start method, not init.
    If your bot has update actions, you should override the on_update method
    If your bot does not keep a running copy of the board, you should override update_board (NOT IMPLEMENTED)
    """

    """
    Subclasses should override some of the below methods
    """
    def start(self):
        pass

    def on_update(self, last_player, last_move):
        pass

    def update_board(self, board):
        pass

    def request(self):
        raise NotImplementedError("Subclasses must override 'request()'")

    """
    Subclasses should not override these methods
    """
    def __init__(self, board, player):
        self.board = board

    def update(self, last_player, last_move):
        self.board.move(*last_move)
        self.on_update(last_player, last_move)

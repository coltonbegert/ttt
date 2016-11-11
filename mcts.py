class MCTS:
    """
    An object that performs Monte-Carlo Tree Search.
    It continuously simulates on a thread using a provided
    simulation function and an initial board state.
    """
    def __init__(self, board_state, simulation_function):
        """
        :param board_state: An object that represents the board
        or board state. It must include a "get_valid" function
        that determines which moves are available for expansion.
        :param simulation_function: A function that returns 1 for win
        and 0 for loss, given a board state.
        """
        self._board_state = board_state
        self._simulation_function = simulation_function

    def predict(self):
        """
        Returns the predicted next best move
        :return:
        """
        # TODO
        pass

    def update(self, new_state):
        """
        Updates the tree to match the new state of board,
        as well as informs all coprocesses that the state as changed
        and the simulation needs to reflect that.
        :return:
        """
        self._board_state = new_state
        # TODO


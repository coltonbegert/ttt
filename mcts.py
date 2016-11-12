import queue
import time
import threading
import simulations

class MCTS:
    """
    An object that performs Monte-Carlo Tree Search.
    It continuously simulates on a thread using a provided
    simulation function and an initial board state.
    """
    def __init__(self, board_state, simulation_function, priority_func=None):
        """
        :param board_state: An object that represents the board
        or board state. It must include the following functions:
            "get_valid(self)" - determines which moves are available for expansion,
            "clone(self)" - creates a safe to use in simulations
            "move(self, *args, player)" - function to apply a move to. Should return
                None if the game is non-terminal, 0 for a tie, otherwise for player win
            "player" - current player turn
        :param simulation_function: A function that returns 1 for win
        and 0 for loss, given a board state.
        :param priority_func: Optional priority function that converts a (win, visits)
        tuple into a score.
        """
        self._board_state = board_state.clone()
        self._simulation_function = simulation_function
        self._tree = queue.PriorityQueue()
        self._updating = False
        self._lock = threading.Lock()
        if priority_func is not None:
            self._get_score = priority_func
        else:
            import random
            self._get_score = lambda x: (x[0] + 1 + random.random()) / (x[1] + 2)

    def search(self):
        # Run a single MCTS search
        board = self._board_state.clone()
        self._search(board, self._tree)

    def _search(self, board, subtree):
        # Selection
        if not subtree.empty():
            # -- Recursively descend the tree until we get an empty node
            # choice = move, subtree
            choice = subtree.get()
            priority, score, move, next_tree = choice
            # -- Descend a non-leaf, fetching the simulation result
            board.move(*move)
            # print(board)
            # -- Get the simulation result
            #    If the simulation was a win, this leaf lost
            # -- (Playing this move yields this winner:)
            winner = self._search(board, next_tree)
            if winner is None:
                result = 0
            else:
                result = winner
            # -- Update the state
            score = score[0]+result, score[1]+1
            priority = self._get_score(score)
            subtree.put( (priority, score, move, next_tree) )
            # -- Backprop the result
            return winner
        else:
            # Expansion
            # -- Expand this leaf node
            for move in board.get_valid():
                score = (0,0)
                priority = self._get_score(score)
                subtree.put( (priority, score, move, queue.PriorityQueue()) )

            # Simulation
            if not subtree.empty():
                priority, score, move, next_tree = subtree.get()
                # -- Run a simulation after the next move
                board.move(*move)
                # -- (Playing this move yields this predicted winner:)
                winner = self._simulation_function(board)
                if winner is None:
                    result = 0
                else:
                    result = winner

                # -- Update the state
                score = score[0]+result, score[1] + 1
                priority = self._get_score(score)
                subtree.put( (priority, score, move, next_tree) )
                # -- Backprop the result
                return winner
            else:
                # -- This is a game tree leaf. Must return a real result
                return self._simulation_function(board)

    def start(self):
        """
        Starts a threaded process to keep trying to update the tree
        in the background
        """
        print("Starting MCTS")
        def run():
            while True:
                # Mutex lock
                self._lock.acquire()
                self.search()
                self._lock.release()
                time.sleep(0.00001)
        self.thread = threading.Thread(target=run, daemon=True)
        self.thread.start()

    def predict(self):
        """
        Returns the predicted next best move
        :return:
        """
        if self._tree.empty():
            self.search()
        priority, score, move, next_tree = self._tree.get()
        self._tree.put( (priority, score, move, next_tree) )
        print("Predicted best move", move, "with score", score, "(conf: %g)" % priority)
        return move

    def update(self, real_move):
        """
        Updates the tree to match the new state of board,
        as well as informs all coprocesses that the state as changed
        and the simulation needs to reflect that.
        :return:
        """
        # Mutex lock
        self._lock.acquire()

        # Update the internal state
        self._board_state.move(*real_move)
        # Shift the tree to the new state
        while not self._tree.empty():
            priority, score, move, next_tree = self._tree.get()
            if move == real_move:
                self._tree = next_tree
                break
        self._lock.release()
        return

def get_player(board, player, sim=simulations.random_mini_final):
    return MCTS(board, sim(player))

def trial():
    import board
    import simulations
    B = board.Board()
    p1 = simulations.random_mini_final(1)
    p2 = simulations.random_mini_final(2)
    bot1 = MCTS(B, p1)
    bot2 = MCTS(B, p2)

    bot1.start()
    bot2.start()

    print("Giving the bots a chance to warm up...")
    time.sleep(5)

    bots = [bot1, bot2]

    while B.winner is None:
        print("Player", B.player)
        print(B)

        # Get the next move
        bot = bots[B.player-1]
        # -- Wait for at least 1 second for the player to think
        time.sleep(.5)
        move = bot.predict()
        for bot in bots:
            bot.update(move)

        # Apply the move
        print("played ({}, {})".format(*move))
        B.move(*move)

    print("\n\n#########")
    if B.winner:
        print("Player", B.winner, "wins!")
    else:
        print("Game is a tie!")
    print("#########")
    print(B)

if __name__ == '__main__':
    trial()

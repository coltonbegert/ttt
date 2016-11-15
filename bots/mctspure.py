from bots.base_bot import BaseBot
from math import sqrt, log
from time import time, sleep
from random import choice, shuffle, randint, random
import numpy as np
import threading

class Bot(BaseBot):
    def setup(self, *args):
        self.tree = []
        self.root_score = [0, 0]

        self.thinking_time = 15
        self.min_searches = 1000
        self.max_searches = 50000

        self.waiting = False
        self.lock = threading.Lock()

        self.select_const = 1 # Const to use for UCB in MCTS.select
        self.picking_const = -1  # Const to use when picking a move to make

    def start(self):
        self.last_time = time()
        self.counter = 0
        self.turn_number = 0

        # Thread process
        def runner():
            while self.board.winner is None:
                self.lock.acquire()
                # -- Guarantee we meet our minimum searches
                for i in range(self.min_searches):
                    self.search()
                    sleep(0.000001)
                self.waiting = True
                self.lock.release()

                # -- If we have time, keep searching
                while self.waiting:
                    self.lock.acquire()
                    self.search()
                    self.lock.release()
                    sleep(0.00001)

        self.thread = threading.Thread(target =runner)
        self.thread.start()

    def search(self):
        # Update the root node using MCTS
        winner = self._search(self.board.clone(), self.tree, self.root_score)
        self._update_score(self.root_score, winner, self.board.player)
        self.counter += 1

    def _search(self, board, tree, parent_score):
        player = board.player
        ## Selection
        if tree:
            parent_score, score, move, subtree = self.get_best(tree, self.select_const)
            board.move(*move)
            winner = self._search(board, subtree, score)
            # -- Fall through for backprop
        else:
            if board.winner is not None:
                # -- This leaf is terminal
                return board.winner
            else:
                ## Expansion
                options = board.get_valid()
                for move in options:
                    branch = [parent_score, [0,0], move, []]
                    tree.append(branch)

                branch = choice(tree)
                parent_score, score, move, subtree = branch

                ## Simulation
                board.move(*move)
                winner = self.simulate(board)

        ## Backprop
        self._update_score(score, winner, player)
        return winner

    def get_best(self, tree, const):
        # Returns the best move in the given tree or subtree
        return max(tree, key=self.scoring_function(const))

    def _update_score(self, score, winner, player):
        # Updates the score for the player and winner
        is_win = int(winner == player)
        score[0] += is_win
        score[1] += 1

    def simulate(self, board):
        # Run a single simulation, returning the player number of the victor
        # or 0 for a tie
        turns = 10

        # Take a 1 move win always, if available,
        # after each player has played about 15 moves
        # (We can start picking after the 9th move, but the chances are low
        # that this will be a successful strategy, so don't waste our precious
        # simulation time doing that).
        if board.turns_left < 50:
            for move in board.get_valid():
                B = board.clone()
                B.move(*move)
                if B.winner is not None and B.winner > 0: return B.winner

        while board.winner is None and turns > 0:
            options = board.get_valid()
            board.move(*choice(options))
            turns -= 1
        if board.winner is not None:
            return board.winner
        state = board._miniwins
        final_state = state + np.random.randint(1,3, state.shape)*(state==0)
        for p in [1,2]:
            for i in range(3):
                if (final_state[:,i] == p).all():
                    return p
                if (final_state[i,:] == p).all():
                    return p
        if final_state[0,0] == final_state[1,1] == final_state[2,2]:
            return final_state[1,1]
        if final_state[2,0] == final_state[1,1] == final_state[0,2]:
            return final_state[1,1]
        return 0

    def scoring_function(self, const):
        # Prefer branches that have high confidence
        # Break ties randomly
        return lambda branch: self.confidence(branch, const)

    def score_branch(self, branch):
        return self.confidence(branch), random()

    def confidence(self, branch, c=.707):
        # UCB formula
        parent_score, score, move, subtree = branch
        if score[1] == 0 or parent_score[1] == 0: return float('inf')

        mean = score[0] / score[1]
        return mean + c*sqrt(log(parent_score[1]) / score[1]) # UCB

    def request(self):
        # Ask the bot for a move

        print("My turn?")
        # -- Give some time to think in case the state changed
        while self.counter < self.max_searches and time() - self.last_time < self.thinking_time:
            sleep(.5)
            m = int(time()%4)
            print('\rHmm', '.'*m, ' '*(4-m), end='', flush=True)

        print("\rOkay, I got it.")

        print("-- Potential moves --")
        for i,branch in enumerate(sorted(self.tree, key=self.confidence, reverse=True)):
            parent_score, score, move, subtree = branch
            print("{:4s}".format(str(i+1)), move, "->", "{:12s}".format(str(score)), "{:.3f}".format(self.confidence(branch)))

        # Pick the move that is least likely to be a bad move
        branch = self.get_best(self.tree, self.picking_const)
        parent_score, score, move, subtree = branch
        print()
        print("Choosing move {} with score {} and confidence {:.3f}".format(move, score, self.confidence(branch)))
        print("  Root score was {}".format(self.root_score))
        print()

        assert parent_score == self.root_score, self.tree
        return move

    def update(self, last_player, last_turn):
        # Cut the dead branches when a move is made
        # We need to override the default behaviour
        # because we're using threads

        self.lock.acquire()
        self.waiting = False

        super(Bot, self).update(last_player, last_turn)
        for branch in self.tree:
            parent_score, score, move, subtree = branch
            if move == last_turn:
                self.root_score = score
                self.tree = subtree

        self.last_time = time()
        self.counter = 0
        self.turn_number += 1
        self.lock.release()


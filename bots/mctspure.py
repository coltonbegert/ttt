from bots.base_bot import BaseBot
from math import sqrt, log
from time import time, sleep
from random import choice, shuffle, randint, random
import numpy as np
import threading
from simulations import mini_game

class Bot(BaseBot):
    def setup(self, *args):
        self.tree = []
        self.root_score = [0, 0]

        self.thinking_time = 15
        self.min_searches = 1000
        self.max_searches = 50000

        # Simulation mechanism
        self.simulate = mini_game

        self.waiting = False
        self.lock = threading.Lock()

        # Optimal constants are theoretically sqrt(2), but
        # this causes a large amount of exploration.
        # Setting these closer to 0 results in more exploitation and less exploration.
        # Setting these to a negative value forces it to
        # use lower confidence bounds instead of upper confidence bounds.

        self.select_const = 1 # Const to use for UCB in MCTS.select
        self.picking_const = -1  # Const to use when picking a move to make

        # Debugging
        self.print_potential_moves = True
        self.print_expected_moves = True

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

    def scoring_function(self, const):
        # Return a function that can be used in sorting
        return lambda branch: self.confidence(branch, const)

    def confidence(self, branch, c):
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

        if self.print_potential_moves:
            print("-- Potential moves --")
            for i,branch in enumerate(sorted(self.tree, key=self.scoring_function(self.picking_const), reverse=True)):
                parent_score, score, move, subtree = branch
                print("{:4s}".format(str(i+1)), move, "->", "{:15s}".format(str(score)),
                      "{:.3f}".format(self.confidence(branch, self.select_const)),
                        "({:.3f})".format(self.confidence(branch, self.picking_const)))

        # Pick the move that is least likely to be a bad move
        branch = self.get_best(self.tree, self.picking_const)
        parent_score, score, move, subtree = branch
        print()
        print("Choosing move {} with score {} and confidence {:.3f}".format(move, score, self.confidence(branch, self.picking_const)))
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

        # Print expected moves
        if self.print_expected_moves and last_player != self.player:
            print("-- Expected moves --")
            for i,branch in enumerate(sorted(self.tree, key=self.scoring_function(self.picking_const), reverse=True)):
                parent_score, score, move, subtree = branch
                print("{:4s}".format(str(i+1)), move, "->", "{:12s}".format(str(score)),
                      "{:.3f}".format(self.confidence(branch, self.select_const)),
                      "({:.3f})".format(self.confidence(branch, self.picking_const)))

        super(Bot, self).update(last_player, last_turn)
        for branch in self.tree:
            parent_score, score, move, subtree = branch
            if move == last_turn:
                self.root_score = score
                self.tree = subtree
                break

        if self.print_expected_moves and last_player != self.player:
            print()
            print("Opponent played {} with score {} and confidence {:.3f}".format(move, score, self.confidence(branch, self.picking_const)))
            print("  Root score was {}".format(parent_score))
            print()

        self.last_time = time()
        self.counter = 0
        self.turn_number += 1
        self.lock.release()


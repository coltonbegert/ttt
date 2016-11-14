from bots.base_bot import BaseBot
from math import sqrt, log
from time import time, sleep
from random import choice, shuffle, randint, random
import threading

class Bot(BaseBot):
    def setup(self, *args):
        self.tree = []
        self.root_score = [0, 0]

        self.thinking_time = 15
        self.min_searches = 1000
        self.max_searches = 10000

        self.waiting = False
        self.lock = threading.Lock()

    def start(self):
        self.last_time = time()
        self.counter = 0
        self.turn_number = 0

        def runner():
            while self.board.winner is None:
                self.lock.acquire()
                for i in range(self.min_searches):
                    self.search()
                    sleep(0.000001)
                self.waiting = True
                self.lock.release()

                while self.waiting:
                    self.lock.acquire()
                    self.search()
                    self.lock.release()
                    sleep(0.00001)

        self.thread = threading.Thread(target =runner)
        self.thread.start()

    def search(self):
        winner = self._search(self.board.clone(), self.tree, self.root_score)
        self._update_score(self.root_score, winner, self.board.player)
        self.counter += 1

    def _search(self, board, tree, parent_score):
        player = board.player
        # Selection
        if tree:
            parent_score, score, move, subtree = self.get_best(tree)
            board.move(*move)
            winner = self._search(board, subtree, score)
        else:
            options = board.get_valid()
            if not options:
                return board.winner
            else:
                # Expansion
                for move in options:
                    branch = [parent_score, [0,0], move, []]
                    tree.append(branch)

                branch = choice(tree)
                parent_score, score, move, subtree = branch

                # Simulation
                board.move(*move)
                winner = self.simulate(board)

        self._update_score(score, winner, player)

        # Backprop
        return winner

    def get_best(self, tree):
        return max(tree, key=self.scoring_function)

    def _update_score(self, score, winner, player):
        is_win = int(winner == player)
        score[0] += is_win
        score[1] += 1

    def simulate(self, board):
        while board.winner is None:
            options = board.get_valid()
            if board.turns_left < 64:
                # Take a winning move if available
                for move in options:
                    B = board.clone()
                    B.move(*move)
                    if B.winner is not None and B.winner > 0:
                        return B.winner
            board.move(*choice(options))
        return board.winner

    def scoring_function(self, branch):
        # Prefer branches that have high confidence
        # Break ties randomly
        return self.confidence(branch), random()

    def confidence(self, branch):
        parent_score, score, move, subtree = branch
        if score[1] == 0 or parent_score[1] == 0: return float('inf')

        mean = score[0] / score[1]
        return mean + sqrt(2*log(parent_score[1]) / score[1]) # UCB

    def request(self):
        print("My turn?")
        # Give some time to think
        while self.counter < self.max_searches and time() - self.last_time < self.thinking_time:
            sleep(.5)
            m = int(time()%4)
            print('\rHmm', '.'*m, ' '*(4-m), end='', flush=True)

        print("\rOkay, I got it.")

        branch = self.get_best(self.tree)
        parent_score, score, move, subtree = branch
        print("Choosing move {} with score {} and confidence {:.3f}".format(move, score, self.confidence(branch)))
        print("  Root score was {}".format(self.root_score))
        assert parent_score == self.root_score, self.tree
        return move

    def update(self, last_player, last_turn):
        # Cut the dead branches
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
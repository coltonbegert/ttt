from bots.base_bot import BaseBot
from math import sqrt, log
from time import time, sleep
from random import choice, shuffle, randint, random
from queue import PriorityQueue
import threading

import numpy as np
import winning_state

class Bot(BaseBot):
    def setup(self, *args):
        """ Called after initialization """
        self.thinking_time = 20
        self.tree = PriorityQueue()
        self.total_sims = 1
        self.lock = threading.Lock()
        self.parent_score = (0,1)

        # arg0 = simulation
        if len(args) > 0:
            sim_name = args[0]
            self.simulation = get_simulator(sim_name)
            print("Using simulation '{}'".format(sim_name))
        else:
            self.simulation = simulation
            print("Using default simulation")

        # arg1 = priority func
        if len(args) > 1:
            name = args[1]
            self.get_priority = get_priority_func(name)
            print("Using priority function '{}'".format(name))
        else:
            self.get_priority = simple_score
            print("Using priority function '{}'".format('simple'))

        # arg2 = thinking time
        if len(args) > 2:
            self.thinking_time = int(args[2])
        print("Using at least {} seconds to think".format(self.thinking_time))

        # arg3 = min turn evaluation
        self.min_evaluations = 300
        if len(args) > 3:
            self.min_evaluations = int(args[3])
        print("Using at least {} move evaluations".format(self.min_evaluations))

        # arg4 = max turn evaluation
        self.max_evaluations = 10000
        if len(args) > 4:
            self.max_evaluations = int(args[4])
        print("Using at most {} move evaluations".format(self.max_evaluations))

    def start(self):
        """ Called after the connection is made """
        self.last_request = time()
        self.choice = None
        self.counter = 0

        print("Getting ready...")
        self.playing = True
        self.thread = threading.Thread(target=self.think)
        self.thread.start()
        print("I'm ready to play!")

    def stop(self):
        self.playing = False

    def think(self):
        while self.playing:
            self.lock.acquire()
            self.search()
            self.lock.release()
            sleep(0.0001)

    def update(self, last_player, last_move):
        """ Called after a move is made """
        self.lock.acquire()
        super(Bot, self).update(last_player, last_move)

        self.parent = self.tree

        # Cut the old tree down
        move = None
        # If we made a choice recently, it probably matches
        if self.choice is not None:
            (_, score, move, subtree) = self.choice

        # Discard root node branches until we find this move
        while move != last_move:
            (_, score, move, subtree) = self.tree.get()

        self.tree = subtree
        self.choice = None
        self.counter = 0
        self.lock.release()

    def dfs_check(self, board, max_depth=2, max_nodes=1000, depth=1, num_checked=None):
        if num_checked is None:
            num_checked = [0]

        if max_depth == 0:
            return None

        if num_checked[0] > max_nodes:
            return None

        # Check current depth
        to_check = []
        # -- Shuffle the options in case we get lucky
        options = board.get_valid()
        shuffle(options)
        for move in options:
            B = board.clone()
            B.move(*move)
            if B.winner == self.player:
                return [move], []
            if B.winner is None:
                to_check.append(move)
            else:
                # Draw or other player win. This branch is dead
                return None
            # Don't over extend
            num_checked[0] += 1
            if num_checked[0] > max_nodes:
                return None
        # Try the next layer
        best_conf = float('inf')
        best_len = float('inf')
        best_path = None
        good_options = []
        for move in to_check:
            B = board.clone()
            B.move(*move)
            search = self.dfs_check(B, max_depth-1, max_nodes, depth+1, num_checked)
            if search is not None:
                moves, _ = search
                path = [move] + moves
                conf = self._lookup(move)
                conf = -float('inf') if conf is None else conf
                good_options.append((len(path), -conf, path))
                if len(path) <= best_len and conf <= best_conf:
                    best_path = path
                    best_len = len(path)
                    best_conf = conf
        if best_path:
            return best_path, sorted(good_options)
        else:
            return None

    def _lookup(self, search_move):
        # Returns the score for a given move in the top-level tree
        self.lock.acquire()
        tmp = []
        result = None
        while not self.tree.empty():
            branch = self.tree.get()
            tmp.append(branch)
            priority, score, move, subtree = branch
            if search_move == move:
                result = priority
        for b in tmp:
            self.tree.put(b)
        self.lock.release()
        return result

    def request(self):
        print("My turn?")
        think_time = self.last_request + (time()-self.last_request)/2

        # If there's a one move win, take it
        # (turn 18 is the first turn in which a player can win)
        board = self.board.clone()
        if board.turns_left < 64:
            dfs = self.dfs_check(board, max_depth=10)
            if dfs is not None:
                path, options = dfs
                conf = None
                if options:
                    length, conf, path = options[0]
                move = path[0]
                if len(path) == 1:
                    print("You're toast!")
                else:
                    print("This seems good...")
                    
                if conf:
                    print("Choosing move {} with confidence {:.3f}".format(move, conf))
                else:
                    print("Choosing move {} for the win".format(move))
                return move

        # Think using MCTS
        def needs_to_think():
            return (self.counter < self.min_evaluations \
                   or time() - think_time < self.thinking_time) \
                   and self.counter < self.max_evaluations

        # Make sure we thought for long enough
        if needs_to_think():
            while needs_to_think():
                print("Hmm", '.'*int(time()%3 + 1), end='    \r', flush=True)
                sleep(.5)
            print("Okay, I got it.")
        else:
            print("Okay")

        # Pick the move that's most likely to win
        self.lock.acquire()
        self.parent_score = self._refresh(self.tree)

        self.choice = self.tree.get()
        self.tree.put(self.choice)

        self.lock.release()

        (priority, score, move, subtree) = self.choice

        print("Choosing move {} with confidence {:.3f} <-- {}".format(move, abs(priority), score))
        print("Evaluated {} moves since last request".format(self.counter))

        # Update the internal state
        self.last_request = time()

        return move

    def _refresh(self, tree, tmp_score=None):
        # Refreshes the priority queue
        if tmp_score is None:
            tmp_score = self.scoring_func
        tmp = []
        parent_score = [0,0]
        while not tree.empty():
            branch = tree.get()
            tmp.append(branch)
            parent_score[0] += branch[1][0]
            parent_score[1] += branch[1][1]
        for (priority, score, move, subtree) in tmp:
            priority = tmp_score(parent_score, score)
            tree.put((priority, score, move, subtree))
        return tuple(parent_score)

    def scoring_func(self, parent_score, score):
        return -self.get_priority(parent_score, score)

    def score_branch(self, branch):
        return self.scoring_func(self.parent_score, branch[1])

    def search(self):
        board = self.board.clone()
        self._search(board, self.tree)
        self.counter += 1

    def _search(self, board, tree, parent_score=(0,0)):
        player = board.player
        self._refresh(tree)

        if not tree.empty():
            # Selection
            (_, score, move, subtree) = tree.get()
            board.move(*move)
            winner = self._search(board, subtree, score)
        else:
            # Expansion
            valid = board.get_valid()
            if len(valid) == 0:
                return board.winner
            shuffle(valid)
            for move in valid[:-1]:
                # (Simulate all branches at least once)
                winner = self.simulation(board.clone())
                win = int(player == winner)
                score = (win,1)
                subtree = PriorityQueue()
                priority = self.scoring_func(parent_score, score)
                tree.put( (priority, score, move, subtree) )

            move = valid[-1]
            score = (0,0)
            subtree = PriorityQueue()

            # Simulation
            board.move(*move)
            winner = self.simulation(board)
            self.total_sims += 1

        # Backprop
        win = int(player == winner)
        wins, samples = score
        wins += win
        samples += 1

        parent_score = parent_score[0] + int((3-player) == winner), parent_score[0]+1
        score = (wins, samples)
        priority = self.scoring_func(parent_score, score)
        tree.put( (priority, score, move, subtree) )

        return winner

def _ucb1(mean, num_plays, total_plays):
    return mean + sqrt(2*log(total_plays) / num_plays)

def get_priority_func(name):
    return {
        'simple': simple_score,
        'ucb': ucb
    }.get(name, simple_score)

def simple_score(parent_score, score):
    return (score[0]+1) / (score[1]+2)

def ucb(parent_score, score):
    wins, played = score

    mean = simple_score(parent_score, score)
    if played == 0:
        return mean

    total_wins, total_played = parent_score
    if total_played == 0:
        return float('inf')

    return mean + sqrt(log(played) / total_played)

# Simulators

def get_simulator(id):
    return {
        'random': sim1,
        'miniwins': sim2,       # Broken?
        'cells': sim3,
        'trials': sim4,
    }.get(id, simulation)

def simulation(board):
    return sim1(board)

def sim1(board):
    while board.winner is None:
        move = choice(board.get_valid())
        board.move(*move)
    return board.winner

def sim2(board):
    state = board._miniwins
    final_state = state + np.random.randint(1,3, state.shape)*(state==0)
    winner = winning_state.winner(final_state)
    return winner

def sim3(board):
    state = board._board
    final_state = state + np.random.randint(1,3, state.shape)*(state==0)
    winner = winning_state.full_winner(final_state)
    return winner

def sim4(board):
    sims = [sim1, sim2, sim3]
    games = [0,0,0]
    for sim in sims:
        win = sim(board.clone())
        games[win] += 1
    return max([0,1,2], key=lambda x: games[x])

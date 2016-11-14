from bots.base_bot import BaseBot
from math import sqrt, log
from time import time, sleep
from random import choice, shuffle, randint
from queue import PriorityQueue
import threading

import numpy as np
import winning_state

class Bot(BaseBot):
    def setup(self, *args):
        """ Called after initialization """
        self.thinking_time = 30
        self.tree = PriorityQueue()
        self.total_sims = 0
        self.lock = threading.Lock()

        if len(args) > 0:
            sim_name = args[0]
            self.simulation = get_simulator(sim_name)
            print("Using simulation '{}'".format(sim_name))
        else:
            self.simulation = simulation
            print("Using default simulation")

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

        move = None
        # If we made a choice recently, it probably matches
        if self.choice is not None:
            (_, score, move, subtree) = self.choice

        # Discard root node branches until we find this move
        while move != last_move:
            (_, score, move, subtree) = self.tree.get()

        self.tree = subtree
        self.choice = None
        self.lock.release()
        print("Done.")

    def request(self):
        print("My turn?")
        sleep(2 + randint(1,5))

        # Make sure we thought for long enough
        if time() - self.last_request < self.thinking_time:
            while time() - self.last_request < self.thinking_time:
                print("Hmm", '.'*int(time()%3 + 1), end='    \r', flush=True)
                sleep(.5)
            print("Okay, I got it now.")
        else:
            print("Okay")

        # Make our move
        self.choice = self.tree.get()
        (priority, score, move, subtree) = self.choice

        print("Chosing move {} with confidence {:.3f} <-- {}".format(move, abs(priority), score))
        print("Evaluated {} moves since last request".format(self.counter))

        # Update the internal state
        self.last_request = time()
        self.counter = 0

        self.thinking_time = self.board.turns_left // 2
        return move

    def get_priority(self, score):
        return -(score[0]+1) / (score[1]+2)

    def search(self):
        board = self.board.clone()
        self._search(board, self.tree)
        self.counter += 1

    def _search(self, board, tree):
        player = board.player

        if not tree.empty():
            # Selection
            (_, score, move, subtree) = tree.get()
            board.move(*move)
            winner = self._search(board, subtree)
        else:
            # Expansion
            valid = board.get_valid()
            if len(valid) == 0:
                return board.winner
            shuffle(valid)
            for move in valid[:-1]:
                score = (0,0)
                subtree = PriorityQueue()
                priority = self.get_priority(score)
                tree.put( (priority, score, move, subtree) )

            move = valid[-1]
            score = (0,0)
            subtree = PriorityQueue()

            # Simulation
            board.move(*move)
            winner = self.simulation(board)
            self.total_sims += 1

        # Backprop
        wins, samples = score
        wins += int(player == winner)
        samples += 1

        score = (wins, samples)
        priority = self.get_priority(score)
        tree.put( (priority, score, move, subtree) )

        return winner

def ucb1(mean, num_plays, total_plays):
    return mean + sqrt(2*log(total_plays) / num_plays)

def get_simulator(id):
    return {
        'random': sim1,
        'cells': sim2,
        'miniwins': sim3,
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

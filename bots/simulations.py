"""
Collection of simulation functions for MCTS
"""
import numpy as np
import winning_state
import random

def random_final(player):
    def _helper(board):
        state = board._board
        final_state = state + np.random.randint(1,3, state.shape)*(state == 0)
        winner = winning_state.full_board_winning_state(final_state, player)
        assert winner != 0
        if winner == 3:
            return None
        else:
            if winner == player:
                print("sim", player, "win")
            return int(winner == player)
    return _helper

def random_mini_final(player):
    def _helper(board):
        state = board._miniwins
        final_state = state + np.random.randint(1,3, state.shape)*(state==0)
        winner = winning_state.winning_state(final_state, player)
        assert winner != 0
        if winner == 3:
            return None
        else:
            return int(winner == player)
    return _helper

def random_game(player):
    def _helper(board):
        while board.winner is None:
            move = random.choice(board.get_valid())
            board.move(*move)
        return None if board.winner == 0 else board.winner == player
    return _helper
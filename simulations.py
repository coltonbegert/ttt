from random import choice
import numpy as np

def random_walk(board):
    while board.winner is None:
        board.move(*choice(board.get_valid()))
    return board.winner

def mini_game(board):
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

    # Random walk
    while board.winner is None and turns > 0:
        options = board.get_valid()
        board.move(*choice(options))
        turns -= 1
    if board.winner is not None:
        return board.winner

    # Perform a mini-win approximation
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

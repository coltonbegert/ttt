import numpy as np

def winner(board):
    # Assumes the game is over and determines the winner
    win = winning_state(board, 1)
    # If tie, return 0
    if win == 3:
        return 0
    # Else, return actual winner
    return 1 if win==1 else 2

def full_winner(board):
    # Assumes the game is over and determines the winner of the 9x9
    win = full_board_winning_state(board, 1)
    if win == 3:
        return 0
    return 1 if win==1 else 2

def winning_state(board, player):
    # check if a state is winning for the given player
    for i in range(3):
        if (board[:, i] == player).all():
            return player
        if (board[i, :] == player).all():
            return player
    if board[1,1] == player:
        if board[0,0] == board[1,1] == board[2,2]:
            return player
        if board[0,2] == board[1,1] == board[2,0]:
            return player

    if (board != 0).all():
        return 3

    return 0

def full_board_winning_state(board, player):
    miniwins = np.zeros((3,3))
    opp = 3 - player
    for br in range(3):
        for bc in range(3):
            miniboard = board[3*br:3*(br+1), 3*bc:3*(bc+1)]
            miniwins[br,bc] = winning_state(miniboard, player) | winning_state(miniboard, opp)
    return winning_state(miniwins, player)
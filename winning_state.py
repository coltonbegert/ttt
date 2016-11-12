import numpy as np

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
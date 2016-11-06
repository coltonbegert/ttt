def winning_state(board, player):
    # check if a state is winning for the given player
    for i in range(3):
        if all(board[:, i] == player):
            return True
        if all(board[i, :] == player):
            return True
    if board[1,1] == player:
        if board[0,0] == board[1,1] == board[2,2]:
            return True
        if board[0,2] == board[1,1] == board[2,0]:
            return True
    return False

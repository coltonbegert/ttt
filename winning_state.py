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
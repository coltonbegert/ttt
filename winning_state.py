def winning_state(board, player):
    # check if a state is winning for the given player
    for i in range(3):
        if all(board[:, i] == player):
            return 1
        if all(board[i, :] == player):
            return 1
    if board[1,1] == player:
        if board[0,0] == board[1,1] == board[2,2]:
            return 1
        if board[0,2] == board[1,1] == board[2,0]:
            return 1

    opponent = (player == 1 : 2 ? 1)
    for i in range(3):
        if all(board[:, i] == opponent):
            return -1
        if all(board[i, :] == opponent):
            return -1
    if board[1,1] == opponent:
        if board[0,0] == board[1,1] == board[2,2]:
            return -1
        if board[0,2] == board[1,1] == board[2,0]:
            return -1


    return 0

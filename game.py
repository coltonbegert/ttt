import board
import human, mcts

def main():
    B = board.Board()
    player1 = human.Human(B)
    player2 = mcts.get_player(B, 2)

    players = [player1, player2]

    for player in players:
        player.start()

    while B.winner is None:
        print("Player", B.player)
        print(B)

        # Get the next move
        bot = players[B.player-1]
        # -- Wait for at least 1 second for the player to think
        move = bot.predict()
        for bot in players:
            bot.update(move)

        # Apply the move
        print("played ({}, {})".format(*move))
        B.move(*move)

    print("\n\n#########")
    if B.winner:
        print("Player", B.winner, "wins!")
    else:
        print("Game is a tie!")
    print("#########")
    print(B)

if __name__ == '__main__':
    main()
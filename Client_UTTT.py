import board
import client
import bots
import argparse

def main(bot, host, port, args):
    B = board.Board()
    c = client.Client(make_bot(B, bot, args), host=host, port=port)
    try:
        c.start()
    finally:
        print('\n########')
        print("Shutting client down...")
        print('########\n')
        c.close()

def make_bot(board, bot, args):
    def _make(player):
        return bot(board, player, *args)
    return _make

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Client module for Ultimate Tic-Tac-Toe")
    parser.add_argument("bot_name", help="Name of the player or bot to use")
    parser.add_argument("--host", default=None, help="Hostname of the host module")
    parser.add_argument("--port", type=int, default=11001, help="Port to communicate over")
    parser.add_argument("bot_args", nargs='*', help="Other arguments for the bot")

    args = parser.parse_args()

    bot = bots.get_bot(args.bot_name)
    main(bot, args.host, args.port, args.bot_args)

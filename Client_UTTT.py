import board
import client
import bots

def main(bot):
    B = board.Board()
    c = client.Client(make_bot(B, bot))
    try:
        c.start()
    finally:
        print('\n########')
        print("Shutting client down...")
        print('########\n')
        c.close()

def make_bot(board, bot):
    def _make(player):
        return bot(board, player)
    return _make

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print("\nUsage: python3 Client_UTTT.py bot_name\n")
        exit(1)
    bot = bots.get_bot(sys.argv[-1])
    main(bot)

import server
import board
import argparse

def main(fname, host, port):
    with open(fname, 'w') as f:
        B = BoardRecorder(f)
        s = server.Server(B, host=host, port=port)
        try:
            s.start()
        finally:
            print('\n########')
            print("Shutting server down...")
            print('########\n')
            s.close()

class BoardRecorder(board.Board):
    def __init__(self, file):
        super(BoardRecorder, self).__init__()
        self._file = file
    def move(self, row, col, player=None):
        super(BoardRecorder, self).move(row, col, player)
        self._file.write('({},{})\n'.format(row,col))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Server module for Ultimate Tic-Tac-Toe")
    parser.add_argument("--host", default=None, help="Hostname of the host module")
    parser.add_argument("--port", type=int, default=11001, help="Port to communicate over")
    parser.add_argument("--replay", default="moves.dat", help="Filename to save the replay to")

    args = parser.parse_args()
    main(args.replay, args.host, args.port)

import server
import board
import traceback

def main():
    with open('moves.dat', 'w') as f:
        B = BoardRecorder(f)
        s = server.Server(B)
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
    main()

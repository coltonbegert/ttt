import server
import board
import traceback

def main():
    B = board.Board()
    s = server.Server(B)
    try:
        s.start()
    finally:
        print('\n########')
        print("Shutting server down...")
        print('########\n')
        s.close()


if __name__ == '__main__':
    main()

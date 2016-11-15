import socket
import threading
import warnings
from socket_helper import *

class Client:
    def __init__(self, bot_const, host=None, port=11001):
        """
        :param bot_const: A bot constructor that creates a bot object
            A bot must implement 2 methods:
                update(last_player, last_move)
                request(valid_moves)
            The bot is responsible for maintaining its own correct
            copy of the board
        """
        self._bot_const = bot_const
        self._bot = None

        if host is None:
            host = socket.gethostname()

        print("Trying to connect to {}:{} ... ".format(host,port), end='',flush=True)
        self._client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._client.connect((host,port))
        print("Success!")

        self._thread = None
        self._closed = False

    def start(self, threaded=False):
        if self._closed:
            raise RuntimeError("Service already closed")
        if threaded:
            if self._thread is None:
                self._thread = threading.Thread(target=self._recv)
                self._thread.start()
        else:
            self._recv()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def _recv(self):
        while True:
            header = recv_int(self._client)
            if header == 0:
                self._receive_move_request()
            elif header == 1:
                self._receive_update()
            elif header == 4:
                self._receive_player_id()
            elif header == 10:
                self._receive_gameover()
                break
            else:
                print("#### WARNING: Received unknown packet type", header)

        self.close()


    def _receive_move_request(self):
        """
        REQUEST_MOVE:
        0: HEADER (0)
        """
        move = self._bot.request()
        self._send_move(move)

    def _receive_player_id(self):
        """
        PLAYER_ID
            0: HEADER (4)
            1: ID (int)
        """
        if self._bot is None:
            player = recv_int(self._client)
            self._bot = self._bot_const(player)
            self._id = player
            self._bot.start()
            print("Bot initialized successfully!")
            print("You are player", player)
        else:
            warnings.warn("Player already has been issued an id. Packet was ignored.", RuntimeWarning)

    def _receive_update(self):
        """
        UPDATE:
            0:      HEADER (1)
            1:      LAST_PLAYER (int)
            2:      SIZEOF(LAST_MOVE) (int)
            3:      LAST_MOVE (position)
        """
        last_player = recv_int(self._client)
        size = recv_int(self._client)
        last_move_data = recv(self._client, size)
        last_move = tuple(p for p in last_move_data)

        self._bot.update(last_player, last_move)

    def _receive_gameover(self):
        print("GAME OVER")
        winner = recv_int(self._client)
        if winner == 0:
            print("Game is a tie!")
        else:
            if self._id == winner:
                print("You win!")
            else:
                print("You lose.")
        self._bot.stop()


    def _send_move(self, move):
        """
        MOVE:
            0:  HEADER (2)
            1:  SIZEOF(OPTION) (int)
            2:  OPTION (int)
        """
        packet = pack(2, len(move), *move)
        send(self._client, packet)

    def close(self):
        if not self._closed:
            self._client.shutdown(0)
            self._client.close()
        self._closed = True
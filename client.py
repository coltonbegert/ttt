import socket
import threading
from socket_helper import *

class Client:
    def __init__(self, bot, host=None, port=11001):
        """
        :param bot: A bot object to call on request
            A bot must implement 2 methods:
                update(last_player, last_move)
                request(valid_moves)
            The bot is responsible for maintaining its own correct
            copy of the board
        """
        self._bot = bot

        if host is None:
            host = socket.gethostname()

        self._client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._client.connect((host,port))

        t = threading.Thread(target=self._recv)
        t.start()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def _recv(self):
        header = int(recv(self._client, 1))
        if header == 0:
            self._receive_move_request()
        if header == 1:
            self._receive_update()

    def _receive_move_request(self):
        """
        REQUEST_MOVE:
        0: HEADER (0)
        1: NUM_OPTIONS (int)
        2: SIZEOF(OPTION) (int)
        3-: OPTIONS* (int*)
        """
        num_options = int(recv(self._client, 1))
        size = int(recv(self._client, 1))
        option_data = recv(self._client, size*num_options)
        options = []
        for i in range(num_options):
            option = option_data[i*size:(i+1)*size]
            option = tuple(map(int, option))
            options.append( option )

        move = self._bot.request(options)
        self._send_move(move)

    def _receive_update(self):
        """
        UPDATE:
            0:      HEADER (1)
            1:      LAST_PLAYER (int)
            2:      SIZEOF(LAST_MOVE) (int)
            3:      LAST_MOVE (position)
        """
        last_player = int(recv(self._client, 1))
        size = int(recv(self._client, 1))
        last_move_data = recv(self._client, size)
        last_move = tuple(map(int, last_move_data))

        self._bot.update(last_player, last_move)

    def _send_move(self, move):
        """
        MOVE:
            0:  HEADER (2)
            1:  SIZEOF(OPTION) (int)
            2:  OPTION (int)
        """
        packet = pack(2, len(move), *move)
        send(packet)

    def close(self):
        self._client.shutdown()
        self._client.close()
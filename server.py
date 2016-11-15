"""
Ultimate Tic-Tac-Toe Board using a sockets API
to enable a more effective multithreading model.

To connect a player, have it connect to the required
port and ip.

Packets:
Move Request (Incoming):
    A request from the game to the player bot for a move, containing a list of the
    options available to it.
        HEADER = 0  (1 byte)
        NUM_OPTIONS (1 byte)
        OPTIONS (NUM_OPTIONS x 1 byte)
Move Update (Incoming):
    Data containing the last accepted move
        HEADER = 1 (1 byte)
        PLAYER (1 byte, 1 or 2)
        OPTION (1 byte)
        BOARD_DATA (81 bytes)
        BOARD_STATE (2 bytes: (player_turn, last_move)

Move Data (Outgoing)
    The move the bot will take after a Move Request packet was issued
        HEADER = 2 (1 byte)
        OPTION (1 byte)

an option should divmod'ed by 9 to get the board row
and board column,
an option should then be repacked into 1-dimensional
form when sending back

Players should connect to port 11001
"""
import socket
import time
from socket_helper import *

class Server:
    def __init__(self, board, host=None, port=11001):
        self._players = []
        self._addresses = []
        self._board = board

        if host is None:
            host = socket.gethostname()

        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server.bind((host, port))
        self._server.listen(2)

        self._closed = False

    def start(self):
        if self._closed:
            raise RuntimeError("Service already closed")

        self._connect()

        while self._board.winner is None:
            player_num = self._board.player
            turn = player_num - 1
            player = self._players[turn]

            print("Player", player_num)
            self._board.pprint()

            self._send_request_move(player)
            move = self._receive_move(player)
            print("Received move:", move)

            self._board.move(*move)

            for sock in self._players:
                self._send_update(sock, turn, move)

        self._board.pprint()
        if self._board.winner == 0:
            print("Game is a tie!")
        else:
            print("Player", self._board.winner, "wins!")

        for sock in self._players:
            self._send_gameover(sock, self._board.winner)

        time.sleep(3)
        self.close()

    def _connect(self):
        count = 0
        while len(self._players) != 2:
            print("Waiting for player", len(self._players)+1, "...", end=' ',flush=True)
            (client, address) = self._server.accept()
            self._players.append(client)
            self._addresses.append(address)

            count += 1
            self._send_player_id(client, count)
            print("Success!")

    def _send_player_id(self, sock, num):
        """
        PLAYER_ID
            0: HEADER (4)
            1: ID (int)
        """
        packet = pack(4, num)
        send(sock, packet)

    def _send_request_move(self, sock):
        """
        REQUEST_MOVE:
            0: HEADER (0)
        """
        packet = pack(0)
        send(sock, packet)

    def _send_update(self, sock, player, move):
        """
        UPDATE:
            0:      HEADER (1)
            1:      LAST_PLAYER (int)
            2:      SIZEOF(LAST_MOVE) (int)
            3:      LAST_MOVE (position)
        """
        packet = pack(1, player, len(move), *move)
        send(sock, packet)

    def _send_gameover(self, sock, winner):
        """
        GAME OVER:
            0:  HEADER (10)
            1:  WINNER (int)
        """
        packet = pack(10, winner)
        send(sock, packet)

    def _receive_move(self, sock):
        """
        MOVE:
            0:  HEADER (2)
            1:  SIZEOF(OPTION) (int)
            2:  OPTION (int)
        """
        header = recv_int(sock)
        assert header == 2, "Control flow issue. Expected to receive move header, got {}".format(header)
        size = recv_int(sock)
        packet = recv(sock, size)
        return tuple(p for p in packet)

    def close(self):
        if not self._closed:
            self._server.shutdown(0)
            self._server.close()
        self._closed = True

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

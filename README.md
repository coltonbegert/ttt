# ttt
ti-tac-toe solver for cmput 396

# Usage

First, run a server instance using:

        python3 Host_UTTT.py

Next, run your players using:

        python3 Client_UTTT.py <client_name> [args*]

where <client_name> is a name of a bot:

* human     - Human player
* mcts      - Pure MCTS implementation
* mctsplus  - Nasty MCTS with tweaks that may or may not help
* random    - Random player (for testing)

The client and host run on INET sockets over port 11001.

# Replayer

The host automatically saves a replay file to moves.dat.
You can play a replay of the match using

        python3 replayer.py

The moves.dat file is a list of tuples.
Each line is exactly 5 characters:

    (r,c)

where r = row, c = col.

You can include markup this file by putting a `#` symbol as the first
character of the line.
Leading and trailing whitespace is ignored.
Empty lines are also ignored.

# Notes

The board object includes a pprint function that marks up
the representation of the board with color strings. If your
terminal does not support these, you may need to change
instances of `board.pprint()` to `print(board)`

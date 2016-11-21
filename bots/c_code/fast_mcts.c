/*
 * fast_mcts.c
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 *
 * This module provides a fast interface for running MCTS.
 * It is designed to by imported by python and called
 * as a regular module.
 */

#include <stdlib.h>
#include <time.h>

#include "fast_mcts.h"
#include "board.h"

board_t board;
void setup(int argc, char** argv) {
	uttt_init(&board);
}
void start() {
	uttt_init(&board);
}
void stop() {
}
void update(int last_player, move_t last_move) {
	uttt_move(&board, last_move.row, last_move.col);

	uttt_print(&board);
}
move_t request() { 
	srand(time(NULL));
	int options[81];
	int N = uttt_get_valid(&board, options);
	
	move_t move;
	uttt_raw_to_rowcol(options[rand()%N], &move.row, &move.col);
	printf("Sending move %d, %d\n", move.row, move.col);

	return move;
}


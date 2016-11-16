/*
 * board.h
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef BOARD_H
#define BOARD_H

#include <pthread.h>

typedef struct {
    int board[9][9];
    int miniwins[3][3];
    int player;
    int turns_left;
    int last_row;
    int last_col;
    int winner;
    pthread_mutex_t lock;
} board_t;

// Given a pointer to allocated memory holding a board
void uttt_init(board_t* board);

// clones a board into memory. Only locks the source
void uttt_clone(board_t* board, board_t* target);

void _uttt_apply_move(board_t* board, int row, int col);
// Apply a move to the board
void uttt_move(board_t* board, int row, int col);

// Apply a move to the board without locking the mutex
void uttt_move_unsafe(board_t* board, int row, int col);

// Gets a list of valid moves in raw position format
// and stores them in buffer. Returns the number of
// valid moves found.
int uttt_get_valid(board_t* board, int* buffer);

void uttt_raw_to_rowcol(int raw, int* row, int* col);

// Check if a player has won
int _uttt_check_win(board_t* board);

int _uttt_check_miniwin(board_t *board, int row, int col, int player);

void uttt_print(board_t *board);

#endif /* !BOARD_H */

/*
 * board.c
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#define _uttt_lock(x) pthread_mutex_lock(&x->lock)
#define _uttt_unlock(x) pthread_mutex_lock(&x->lock)

#include "board.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Given a pointer to allocated memory holding a board
void uttt_init(board_t* board) {
    _uttt_lock(board);
    memset(&board->board, 0, sizeof(board->board));
    memset(&board->miniwins, -1, sizeof(board->miniwins));
    board->last_row = -1;
    board->last_col = -1;
    board->turns_left = 81;
    board->player = 1;
    board->winner = -1;
    _uttt_unlock(board);
}

// clones a board into memory. Only locks the source
void uttt_clone(board_t* board, board_t* target) {
    _uttt_lock(board);
    memcpy(target, board, sizeof(board_t));
    _uttt_unlock(board);
}

void _uttt_apply_move(board_t* board, int row, int col) {
    board->board[row][col] = board->player;

    int player = board->player;
    int br = row / 3;
    int bc = col / 3;

    int miniwin = _uttt_check_miniwin(board, row, col, player);
    board->miniwins[br][bc] = miniwin;

    if (miniwin >= 0) {
        printf("--> Miniwin for player %d\n", miniwin);
        int winner = _uttt_check_win(board);
        board->winner = winner;
        if (winner >= 0) {
            printf("--> Win for player %d\n", winner);
            board->turns_left = 0;
        }
    }

    board->player = 3^player;
    board->turns_left--;

    board->last_row = row;
    board->last_col = col;
}

// Apply a move to the board
void uttt_move(board_t* board, int row, int col) {
    _uttt_lock(board);
    _uttt_apply_move(board, row, col);
    _uttt_unlock(board);
}

// Apply a move to the board without locking the mutex
void uttt_move_unsafe(board_t* board, int row, int col) {
    _uttt_apply_move(board, row, col);
}

// Check if the move just played was a miniwin
int _uttt_check_miniwin(board_t* board, int row, int col, int player) {
    int br = row / 3;
    int bc = col / 3;
    int mr = row % 3;
    int mc = col % 3;
    
    // Check row
    if (board->board[3*br][col] == player &&
            board->board[3*br+1][col] == player &&
            board->board[3*br+2][col] == player)
        return player;

    // Check col
    if (board->board[row][3*bc] == player &&
            board->board[row][3*bc+1] == player &&
            board->board[row][3*bc+2] == player)
        return player;

    // Check diagonal
    if (mr == 1 || mc == 1) {
        // If played in center, check opposite corners
        if (board->board[3*br][3*bc] == player &&
                board->board[3*br+2][3*bc+2] == player)
            return player;
        if (board->board[3*br+2][3*bc] == player &&
                board->board[3*br][3*bc+2] == player)
            return player;
    } else {
        // If played in corner, check the center next
        if (board->board[3*br+1][3*bc+1] == player) {
            // ...then check the other corner
            if (board->board[3*br+mr^2][3*bc+mc^2] == player)
                return player;
        }
    }

    // Check for non-empty cells
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board->board[3*br+i][3*bc+j] == 0)
                return -1;
        }
    }
    return 0;
}

// Gets a list of valid moves in raw position format
// and stores them in buffer. Returns the number of
// valid moves found.
int uttt_get_valid(board_t* board, int* buffer) {
    int r = board->last_row;
    int c = board->last_col;

    int k = 0;
    // If first turn always return the default.
    // Separate this so the compiler can 
    if (r < 0 || c < 0) {
        k = 81;
        for (int i = 0; i < k; i++)
            buffer[i] = i;
        return 81;
    }
    // If last move puts you in a miniwin
    if (board->miniwins[r%3][c%3] >= 0) {
        printf("--> Getting all available moves\n");
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                // Skip miniwinss that have already ended
                if (board->miniwins[i/3][j/3] >= 0)
                    continue;
                // Add all open squares
                if (board->board[i][j] == 0)
                    buffer[k++] = 9*i+j;
            }
        }
    } else {
        int br = r % 3;
        int bc = c % 3;
        printf("--> Getting moves from board %d, %d\n", br,bc);
        for (int i = 3*br; i < 3*(br+1); i++) {
            for (int j = 3*bc; j < 3*(bc+1); j++) {
                if (board->board[i][j] == 0)
                    buffer[k++] = 9*i+j;
            }
        }
    }
    return k;
}

void uttt_raw_to_rowcol(int raw, int* row, int* col) {
    *row = raw / 9;
    *col = raw % 9;
}

// Check if a player has won
int _uttt_check_win(board_t* board) {
    int num_boards = 0;
    for (int i=0; i<3; i++) {
        // Count number of available boards
        for (int j = 0; j < 3; j++) {
            num_boards += (board->miniwins[i][j] == -1);
        }

        // Check row
        if (board->miniwins[i][0] > 0) {
            if (board->miniwins[i][0] == board->miniwins[i][1] &&
                    board->miniwins[i][1] == board->miniwins[i][2])
                return board->miniwins[i][0];
        }
        // Check col
        if (board->miniwins[0][i] > 0) {
            if (board->miniwins[0][i] == board->miniwins[1][i] &&
                    board->miniwins[1][i] == board->miniwins[2][i])
                return board->miniwins[0][i];
        }
    }
    // Check diagonals
    if (board->miniwins[1][1] > 0) {
        if (board->miniwins[0][0] == board->miniwins[2][2] &&
                board->miniwins[0][0] == board->miniwins[1][1])
            return board->miniwins[0][0];
        if (board->miniwins[2][0] == board->miniwins[0][2] &&
                board->miniwins[2][0] == board->miniwins[1][1])
            return board->miniwins[2][0];
    }

    // Return 0 if nowhere left to play
    return num_boards == 0 ? 0 : -1;
}

#define NO_WIN "\033[0;37;50m%s\033[0;0;0m"
#define P0_WIN "\033[0;30;50m%s\033[0;0;0m"
#define P1_WIN "\033[0;36;50m%s\033[0;0;0m"
#define P2_WIN "\033[0;35;50m%s\033[0;0;0m"
#define LAST_PLAY "\033[1;33;50m%s\033[0;0;0m"
#define ACTIVE "\033[1;33;50m%s\033[0;0;0m"
void uttt_print(board_t *board) {
    char sym[3] = " XO";

    const char* colors[] = {NO_WIN, P0_WIN, P1_WIN, P2_WIN};

    char buffer[200];

    for (int br = 0; br < 3; br++){
        for (int mr = 0; mr < 3; mr++){
            for (int bc = 0; bc < 3; bc++){
                printf(" ");
                int r = br*3 + mr;
                for (int mc = 0; mc < 3; mc++) {
                    int c = bc*3 + mc;
                    int p = board->board[r][c];

                    if (board->last_row == r && board->last_col == c) {
                        sprintf(buffer, "%c", sym[p]);
                        printf(LAST_PLAY, buffer);
                    } else {
                        printf("%c", sym[p]);
                    }

                    if (mc < 2) {
                        int miniwin = board->miniwins[br][bc]+1;
                        if (miniwin < 0)
                            printf("|");
                        else {
                            sprintf(buffer, "|");
                            printf(colors[miniwin], buffer);
                        }
                    }
                }
            }
            printf("\n");
            if (mr < 2) {
                for (int bc = 0; bc < 3; bc++) {
                    int miniwin = board->miniwins[br][bc]+1;
                    if (miniwin < 0)
                        printf(" -+-+-");
                    else {
                        sprintf(buffer, " -+-+-");
                        printf(colors[miniwin], buffer);
                    }
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}

#undef _uttt_lock
#undef _uttt_unlock

/*
 * test.c
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
    board_t* board = malloc(sizeof(board_t));
    uttt_init(board);
    srand(time(NULL));

    int options[81];
    int n;
    while (board->winner < 0 && board->turns_left > 0) {
        uttt_print(board);
        n = uttt_get_valid(board, options);
        int row, col;
        uttt_raw_to_rowcol(options[rand() % n], &row, &col);
        printf("Found %d valid moves\n", n);
        printf("Move: %d, %d\n", row, col);
        uttt_move(board, row, col);
    }
    printf("Final board:\n");
    uttt_print(board);
    free(board);
    return 0;
}


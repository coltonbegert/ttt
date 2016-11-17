/*
 * benchmark.c
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 *
 * Tests how fast simulations can run
 */

#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, const char** argv) {
    if (argc == 1) {
        printf("Usage: benchmark N\n");
        printf("\tN - The number of simulations to run\n");
        return 1;
    }
    int iters;
    sscanf(argv[1], "%d", &iters);
    printf("Simulating %d games...\n", iters);

    int wins[3] = {0,0,0};

    int options[81];
    int n;
    int row, col;

    srand(time(NULL));

    for (int i = 0; i < iters; i++) {
        board_t board;
        uttt_init(&board);

        while (board.turns_left > 0) {
            // Pick a random move of the n options
            n = uttt_get_valid(&board, options);
            int move = options[rand()%n];
            // Apply the move
            uttt_raw_to_rowcol(move, &row, &col);
            uttt_move(&board, row, col);
        }

        wins[board.winner]++;
    }

    printf("Done.\n");
    printf("Ties: %d\nPlayer 1 Wins: %d\nPlayer 2 wins: %d\n",
            wins[0],
            wins[1],
            wins[2]);
    return 0;
}


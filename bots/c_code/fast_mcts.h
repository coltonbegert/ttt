/*
 * fast_mcts.h
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef FAST_MCTS_H
#define FAST_MCTS_H

#include "board.h"

typedef struct tree_node_t {
    int player;
    int row;
    int col;

    float mean;
    int visits;

    struct tree_node_t* parent;

    int num_children;
    struct tree_node_t** children;
} tree_node_t;

float lct(float mean, int visits, int total_visits);
float uct(float mean, int visits, int total_visits);

void* worker(void* args);
void start_threads(void);
void stop_threads(void);

tree_node_t* find_node(tree_node_t* node, int row, int col);
tree_node_t* pick_best(tree_node_t* node);
tree_node_t* select_best(tree_node_t* node);

void selection(board_t* board, tree_node_t* tree, tree_node_t** leaf);
int expand(board_t* board, tree_node_t* leaf, tree_node_t** node);
int simulate(board_t* board);
void backprop(int player, int winner, tree_node_t* leaf);
void prune(tree_node_t* branch);
void cut_branch(tree_node_t* branch);

#endif /* !FAST_MCTS_H */

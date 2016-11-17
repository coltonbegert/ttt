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
#define MIN_SEARCHES 500000

#include "fast_mcts.h"

#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <math.h>
#include "board.h"
#include "cpybot.h"

static board_t board;
static tree_node_t* tree;

int searches = 0;
int num_states = 0;

void setup(int argc, char** argv) {
	uttt_init(&board);
    
    tree = malloc(sizeof(tree_node_t));
    tree->player = board.player;
    tree->mean = 0.0f;
    tree->children = NULL;
    tree->num_children = 0;
    tree->visits = 0;
    tree->parent = NULL;

	srand(time(NULL));
}
void start() {
    start_threads();
}
void stop() {
    stop_threads();
}
void update(int last_player, move_t last_move) {
    stop_threads();

    if (board.player != last_player)
        printf("ERROR: Did not simulate the right board\n");

	uttt_move(&board, last_move.row, last_move.col);

    tree_node_t* new_tree = find_node(tree, last_move.row, last_move.col);
    if (new_tree == NULL) {
        printf("Could not find child (pruned?). Creating new branch\n");

        new_tree = malloc(sizeof(tree_node_t));
        new_tree->player = board.player;
        new_tree->mean = 0.0f;
        new_tree->children = NULL;
        new_tree->num_children = 0;
        new_tree->visits = 0;
        new_tree->parent = NULL;

    }

    if (board.player != new_tree->player)
        printf("ERROR: Did not predict the right player. Expected %d, got %d\n", board.player, new_tree->player);

    int old_states = num_states;
    int N = tree->num_children;
    for (int i=0; i<N; i++) {
        if (new_tree != tree->children[i]) {
            cut_branch(tree->children[i]);
        }
    }
    if (tree->num_children != 1)
        printf("ERROR: tree has %d children instead of 1\n", tree->num_children);
    printf("    Cutting branches: %d states -> ", old_states);
    printf("    %d states\n", num_states);

    free(tree->children);
    free(tree);
    tree = new_tree;
    tree->parent = NULL;

    start_threads();
}
move_t request() { 
    while (searches < MIN_SEARCHES) {
        printf("Still thinking.  \r");
        usleep(100000);
        printf("Still thinking.. \r");
        usleep(100000);
        printf("Still thinking...\r");
        usleep(100000);
    }
    stop_threads();

    printf("Getting best...\n");
    tree_node_t* node = pick_best(tree);
    printf("Found move.\n");
    move_t move;
    move.row = node->row;
    move.col = node->col;
    
    float conf = uct(node->mean, node->visits, tree->visits);
    printf("Picking move (%d, %d) with confidence %.3f", move.row, move.col, conf);
    printf("    Visited node %d times\n", node->visits);
	
    start_threads();
	return move;
}


// Number of worker threads
#define num_threads 1

// Signal to the workers to stop working
static int working = 1;
pthread_t worker_thread[num_threads];

void* worker( void* argument ) {
    tree_node_t* leaf;
    tree_node_t* node;

    int winner;
    board_t board_copy;

    while (working) {
        uttt_clone(&board, &board_copy);
        // MCTS
        selection(&board_copy, tree, &leaf);
        int player = board_copy.player;
        num_states += expand(&board_copy, leaf, &node);
        winner = simulate(&board_copy);
        backprop(player, winner, node);

        if (++searches % 20000 == 0) {
            prune(tree);
        }
    }
    return NULL;
}

float lct(float mean, int visits, int total_visits) {
    if (visits == 0 || total_visits == 0)
        return -INFINITY;
    return mean; //- sqrtf(logf((float)total_visits) / ((float)visits));
}

float uct(float mean, int visits, int total_visits) {
    if (visits == 0 || total_visits == 0)
        return INFINITY;
    return mean + 1.414*sqrtf(logf((float)total_visits) / ((float)visits));
}

tree_node_t* find_node(tree_node_t* parent, int row, int col) {
    tree_node_t* child;
    for (int i = 0; i < parent->num_children; i++) {
        child = parent->children[i];
        if (child->row == row && child->col == col) return child;
    }
    printf("Warning: Could not find child\n");
    return NULL;
}

tree_node_t* pick_best(tree_node_t* parent) {
    tree_node_t* child;
    int best = 0;
    float best_score = -INFINITY;
    float score;
    printf("Picking best choice of %d nodes\n", parent->num_children);
    for (int i = 0; i < parent->num_children; i++) {
        child = parent->children[i];
        score = lct(child->mean, child->visits, parent->visits);
        printf("(%d,%d) ", child->row, child->col);
        printf("mean = %.3f, conf = [%.3f,%.3f]; ", child->mean, score, uct(child->mean, child->visits, parent->visits));
        printf("visits = %d / %d\n", child->visits, parent->visits);
        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }
    return parent->children[best];
}

tree_node_t* select_best(tree_node_t* parent) {
    tree_node_t* child;
    int best = 0;
    float best_score = -INFINITY;
    float score;
    for (int i = 0; i < parent->num_children; i++) {
        child = parent->children[i];
        score = uct(child->mean, child->visits, parent->visits);
        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }
    return parent->children[best];
}

void selection(board_t* game, tree_node_t* tree, tree_node_t** node) {
    *node = tree;
    while ((*node)->num_children > 0) {
        *node = select_best(*node);
        uttt_move(game, (*node)->row, (*node)->col);
    }
}

int expand(board_t* game, tree_node_t* leaf, tree_node_t** node) {
    int options[81];
    int N = uttt_get_valid(game, options);
    
    if (N == 0) {
        return 0;
    }
    leaf->num_children = N;
    leaf->children = calloc(N, sizeof(tree_node_t*));

    int row, col;
    for (int i = 0; i < N; i++) {
        uttt_raw_to_rowcol(options[i], &row, &col);

        tree_node_t* child = malloc(sizeof(tree_node_t));

        // NOTE: wtf?
        child->player = 3^game->player;
        child->num_children = 0;
        child->children = NULL;
        child->mean = 0.0f;
        child->visits = 0;
        child->parent = leaf;
        child->row = row;
        child->col = col;
        
        leaf->children[i] = child;
    }

    *node = leaf->children[rand()%N];
    uttt_move(game, (*node)->row, (*node)->col);

    return N;
}

int simulate(board_t* game) {
    srand(time(NULL));
    int options[81];
    int row, col;
    while (game->turns_left > 0) {
        int n = uttt_get_valid(game, options);
        uttt_raw_to_rowcol(options[rand()%n], &row, &col);
        uttt_move(game, row, col);
    }
    return game->winner;
}

void backprop(int player, int winner, tree_node_t* node) {
    if (winner == 0) {
        // backprop a tie
        while (node != NULL) {
            node->visits++;
            node->mean -= node->mean / node->visits;
            node = node->parent;
        }
    } else {
        // Backprop winner/loser
        while (node != NULL) {
            int amtP = winner == node->player ? 1 : 0;
            node->visits++;
            node->mean += (amtP - node->mean) / node->visits;
            node = node->parent;
        }
    }
}

void start_threads(void) {
    printf("Starting threads...");
    working = 1;
    for (int i=0; i < num_threads; i++) {
        fflush(stdout);
        pthread_create(&worker_thread[i], NULL, worker, NULL);
    }
    printf("OK.\n");
}

void stop_threads(void) {
    working = 0;
    printf("Stopping threads...");
    for (int i=0; i < num_threads; i++) {
        fflush(stdout);
        pthread_join(worker_thread[i], NULL);
    }
    printf("OK.\n");

    printf("    Searches ran: %d\n", searches);
    searches = 0;
}

// Wipe out really bad tree values to save memory
void prune(tree_node_t* branch) {
    return;
    if (branch->num_children == 0)
        return;

    int old_states = num_states;
    float best_lcb = -INFINITY;
    float worst_ucb = INFINITY;

    float lower;
    float upper;
    tree_node_t* child;
    tree_node_t* best;
    tree_node_t* worst;
    int pos = 0;
    for (int i = 0; i < branch->num_children; i++) {
        child = branch->children[i];
        lower = lct(child->mean, child->visits, branch->visits);
        upper = uct(child->mean, child->visits, branch->visits);

        if (lower > best_lcb) {
            best_lcb = lower;
            best = child;
        }
        if (upper < worst_ucb) {
            worst_ucb = upper;
            worst = child;
            pos = i;
        }
    }

    // If the best option is significantly better
    // than the worst option, delete the branch entirely.
    if (best_lcb > worst_ucb) {
        cut_branch(worst);
        branch->num_children -= 1;
        // Shift the children to fill the gap
        for (int i = pos; i < branch->num_children; i++) {
            branch->children[i] = branch->children[i+1];
        }
    }
    // Prune each child, too
    for (int i = 0; i < branch->num_children; i++) {
        prune(branch->children[i]);
    }
}

// Properly remove a child and all of its descendents
void cut_branch(tree_node_t* branch) {
    if (branch == NULL) return;

    if (branch->num_children > 0) {
        int N = branch->num_children;
        for (int i=0; i < N; i++) {
            cut_branch(branch->children[i]);
        }
        if (branch->num_children > 0)
            printf("ERROR: branch did not delete all its children\n");
    }
    num_states--;
    if (branch->parent != NULL) branch->parent->num_children--;
    if (branch->children != NULL) free(branch->children);
    free(branch);
}

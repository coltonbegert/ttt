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
#define MIN_SEARCHES 10000
#define MAX_STATES 10000000
#define TIMEOUT 30
#define UCB_CONST 1
#define LCB_CONST -1
#define PICK_CONST -.5
#define SELECT_CONST 1

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

int count_children(tree_node_t* node) {
    if (node == NULL) return 0;

    int count = 1;
    for (int i = 0; i < node->num_children; i++) {
        count += count_children(node->children[i]);
    }
    return count;
}

void setup(int argc, char** argv) {
	uttt_init(&board);
    
    tree = malloc(sizeof(tree_node_t));
    num_states++;

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

    int old_states = num_states;
    int cut = 0;

    int tree_size = count_children(tree);
    printf("tree size = %d, num_states = %d\n", tree_size, num_states);

    if (board.player != last_player)
        printf("ERROR: Did not simulate the right board\n");

    show_choices(tree);

	uttt_move(&board, last_move.row, last_move.col);

    tree_node_t* node = find_node(tree, last_move.row, last_move.col);
    if (node == NULL) {
        printf("\n#####\n");
        printf("Could not find child (pruned?). Creating new branch\n");
        printf("Chopping down the tree\n");
        printf("#####\n\n");

        node = malloc(sizeof(tree_node_t));
        num_states++;

        node->player = board.player;
        node->mean = 0.0f;
        node->children = NULL;
        node->num_children = 0;
        node->visits = 0;
        node->parent = NULL;

        cut = cut_branch(tree);
    } else {
        float conf = uct(node->mean, node->visits,
                node->visits, PICK_CONST);
        printf("\nMove selected was (%d, %d) with confidence %.3f",
                last_move.row, last_move.col, conf);

        printf("    Visited node %d times\n", node->visits);

        int N = tree->num_children;
        for (int i=0; i<N; i++) {
            if (node != tree->children[i]) {
                cut += cut_branch(tree->children[i]);
            }
        }

        free(tree->children);
        free(tree);
        num_states--;
    }

    printf(" Cut %d states (%d->%d)\n", cut, old_states, num_states);

    if (board.player != node->player)
        printf("ERROR: Did not predict the right player. Expected %d, got %d\n", board.player, node->player);

    tree = node;
    tree->parent = NULL;

    start_threads();
}
move_t request() { 
    int timeout = 0;
    while (timeout++ < TIMEOUT && num_states < MAX_STATES && searches < MIN_SEARCHES) {
        printf("Still thinking.  \r");
        fflush(stdout);
        usleep(333333);
        printf("Still thinking.. \r");
        fflush(stdout);
        usleep(333333);
        printf("Still thinking...\r");
        fflush(stdout);
        usleep(333334);
    }
    stop_threads();

    printf("Getting best...\n");
    tree_node_t* node = select_best(tree, PICK_CONST);
    printf("Found move.\n");
    move_t move;
    move.row = node->row;
    move.col = node->col;
	
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

    // FIXME: loops forever if num_states >= MAX_STATES
    while (working && num_states < MAX_STATES) {
        uttt_clone(&board, &board_copy);
        // MCTS
        selection(&board_copy, tree, &leaf);
        int player = board_copy.player;
        expand(&board_copy, leaf, &node);
        winner = simulate(&board_copy);
        backprop(player, winner, node);
        
        if (++searches % 50000 == 0) {
            prune(tree);
        }
    }
    return NULL;
}

float uct(float mean, int visits, int total_visits, const float coeff) {
    if (coeff == 0.0f) return mean;
    if (visits == 0 || total_visits == 0) return INFINITY;
    return mean + coeff*sqrtf(logf((float)total_visits) / ((float)visits));
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

tree_node_t* select_best(tree_node_t* parent, const float coeff) {
    tree_node_t* child;
    int best = 0;
    float best_score = -INFINITY;
    float score;
    for (int i = 0; i < parent->num_children; i++) {
        child = parent->children[i];
        score = uct(child->mean, child->visits, parent->visits, coeff);
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
        *node = select_best(*node, SELECT_CONST);
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
        num_states++;

        child->player = game->player;
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
    int n = uttt_get_valid(game, options);
    for (int i = 0; i < n; i++) {
        board_t test;
        uttt_clone(game, &test);
        uttt_raw_to_rowcol(options[i], &row, &col);
        uttt_move(&test, row, col);
        if (test.turns_left == 0 && test.winner > 0)
            return test.winner;
    }
    while (game->turns_left > 0) {
        n = uttt_get_valid(game, options);
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
    printf("    Total States: %d\n", num_states);
    searches = 0;
}

// Wipe out really bad tree values to save memory
void prune(tree_node_t* branch) {
    if (branch->num_children == 0)
        return;

    float best_lcb = -INFINITY;
    float worst_ucb = INFINITY;

    float lower;
    float upper;
    tree_node_t* child;
    tree_node_t* worst = NULL;
    int pos = 0;
    for (int i = 0; i < branch->num_children; i++) {
        child = branch->children[i];
        lower = uct(child->mean, child->visits, branch->visits, LCB_CONST);
        upper = uct(child->mean, child->visits, branch->visits, UCB_CONST);

        if (lower > best_lcb) {
            best_lcb = lower;
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
int cut_branch(tree_node_t* branch) {
    if (branch == NULL) return 0;

    int cut = 0;
    int N = branch->num_children;
    for (int i=0; i < N; i++) {
        cut += cut_branch(branch->children[i]);
    }

    if (branch->parent != NULL) branch->parent->num_children--;
    if (branch->children != NULL) free(branch->children);
    
    free(branch);
    num_states--;
    cut++;
    
    return cut;
}

void show_choices(tree_node_t* branch) {
    tree_node_t* child;
    float lcb, ucb;
    for (int i = 0; i < branch->num_children; i++) {
        child = branch->children[i];
        lcb = uct(child->mean, child->visits, branch->visits, LCB_CONST);
        ucb = uct(child->mean, child->visits, branch->visits, UCB_CONST);

        printf("-- (%d, %d) -> mean = %.3f in [%.3f, %.3f] ",
                child->row, child->col, child->mean, lcb, ucb);
        printf("with %d visits\t(%.2f%%)\n",
                child->visits, 100.0*child->visits / branch->visits);
    }
}

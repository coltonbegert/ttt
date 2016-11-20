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

// Basic settings of the search program
// MAX_STATES limits the amount of memory it uses,
// A state uses about 16 Bytes of memory. Using a value
// that is too high may slow down the pruning process. If you
// increase this number significantly, consider disabling
// pruning.
// Expect about 10,000,000 per GB
// TIMEOUT prevents the bot from spending too long thinking
//  (does not count pruning time)
// MIN_SEARCHES ensures the bot considered enough options
#define MIN_SEARCHES 100000
#define MAX_STATES 10000000
#define TIMEOUT 20

// Coefficients for different calculations of UCT
// UCB_CONST and LCB_CONST are used for printing statistics
// PICK_CONST is the value to use when picking a move
// SELECT_CONST is the value to use in MCTS selection step
#define UCB_CONST 1
#define LCB_CONST -1
#define PICK_CONST -0.1
#define SELECT_CONST 1.414

// ** Simulation **
// Using a random roll simulation is fast enough for many
// evaluations, but some heuristics can greatly improve the
// accuracy of the statistics

// Rapid Simulate (Simulation)
// Use a heuristic on the first RAPID_SEARCHES iterations
// This greatly improves the statistics with low cost
// but the heuristic may be innacurate.
#define RAPID_SEARCHES 10000

// ** Pruning **
// Pruning allows the bot to consider more states with
// finite amount of memory. If the opponent is quick enough,
// we'll usually cut away the tree before it matters. But
// it can still help to remove moves that are unlikely to
// ever be influential from the board to improve search efficiency.
// The trade off is that pruning is a very slow operation
// and it maybe better to simply use more simulations.
#define USB_LCR_PRUNING 1
#define USB_AB_PRUNING 1

// Min number of searches before trying to prune
#define PRUNE_TIMER 50000
// Minimum number of states to use before pruning
#define MIN_PRUNE_STATES 20000

// Low-Confidence Removal (Pruning)
// Removes moves that have a low chance of winning
// based on the available stats.
// Based on hypothesis testing. If a move's upper-confidence
// bound does not capture the lower confidence bound of the
// best move, it is rejected and all of its children are
// evicted from the tree.
// Fast but not guaranteed to be safe.
#define SIG_UCB_CONST 0.95
#define SIG_LCB_CONST -0.95
#define MIN_CUT_VISITS 2000

// Alpha-Beta Pruning (Pruning)
// Slow but guaranteed to produce good trees.
// Propogates in a bottom-up fashion.
// Might delete valuable search results if the
// opponent does not play perfectly.
#define AB_MIN_TURNS_LEFT 50

#include "fast_mcts.h"

#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <math.h>
#include "board.h"
#include "cpybot.h"

static board_t board;
static tree_node_t* tree;
int options[81];

int searches = 0;
int num_sims = 0;
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
    srand(time(NULL));

	uttt_init(&board);
    
    tree = malloc(sizeof(tree_node_t));
    num_states++;

    tree->is_win = 0;
    tree->player = board.player;
    tree->mean = 0.0f;
    tree->children = NULL;
    tree->num_children = -1;
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

    tree_node_t* node = find_node(tree, last_move.row, last_move.col);
    if (node == NULL) {
        printf("\n##########\n");
        printf("Got very surprising move!!\n\n");
        printf("Did not expect (%d, %d) to be a good move (pruned).\n",
                last_move.row, last_move.col);
        printf("Chopping down the search tree and restarting\n");
        printf("###########\n\n");

        node = malloc(sizeof(tree_node_t));
        num_states++;

        node->is_win = 0;
        node->player = board.player;
        node->mean = 0.0f;
        node->children = NULL;
        node->num_children = -1;
        node->visits = 0;
        node->parent = NULL;

        cut = cut_branch(tree);
    } else {
        float conf = uct(node->mean, node->visits,
                node->visits, PICK_CONST);
        printf("\nMove selected was (%d, %d) with confidence %.3f",
                last_move.row, last_move.col, conf);

        printf("\n\t-->Visited node %d times\n", node->visits);

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

    printf(" Cut %d states (%d->%d) [%.2f%% misprediction rate]\n",
            cut, old_states, num_states, 100.0*cut / old_states);

    tree = node;
    tree->parent = NULL;

    if (board.player != node->player)
        printf("ERROR: Did not predict the right player. Expected %d, got %d\n", board.player, node->player);

	uttt_move(&board, last_move.row, last_move.col);

    start_threads();
}
move_t request() { 
    int timeout = 0;
    printf("Thinking...\r");
    fflush(stdout);
    while (tree->num_children <= 0 || 
            (timeout++ < TIMEOUT && searches < MIN_SEARCHES)) {
        sleep(1);
    }
    if (timeout >= TIMEOUT) {
        printf("Timed out.\n");
    } else {
        printf("OK.\n");
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

    int use_ab = USB_AB_PRUNING;
    int use_lcr = USB_LCR_PRUNING;
    int can_prune = use_ab || use_lcr;
    int prune_timer = PRUNE_TIMER;

    while (working) {
        searches++;
        uttt_clone(&board, &board_copy);
        // MCTS with pruning
        // ** Selection **
        selection(&board_copy, tree, &leaf);
        // ** Expansion **
        int player = board_copy.player;
        if (num_states < MAX_STATES)
            expand(&board_copy, leaf, &node);
        else
            node = leaf;
        // ** Simulation **
        if (searches < RAPID_SEARCHES) { 
            winner = rapid_simulate(&board_copy);
            num_sims++;
        } else {
            winner = simulate(&board_copy);
            num_sims++;
        }
        // ** Backprop **
        backprop(player, winner, node);
        
        // ** Pruning **
        if (can_prune && searches % prune_timer == 0 &&
                num_states > MIN_PRUNE_STATES) {
            int old_states = num_states;
            printf("Pruning...\n");

            int total_pruned = 0;
            int pruned;

            if (use_ab) {
                // Try alpha-beta pruning
                pruned = prune(tree, &board);
                printf("--> AB-Pruned %d nodes.\n", pruned);
                // If there was no improvement, we probably can't
                // improve later.
                if (pruned == 0) use_ab = 0;
                total_pruned += pruned;
            }

            if (use_lcr) {
                pruned = remove_low_conf(tree);
                printf("--> Low-Conf Pruned %d nodes.\n", pruned);
                // If we reduced the states, we can use AB again.
                if (pruned > 0) use_ab = USB_AB_PRUNING;
                total_pruned += pruned;
            }

            if (total_pruned == 0) {
                prune_timer <<= 1;
                printf("Prune ineffective. Waiting %d searches\n", prune_timer);
            } else {
                prune_timer = PRUNE_TIMER;
                printf("Reduced from %d -> %d\n", old_states, num_states);
            }
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
    int N = uttt_get_valid(game, options);

    *node = leaf;
    if (N > 0) {
        leaf->num_children = N;
        leaf->children = calloc(N, sizeof(tree_node_t*));

        int row, col;
        for (int i = 0; i < N; i++) {
            uttt_raw_to_rowcol(options[i], &row, &col);

            tree_node_t* child = malloc(sizeof(tree_node_t));
            num_states++;

            child->is_win = 0;
            child->player = game->player;
            child->num_children = -1;
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
    }

    return game->winner;
}

int rapid_simulate(board_t* game) {
    if (game->turns_left == 0) return game->winner;

    int moves = 10;
    int row, col, n;
    while (moves-->0 && game->turns_left > 0) {
        n = uttt_get_valid(game, options);
        uttt_raw_to_rowcol(options[rand()%n], &row, &col);
        uttt_move(game, row, col);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game->miniwins[i][j] == -1)
                game->miniwins[i][j] = rand() % 2 + 1;
        }
    }
    return _uttt_check_win(game);
}
int simulate(board_t* game) {
    if (game->turns_left == 0) return game->winner;
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
            int amtP = winner == node->player ? 1 : -1;
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

    printf("    Searches run: %d\n", searches);
    printf("    Simulations run: %d\n", num_sims);
    printf("    Total States: %d\n", num_states);
    searches = 0;
    num_sims = 0;
}

// Wipe out really bad tree values to save memory
// for use elsewhere. Assumes that opponent won't
// waste time on a losing move so we won't either.
// If we evaluate enough, this will converge to
// a win-only tree.
int prune(tree_node_t* branch, board_t* game) {
    // Don't prune if there's a low chance the game is even over.
    if (AB_MIN_TURNS_LEFT < game->turns_left) return 0;
    int count = 0;
    int i;
    int player;
    int has_win = 0;
    // If branch is devoid of children, return the call
    if (branch->num_children <= 0)
        return count;
    // If there are children, ask them to do tidy themselves
    // first in case they end up empty
    i = 0;
    while (i < branch->num_children) {
        tree_node_t* child = branch->children[i++];
        if (child->is_win == 0) {
            // Test the child's move
            board_t test;
            uttt_clone(game, &test);
            player = test.player;
            uttt_move(&test, child->row, child->col);

            // Check if there's a winner available
            if (test.turns_left == 0) {
                // If we are the winner, delete the child
                if (test.winner == player) {
                    child->is_win = 1;
                    child->mean = 1.0;
                    has_win = 1;
                    continue;
                }
            }

            count += prune(child, &test);
        }

        if (child->is_win == 1) {
            has_win = 1;
            continue;
        }

        // If the child has pruned all of it's children,
        // then it's a winning move for us.
        if (child->num_children == 0) {
            child->is_win = 1;
            child->mean = 1.0;
            has_win = 1;
            continue;
        }
    }

    i = 0;
    while (i < branch->num_children) {
        tree_node_t* child = branch->children[i];
        // If the child already knows it's a winner,
        // give up on this branch, we can't win.
        // Leave it up to the parent to decide if we
        // cut this branch
        if (child->is_win == 1) {
            branch->is_win = -1;
            branch->mean = -1.0;
            i++;
            continue;
        } else if (has_win) {
            // If there was a winning move and this child
            // isn't one of them, remove the child
            count += shift_cut(branch, i);
            continue;
        }

        // If the child already knows it's a loser, cut him out.
        if (child->is_win == -1) {
            count += shift_cut(branch, i);
            continue;
        }

        // If we get to this situation, we don't have enough
        // information. try the next child.
        i++;
    }

    return count;
}

int remove_low_conf(tree_node_t* branch) {
    int count = 0;
    if (branch == NULL) return 0;
    if (branch->children == NULL) return 0;
    if (branch->visits < MIN_CUT_VISITS) return 0;

    tree_node_t* best = select_best(branch, PICK_CONST);
    if (best == NULL) return 0;
    if (best->visits < MIN_CUT_VISITS) return 0;

    float ucb;
    float best_lcb = uct(best->mean, best->visits, branch->visits, SIG_LCB_CONST);

    int i = 0;
    while (i < branch->num_children) {
        // Don't cut a branch that hasn't been visited much yet
        tree_node_t* child = branch->children[i];
        if (child->visits < MIN_CUT_VISITS) {
            i++;
            continue;
        }

        // If the best this child can promise is worse than
        // the worst of the best child, delete it.
        ucb = uct(child->mean, child->visits, branch->visits, SIG_UCB_CONST);
        if (ucb < best_lcb) {
            count += shift_cut(branch, i);
        } else {
            // This branch survived. Sacrifice its children.
            count += remove_low_conf(child);
            i++;
        }
    }
    return count;
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

// Adjusts the children to account for a removed child at pos.
// Use if you plan on preserving the rest of the tree
// instead of cut_branch
int shift_cut(tree_node_t* node, int pos) {
    int count = 0;
    tree_node_t* child = node->children[pos];

    int N = child->num_children;
    for (int i = 0; i < N; i++) {
        count += cut_branch(child->children[i]);
    }
    free(child->children);
    free(child);

    node->num_children--;
    for (int i = pos; i < node->num_children; i++) {
        node->children[i] = node->children[i+1];
    }
    count++;
    num_states--;

    return count;
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

/*
 * fast_mcts.h
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef FAST_MCTS_H
#define FAST_MCTS_H

#include "board.h"

#ifndef LOCAL_BUILD
#include "cpybot.h"
#endif

typedef struct tree_node_t {
    int player;
    int row;
    int col;
    int is_win;

    float mean;
    int visits;

    struct tree_node_t* parent;

    int num_children;
    struct tree_node_t** children;
} tree_node_t;

float uct(float mean, int visits, int total_visits, const float coeff);

void* worker(void* args);
void start_threads(void);
void stop_threads(void);

tree_node_t* find_node(tree_node_t* node, int row, int col);
tree_node_t* select_best(tree_node_t* node, float coeff);

void selection(board_t* board, tree_node_t* tree, tree_node_t** leaf);
int expand(board_t* board, tree_node_t* leaf, tree_node_t** node);
int rapid_simulate(board_t* board);
int simulate(board_t* board);
void backprop(int player, int winner, tree_node_t* leaf);
int prune(tree_node_t* branch, board_t* game);
int remove_low_conf(tree_node_t* branch);
int cut_branch(tree_node_t* branch);
int shift_cut(tree_node_t* node, int pos);

void show_choices(tree_node_t* branch);

#ifndef LOCAL_BUILD
// Parts needed to integrate with python host.
static PyMethodDef Cpybot_Methods[] = {
	{"setup", _pybot_setup, METH_VARARGS, "Setup the bot"},
	{"start", _pybot_start, METH_VARARGS, "Start the bot thread"},
	{"stop", _pybot_stop, METH_VARARGS, "Stop the bot thread"},
	{"update", _pybot_update, METH_VARARGS, "Send an update of the game state"},
	{"request", _pybot_request, METH_VARARGS, "Request a move from the bot"},
	{NULL, NULL, 0, NULL}	// Sentinel
};

static struct PyModuleDef Cpybot_Module =
{
    PyModuleDef_HEAD_INIT,
    "fast_mcts", /* name of module */
    "",          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    Cpybot_Methods
};

PyMODINIT_FUNC PyInit_fast_mcts(void)
{
    PyObject *m;
	m = PyModule_Create(&Cpybot_Module);
	return m;
}
#endif

#endif /* !FAST_MCTS_H */

/*
 * fast_mcts.h
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef FAST_MCTS_H
#define FAST_MCTS_H

#include "cpybot.h"

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


#endif /* !FAST_MCTS_H */

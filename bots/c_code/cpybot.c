/*
 * cpybot.c
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 */

#include "cpybot.h"
#include <python3.5/Python.h>

static PyObject* _pybot_setup(PyObject* self, PyObject* args) {
    // Source:
    // http://stackoverflow.com/questions/8001923/python-extension-module-with-variable-number-of-arguments

    Py_ssize_t TupleSize = PyTuple_Size(args);

    char* argv[TupleSize];

    for (Py_ssize_t i = 0; i < TupleSize; i++) {
        PyObject* item = PyTuple_GetItem(args, i);
        argv[i] = PyUnicode_AsUTF8(item);
        Py_DECREF(item);
    }

    int argc = TupleSize;

    setup(argc, argv);


    return Py_None;
}
static PyObject* _pybot_start(PyObject* self, PyObject* args) {
    start();
    return Py_None;
}
static PyObject* _pybot_stop(PyObject* self, PyObject* args){
    stop();
    return Py_None;
}
static PyObject* _pybot_update(PyObject* self, PyObject* args){
    int last_player;
    move_t last_move;

    int row, col;
    PyObject* py_last_player;
    PyObject* py_last_move;
    PyObject* py_row;
    PyObject* py_col;

    if (PyArg_ParseTuple(args, "OO", &py_last_player, &py_last_move)) {
		if (!PyLong_Check(py_last_player)) {
			PyErr_SetString(PyExc_TypeError, "parameter must be integer");
			return NULL;
		}
		last_player = PyLong_AsLong(py_last_player);

	} else {
		return NULL;
	}

	py_row = PyTuple_GetItem(py_last_move, 0);
	py_col = PyTuple_GetItem(py_last_move, 1);

	row = PyLong_AsLong(py_row);
	col = PyLong_AsLong(py_col);

    last_move.row = row;
    last_move.col = col;

    update(last_player, last_move);
    return Py_None;
}

static PyObject* _pybot_request(PyObject* self, PyObject* args){
    move_t move = request();

    return Py_BuildValue("(ii)", move.row, move.col);
}

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


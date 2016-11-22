/*
 * cpybot.h
 * Copyright (C) 2016 Tristan Meleshko <tmeleshk@ualberta.ca>
 *
 * Distributed under terms of the MIT license.
 *
 *
 * You should import cpybot.h and implement the missing methods
 * (marked below). These will be called automatically by the
 * python handler.
 *
 * Unlike the python handler, you are responsible for ALL internal
 * state of the bot. You cannot request the python code for a
 * new copy of the board if you lose track.
 */

#ifndef CPYBOT_H
#define CPYBOT_H

#include <python3.5/Python.h>

typedef struct {
    int row;
    int col;
} move_t;

// NOTE: Implement the following methods in YOUR code
void setup(int argc, char** argv);
void start(void);
void stop(void);
void update(int last_player, move_t last_move);
move_t request(void);

// The methods below adapt the python client to the C code.
static PyObject* _pybot_setup(PyObject* self, PyObject* args);
static PyObject* _pybot_start(PyObject* self, PyObject* args);
static PyObject* _pybot_stop(PyObject* self, PyObject* args);
static PyObject* _pybot_update(PyObject* self, PyObject* args);
static PyObject* _pybot_request(PyObject* self, PyObject* args);

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


#endif /* !CPYBOT_H */

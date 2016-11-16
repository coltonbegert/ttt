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

    return NULL;
}
static PyObject* _pybot_start(PyObject* self, PyObject* args) {
    start();
    return NULL;
}
static PyObject* _pybot_stop(PyObject* self, PyObject* args){
    stop();
    return NULL;
}
static PyObject* _pybot_update(PyObject* self, PyObject* args){
    int last_player;
    move_t last_move;

    int row, col;
    PyObject* py_last_move;

    PyArg_Parse(args, "iO", &last_player, py_last_move);
    PyArg_Parse(py_last_move, "ii", &row, &col);
    last_move.row = row;
    last_move.col = col;

    update(last_player, last_move);
    return NULL;
}

static PyObject* _pybot_request(PyObject* self, PyObject* args){
    move_t move = request();

    return Py_BuildValue("(ii)", move.row, move.col);
}


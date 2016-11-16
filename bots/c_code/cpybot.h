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
void start();
void stop();
void update(int last_player, move_t last_move);
move_t request();

// The methods below adapt the python client to the C code.
static PyObject* _pybot_setup(PyObject* self, PyObject* args);
static PyObject* _pybot_start(PyObject* self, PyObject* args);
static PyObject* _pybot_stop(PyObject* self, PyObject* args);
static PyObject* _pybot_update(PyObject* self, PyObject* args);
static PyObject* _pybot_request(PyObject* self, PyObject* args);

#endif /* !CPYBOT_H */

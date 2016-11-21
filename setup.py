#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

from setuptools import setup
from distutils.core import Extension

modules = []
modules.append(Extension('fast_mcts',
                    sources = [
                                'bots/c_code/fast_mcts.c',
				'bots/c_code/board.c'
                                ]))
modules.append(Extension('c_mcts',
                    sources = ['bots/mcts.c']))

setup (name = 'TTT',
       version = '0.0',
       description = 'Ultimate Tic-Tac-Toe player',
       ext_modules = modules)

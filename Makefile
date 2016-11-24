#
# Makefile
# tmeleshk, 2016-11-24 12:09
#

FLAGS=-O1 -std=c99
all:
	CFLAGS="$(FLAGS)" python3 setup.py build_ext --inplace

clean:
	rm -rf build/ *.so

# vim:ft=make
#

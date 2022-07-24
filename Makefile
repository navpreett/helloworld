#
# Makefile that builds btest and other helper programs for the CS:APP data lab
# 
VPATH = src

CC = gcc
CFLAGS = -O -Wall -g -std=c99

all: cacheSim

cacheSim: cacheSim.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -fv *.o cacheSim

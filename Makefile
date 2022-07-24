#
# Makefile that builds btest and other helper programs for the CS:APP data lab
# 
CC = gcc
CFLAGS = -O -Wall -m32 -g

all: cacheSim

cacheSim: cacheSim.c
	$(CC) $(CFLAGS) -o cacheSim cacheSim.c

clean:
	rm -f *.o cacheSim



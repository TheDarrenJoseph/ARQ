DIR=cd src;
CC=g++
CFLAGS=-Wall
LIBS=-lncurses
INCLUDES=-I../include -std=gnu++11
TARGET=arq
SOURCES=main.cpp characters.cpp grid.cpp ui.cpp items.cpp containers.cpp
OBJECTS=main.o characters.o grid.o ui.o items.o containers.o

all : arq clean

arq : objects
	cd src;$(CC) -o "arq" $(CFLAGS) $(INCLUDES) $(LIBS) $(OBJECTS)

objects : 
	cd src;$(CC) -c $(CFLAGS) $(INCLUDES) $(LIBS) $(SOURCES)

clean : 
	rm \$^*.o

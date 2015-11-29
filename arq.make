DIR=cd src;
CC=g++
CFLAGS=-Wall
LIBS=-lncurses 
INCLUDES=-I../include -std=gnu++11
TARGET=arq
SOURCES=main.cpp game.cpp characters.cpp map.cpp cursesUI.cpp playerUI.cpp items.cpp containers.cpp room.cpp
OBJECTS=main.o game.o characters.o map.o cursesUI.o playerUI.o items.o containers.o room.o
all : arq clean

arq : objects
	cd src;$(CC) -o "arq" $(CFLAGS) $(INCLUDES) $(LIBS) $(OBJECTS)

objects : 
	cd src;$(CC) -c $(CFLAGS) $(INCLUDES) $(LIBS) $(SOURCES)

clean : 
	cd src;rm *.o

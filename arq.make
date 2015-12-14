CFLAGS=-Wall -Wextra -Weffc++ -pedantic
LIBS=-l curses
INCLUDES=-I ../include -std=gnu++11
SOURCES=main.cpp game.cpp characters.cpp map.cpp cursesUI.cpp playerUI.cpp items.cpp containers.cpp room.cpp
OBJECTS=main.o game.o characters.o map.o cursesUI.o playerUI.o items.o containers.o room.o
all : arq clean

arq : objects
	cd src;g++ -o "arq" $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LIBS);

objects : 
	cd src;g++ -c $(CFLAGS) $(INCLUDES) $(SOURCES) $(LIBS);

clean : 
	cd src;rm *.o
CFLAGS=-Wall -Wextra -Weffc++ -pedantic
LIBS=-l curses
INCLUDES=-I ../include -std=gnu++11
SOURCES=main.cpp game.cpp characters.cpp map.cpp cursesUI.cpp playerUI.cpp items.cpp containers.cpp room.cpp stringUtils.cpp
OBJECTS=main.o game.o characters.o map.o cursesUI.o playerUI.o items.o containers.o room.o stringUtils.o

all : clean arq

clean : 
	rm -f *.out;cd src;rm -f *.o

objects : 
	cd src;g++ -g3 -c $(CFLAGS) $(INCLUDES) $(SOURCES);

arq : objects
	cd src;g++ -g3 -o "../arq.out" $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LIBS);


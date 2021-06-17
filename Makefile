CFLAGS=-Wall -Wextra -Weffc++ -pedantic
LIBS=-l curses
INCLUDES=-I ../include -std=gnu++11
SOURCES=main.cpp game.cpp characters.cpp map.cpp items.cpp containers.cpp doors.cpp room.cpp stringUtils.cpp pathfinding.cpp cursesUI.cpp inventoryUI.cpp containerSelection.cpp playerUI.cpp  
OBJECTS=main.o game.o characters.o map.o items.o containers.o doors.o room.o stringUtils.o pathfinding.o cursesUI.o inventoryUI.o containerSelection.o playerUI.o 

all : clean arq

clean : 
	rm -f *.out;cd src;rm -f *.o

objects : 
	cd src;g++ -g3 -c $(CFLAGS) $(INCLUDES) $(SOURCES);

arq : objects
	cd src;g++ -g3 -o "../arq.out" $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LIBS);


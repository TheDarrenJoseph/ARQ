#makefile for The ASCII Rougelike Quester 
arq : main.o characters.o grid.o ui.o items.o containers.o							
	cd src; g++ -g -o "arq" -Wall -I../include -lncurses -std=gnu++11 main.o characters.o ui.o grid.o items.o containers.o

#//Linux G++ -- "g++ -Wall -I../include -o "%e" "%f" -lncurses -std=gnu++11 characters.cpp ui.cpp grid.cpp items.cpp"
main.o characters.o grid.o ui.o items.o containers.o :
	cd src; g++ -c -Wall -I../include -lncurses -std=gnu++11 main.cpp characters.cpp grid.cpp ui.cpp items.cpp containers.cpp 

#include "room.h"

std::pair<int,int> Room :: GetStartPos() {return startPos;}
std::pair<int,int> Room :: GetEndPos() {return endPos;}

int Room :: GetSize() {return size;}

bool Room :: AddDoor(tile doorType, side s, int* x, int* y) {
    
    switch(s) {
    case (TOP) : 
        (*x) = startPos.first+size/2; 
        (*y) = startPos.second; 
        break;
        
    case (LEFT) :
        (*x) = startPos.first;
        (*y) = startPos.second+size/2;
        
        
        break;
        
    case (BOTTOM) :
        (*x) = startPos.first+size/2;     
        (*y) = endPos.second-1; //-1 to endPos to make up for 0 indexing      
        break;
        
    case (RIGHT) :
        (*x) = endPos.first-1;
        (*y) = startPos.second+size/2;
        break;
        
    }

    doors[doorNo++] = Door(cd1,(*x),(*y));

    
    return true;
}
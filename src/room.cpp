#include "room.h"
#include "position.h"

bool Room :: intersects(Room r) { 
    int startX = this->startPos.x;
    int startY = this->startPos.y;
    
    int endX = this->endPos.x;
    int endY = this->endPos.y;
    
    int newRoomStartX = r.GetStartPos().x;
    int newRoomStartY = r.GetStartPos().y;
    
    int newRoomEndX = r.GetEndPos().x;
    int newRoomEndY = r.GetEndPos().y;
  
    if( newRoomStartX > endX 
        || newRoomStartY > endY
        || (newRoomStartX < endX && (newRoomStartY > endY || newRoomEndY < startY)) 
        || (newRoomStartY < endY && (newRoomStartX > endX || newRoomEndX < startX)) ) {
	  return false;
    }
    
    // Must collide...
    return true;           
}

Position Room :: GetStartPos() {return startPos;}
Position Room :: GetEndPos() {return endPos;}

int Room :: GetSize() {return size;}

Door* Room :: getDoors(int* size) {
    (*size) = doorNo;
    return &doors[0];
}

bool Room :: AddDoor(tile doorType, side s, int* x, int* y) {
    
    switch(s) {
    case (TOP) : 
        (*x) = startPos.x+size/2; 
        (*y) = startPos.y; 
        break;
        
    case (LEFT) :
        (*x) = startPos.x;
        (*y) = startPos.y+size/2;
        break;
        
    case (BOTTOM) :
        (*x) = startPos.x+size/2;     
        (*y) = endPos.y-1; //-1 to endPos to make up for 0 indexing      
        break;
        
    case (RIGHT) :
        (*x) = endPos.x-1;
        (*y) = startPos.y+size/2;
        break;
        
    }

    doors[doorNo++] = Door(doorType,(*x),(*y));
    
    return true;
}

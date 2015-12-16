#include "room.h"

bool Room :: intersects(Room r) { 
    int startX = this->startPos.first;
    int startY = this->startPos.second;
    
    int endX = this->endPos.first;
    int endY = this->endPos.second;
    
    int newRoomStartX = r.GetStartPos().first;
    int newRoomStartY = r.GetStartPos().second;
    
    int newRoomEndX = r.GetEndPos().first;
    int newRoomEndY = r.GetEndPos().second;
    
    int newRoomTotalX = newRoomStartX+newRoomEndX;
    int newRoomTotalY = newRoomStartY+newRoomEndY;
    
    int roomTotalX = startX+endX;
    int roomTotalY = startY+endY;
    
    int totalX = newRoomTotalX-roomTotalX;
    int totalY = newRoomTotalY-roomTotalY;

    //Room starts after this one
    if(newRoomStartX>endX && newRoomStartY>endY && newRoomEndX>endX && newRoomEndY>endY) {  
        return false;
    }
    
    //Room comes before this one
    if (newRoomStartX<startX && newRoomStartY<startX && newRoomEndX<startX && newRoomEndY<startX) {
        return false;
    }
    
    //Same x
    if(newRoomStartX<endX && newRoomStartX>startY) {
        if (newRoomStartY>endY || newRoomEndY<startY) {
            return false;
        }
    }
    
    
    //Simmilar y
    if(newRoomStartY<endY && newRoomStartY>startY) {
        if (newRoomStartX>endX || newRoomEndX<startX) {
            return false;
        }
    }
    
    else {
        return true;
    }
               
}

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
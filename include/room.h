#ifndef ROOM_H
#define	ROOM_H

#include "tile.h"
#include "doors.h"
#include "containers.h"
#include "position.h"

enum side{TOP,BOTTOM,LEFT,RIGHT};

class Room {
    
    private:
        //Top-left corner/starting co-ordinates for a room
        Position startPos = Position(0,0);
        
        //Size of each wall/overall square size
        int size=3;
        
        //Co-ordinates for the bottom-right corner of a room
        Position endPos = Position(0,0);
        
        Door doors[8];
        int doorNo = 0;
        
public:
    
    bool intersects(Room r);
    Position GetStartPos();
    Position GetEndPos();
    int GetSize();
    
    Door AddDoor(Door door, side s);
    Door* getDoors(int* doorCount);
    
    Room(int x, int y, int size) {
        doorNo = 0;
        startPos = Position(x,y);
        endPos   = Position(x+size,y+size);
        this->size = size;
    }
    
    Room() : Room(0,0,0) {
    }
    
    ~Room() {
        
    }

};

#endif	/* ROOM_H */


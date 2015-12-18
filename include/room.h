#ifndef ROOM_H
#define	ROOM_H

#include "tile.h"
#include "position.h"

enum side{TOP,BOTTOM,LEFT,RIGHT};

/** A class type to store indicator information for rooms in use in the game,
 *  isn't tied to the creation of rooms on the map (e.g the walls that form it), 
 * this allows "zones" or special room implementations, while still sectioning off parts of the map as rooms.
 */
 struct Door {
            tile doorType=od0; 
            int posX=0;
            int posY=0;
            
            Door() : Door(cd1,0,0) {
            }
            
            Door(tile t, int x, int y) {
                doorType = t;
                posX = x;
                posY = y;
            }
        };  
   

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
    
    bool AddDoor(tile doorType, side s, int* x, int* y);
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


#ifndef ROOM_H
#define	ROOM_H

#include "tile.h"

enum side{TOP,BOTTOM,LEFT,RIGHT};

/** A class type to store indicator information for rooms in use in the game,
 *  isn't tied to the creation of rooms on the map (e.g the walls that form it), 
 * this allows "zones" or special room implementations, while still sectioning off parts of the map as rooms.
 */
class Room {
    
    private:
        
        struct Door {
            tile doorType;
            int posX;
            int posY;
            
            Door() : Door(cd1,0,0) {
            }
            
            Door(tile t, int x, int y) {
                doorType = t;
                posX = x;
                posY = y;
            }
        };   
        
        //Top-left corner/starting co-ordinates for a room
        std::pair<int,int> startPos;
        
        //Size of each wall/overall square size
        int size;
        
        //Co-ordinates for the bottom-right corner of a room
        std::pair<int,int> endPos;
        
        Door doors[8];
        int doorNo = 0;
        
public:
    
    std::pair<int,int> GetStartPos();
    std::pair<int,int> GetEndPos();
    int GetSize();
    
    bool AddDoor(tile doorType, side s, int* x, int* y);
    
    
    Room(int x, int y, int size) {
        doorNo = 0;
        startPos = std::make_pair(x,y);
        endPos   = std::make_pair(x+size,y+size);
        this->size = size;
    }
    
    Room() : Room(0,0,0) {
        
    }
    
    ~Room() {
        
    }

};

#endif	/* ROOM_H */


#ifndef ROOM_H
#define	ROOM_H

#include "tile.h"

using namespace std;

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
        pair<int,int> startPos;
        
        //Size of each wall/overall square size
        int size;
        
        //Co-ordinates for the bottom-right corner of a room
        pair<int,int> endPos;
        
        Door doors[8];
        int doorNo = 0;
        
public:
    
    pair<int,int> GetStartPos();
    pair<int,int> GetEndPos();
    int GetSize();
    
    bool AddDoor(tile doorType, side s, int* x, int* y);
    
    
    Room(int x, int y, int s) {
        doorNo = 0;
        startPos = make_pair(x,y);
        endPos   = make_pair(x+s,y+s);
        size = s;
    }
    
    Room() : Room(0,0,0) {
        
    }
    
    ~Room() {
        
    }

};

#endif	/* ROOM_H */


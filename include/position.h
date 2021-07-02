#ifndef POSITION_H
#define	POSITION_H

#include <list>

struct Position {
    
private:
public:
     int x=0;
     int y=0;
    
    Position() {
        x=0;
        y=0;
    }
    
    Position(int x,  int y) {
        this->x = x;
        this->y = y;
    }

    //Overriding equality operator
    bool operator==(const Position& p) {  
        if (this->x == p.x && this->y == p.y) {
            return true;
        }
        else {
            return false;
        }
    }
    
     bool operator<(const Position& rval) const {  
         return this->x < rval.x || (!(rval.x < this->x) && this->y < rval.y);
    }
     
     bool operator!=(const Position& p) {  
        if (this->x == p.x && this->y == p.y) {
            return false;
        }
        else {
            return true;
        }
    }
    
};

typedef std::list<Position> Path;

#endif


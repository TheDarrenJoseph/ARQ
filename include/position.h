#ifndef POSITION_H
#define	POSITION_H

#include <list>

struct Position {
    
private:
public:
    unsigned int x=0;
    unsigned int y=0;
    
    Position() {
        x=0;
        y=0;
    }
    
    Position(int x,int y) {
         if (x>0 && y>0) {  
             this->x = x;
             this->y = y;
         } else {
             this->x =0;
             this->y =0;
         }
    }
    
    Position(unsigned int x, unsigned int y) {
        this->x = x;
        this->y = y;
        
    }
    
    //Overriding assignment operator
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


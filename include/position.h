#ifndef POSITION_H
#define	POSITION_H
#include <list>

struct Position {
    
private:
public:
    unsigned short int x=0;
    unsigned short int y=0;
    
    Position(unsigned short int x, unsigned short int y) {
        this->x = x;
        this->y = y;
    }
};

typedef std::list<Position> Path;

#endif	/* POSITION_H */


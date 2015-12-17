#ifndef POSITION_H
#define	POSITION_H
typedef std::list<Position> Path;

struct Position {
    unsigned short int posX;
    unsigned short int posY;
private:
public:
    
    Position(unsigned short int x, unsigned short int y) {
        posX = x;
        posY = y;
    }
};

#endif	/* POSITION_H */


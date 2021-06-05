#ifndef DOORS_H
#define DOORS_H
#include "tile.h"

class Door {
  public:
    tile doorTile = dor;
    tile_details tileDetails = tile_library[dor];
    // TODO remove these co-ords
    int posX=-1;
    int posY=-1;
    std::string name = "";
    int lockCount = 0;
    bool isLocked = false;
    bool isOpen = false;
    int hitPoints = 0;

    void Open();
    void Close();
    void Lock();
    void Unlock();

  Door(std::string name, int lockCount, int hitPoints) {
    this -> name = name;
    this -> lockCount = lockCount;
    this -> hitPoints = hitPoints;
  }
  
  Door(const Door* toCopy) {  
    this -> posX = toCopy -> posX;
    this -> posY = toCopy -> posY;
    this -> name = toCopy -> name;
    this -> lockCount = toCopy -> lockCount;
    this -> isLocked = toCopy -> isLocked;
    this -> isOpen = toCopy -> isOpen;
    this -> hitPoints = toCopy -> hitPoints;
  }

  Door() : name("Wooden Door"), hitPoints(100) {};

};

extern Door WOODEN_DOOR;
extern Door IRON_DOOR;
extern Door STEEL_DOOR;

#endif

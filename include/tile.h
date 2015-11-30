#ifndef TILE_H
#define TILE_H
#include <string>

//enum for indexing fixed instances of tile_details
enum tile {ntl,cor,rom,wa1,wa2,wa3,win,od0,cd0,od1,cd1,ld1,od2,cd2,ld2,ent,ext,ded};

struct tile_details 
{
  int id;
  bool traversible;
  bool destroyable;
  const char* symbol;
  int color;
  std::string name;
  int locks;
  int health;
};

//creates detail for each tile, must be the same size as the enum
//these only ever have one instance, which are used as reference.
const tile_details tile_library[] =
{
 /*Index*/ /*traversible*/ /*Destroyable*/ /*Symbol*/ /*Colour*/ /*Name*/             /*Locks*/
 {0,       0,              0,              " ",       0,         "Empty     ",        0,            /*Hitpoints*/ 0}, //non
 {1,       1,              0,              "-",       4,         "Corridor",          0,            /*Hitpoints*/ 0 }, //cor
 {2,       1,              0,              "-",       4,         "Room",              0,            /*Hitpoints*/ 0 }, //rom
 {3,       0,              0,              "#",       3,         "Concrete Wall",     0,            /*Hitpoints*/ 0 }, //wa1
 {4,       0,              0,              "#",       3,         "Stone Wall",        0,            /*Hitpoints*/ 0 }, //wa2
 {5,       0,              0,              "#",       3,         "Brick Wall",        0,            /*Hitpoints*/ 0 }, //wa3
 {6,       0,              0,              "%",       6,         "Window",            0,            /*Hitpoints*/ 0},
 {7,       0,              0,              "-",       0,         "Open Office Door",  0,            /*Hitpoints*/ 100},
 {8,       0,              1,              "=",       2,         "Closed Office Door",0,            /*Hitpoints*/ 100},
 {9,       1,              0,              "-",       0,         "Open Metal Door",   1,            /*Hitpoints*/ 150},
 {10,      0,              1,              "=",       2,         "Closed Iron Door",  1,            /*Hitpoints*/ 150 },
 {11,      0,              1,              "=",       1,         "Locked Iron Door",  1,            /*Hitpoints*/ 150 },
 {12,      1,              0,              "-",       0,         "Open Steel Door",   2,            /*Hitpoints*/ 200 },
 {13,      0,              1,              "=",       2,         "Closed Steel Door", 2,            /*Hitpoints*/ 200 },
 {14,      0,              1,              "=",       1,         "Locked Steel Door", 2,            /*Hitpoints*/ 200},
 {15,      0,              0,              "@",       1,         "Entrance",          0,            /*Hitpoints*/ 0},
 {16,      0,              0,  	          "@",       2,         "Exit",              0,            /*Hitpoints*/ 0 },
 {17,      0,              0,	           "!",        7,         "Deadly Floor",      0,            /*Hitpoints*/ 0 },
};

#endif

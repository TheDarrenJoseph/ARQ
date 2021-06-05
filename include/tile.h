#ifndef TILE_H
#define TILE_H
#include <string>

//enum for indexing fixed instances of tile_details
enum tile {ntl,cor,rom,wa1,wa2,wa3,win,dor,ent,ext,ded};

struct tile_details 
{
  int id;
  tile tile_type;
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
 /*Index*/ /*tile_type*/ /*traversible*/ /*Destroyable*/ /*Symbol*/ /*Colour*/ /*Name*/             /*Locks*/
 {0,       ntl,            0,              0,              " ",       0,         "Empty     ",        0,            /*Hitpoints*/ 0},
 {1,       cor,            1,              0,              "-",       4,         "Corridor",          0,            /*Hitpoints*/ 0 },
 {2,       rom,            1,              0,              "-",       4,         "Room",              0,            /*Hitpoints*/ 0 },
 {3,       wa1,            0,              0,              "#",       3,         "Concrete Wall",     0,            /*Hitpoints*/ 0 },
 {4,       wa2,            0,              0,              "#",       3,         "Stone Wall",        0,            /*Hitpoints*/ 0 },
 {5,       wa3,            0,              0,              "#",       3,         "Brick Wall",        0,            /*Hitpoints*/ 0 },
 {6,       win,            0,              0,              "%",       6,         "Window",            0,            /*Hitpoints*/ 0},
 {7,       dor,            0,              0,              "=",       0,         "Door",              0,            /*Hitpoints*/ 100}, 
 {8,       ent,            1,              0,              "^",       1,         "Entrance",          0,            /*Hitpoints*/ 0},
 {9,       ext,            1,              0,  	           "^",       2,         "Exit",              0,            /*Hitpoints*/ 0 },
 {10,      ded,            0,              0,	             "!",       7,         "Deadly Floor",      0,            /*Hitpoints*/ 0 },
};

#endif

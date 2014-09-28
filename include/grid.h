#ifndef GRID_H
#define GRID_H

#include "tile.h"
#include "containers.h"
#include "items.h"
#include "ui.h"

const int grid_x = 30;
const int grid_y = 15;

tile GetTile(int x, int y);
void SetTile(int x, int y,tile t);

item* GetItem(int x, int y);
void SetItem(int x, int y,item* i);

area* GetArea(int x, int y);
void SetArea(int x, int y, area* a);

#endif

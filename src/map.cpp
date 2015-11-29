#include "map.h"
#include "room.h"

bool Map::CanPlaceRoom(int newRoomStartX, int newRoomStartY, int newRoomSize)
{
    //return true; //DEBUGGING!

    if (roomCount == 0) return true;

    else if (roomCount < MAX_ROOMS) {

        int newRoomEndX = newRoomStartX + newRoomSize;
        int newRoomEndY = newRoomStartY + newRoomSize;


        int startX;
        int startY;

        int endX;
        int endY;

        //Order/Sort a room list this to compare rooms in the area?? (EFFICIENCY??)
        for (int i = 0; i < roomCount; i++) {

            startX = rooms[i].GetStartPos().first;
            startY = rooms[i].GetStartPos().second;

            endX = rooms[i].GetEndPos().first;
            endY = rooms[i].GetEndPos().second;


            if ((newRoomStartX > endX || newRoomStartY > endY) && (newRoomEndX > endX || newRoomEndY > endY)) return true;
            else if ((newRoomStartX < startX || newRoomStartY < startY) && (newRoomEndX < startX || newRoomEndY < startY)) return true;
        }

    }

    return false;
}

void Map::CreateWall(int x, int y, int size, bool horizontal)
{

    //offset position and size to give proper room coordinates
    int xSize = x + size;
    int ySize = y + size;

    if (horizontal) {
        for (; x < xSize; x++) {
            game_grid[y][x] = wa1;
        }
    }
    else {
        for (; y < ySize; y++) {
            game_grid[y][x] = wa1;
        }
    }
}

/**
 * 
 * @param x
 * @param y
 */
bool Map::CreateRoom(int x, int y, int size, Room* room)
{

    //min size 3x3 acceptable
    //Min area for a room = 9

    //Sanity check
    if (x > 0 && x < GRID_X - size && y > 0 && y < GRID_Y - size && size >= 3) {

        if (CanPlaceRoom(x, y, size)) {
            //Create first half/corner
            CreateWall(x, y, size, true);
            CreateWall(x, y, size, false);

            //Create the opposite half/opposing corner of the room
            CreateWall(x + 1, y + size - 1, size - 1, true);
            CreateWall(x + size - 1, y + 1, size - 2, false);

            rooms[roomCount] = Room(x, y, size); //add our new room and increase the count
            room = &rooms[roomCount++];

            //Door addition algorithm


            int doorNo = rand() % 4 + 1; //rand number from 1 to 8


            tile doorTile = cd1; //default door tile

            int doorX, doorY;

            //rooms[i].GetStartPos();

            for (int i = 0; i < doorNo; i++) {
                side roomSide;

                switch (rand() % 4 + 1) {
                case 1: roomSide = LEFT;
                    break;
                case 2: roomSide = RIGHT;
                    break;
                case 3: roomSide = TOP;
                    break;
                case 4: roomSide = BOTTOM;
                    break;
                }

                if (room->AddDoor(doorTile, roomSide, &doorX, &doorY)) {
                    game_grid[doorY][doorX] = doorTile;
                }

            }


            return true;
        }
    }

    return false;
}

/** Generates a layer of the dungeon, creating rooms and corridors procedurally.*/
void Map::CreateMap(int seed)
{
    //Inefficient, try to set all tiles by default
    for (int x = 0; x < GRID_X; x++) {
        for (int y = 0; y < GRID_Y; y++) {
            game_grid[y][x] = cor; //make every tile a corridor to begin with

            //int roomChance = rand();
            //int size = rand(); //size>=4

            //Run over the map
            //Start at top-left corner of the map
            //if (roomChance>50) {
            //1. generate a room
            //2. pick random side of the room (non-diagonal, south, or east) 
            //3. create a door at that location
            //4. create a path and repeat 

            //random No from 4 up
            //random wall type
            //set all 4 sides to a wall type

            //Finding a door location
            //pick a corner
            //size of a wall-2
            //random number in this range
            //corner x,y + this number
            //}

        }

    }


    //testing room creation
    CreateRoom(1, 2, 5, NULL);
    CreateRoom(7, 7, 3, NULL);
    CreateRoom(1, 3, 3, NULL);


}

bool Map::IsTraversable(int x, int y)
{

    if ((x < 0) || (x > gridX) || (y < 0) || (y > gridY)) {
        return false;
    }

    tile t = game_grid[y][x];

    return tile_library[t].traversible;
}

int Map::EnvironmentCheck(int x, int y)
{
    tile t = GetTile(x, y);

    if (t == cd0 || (t == cd1) || (t == cd2) || (t == ld1) || (t == ld2)) {
        return 1; //door processing
    }
    else if (t == ded) {
        return 2; //danger sign
    }
    else return 0;

}

bool Map::CanPlaceItems(int x, int y)
{
    tile playerTile = GetTile(x, y);
    if ((playerTile == ntl) || (playerTile == rom) || (playerTile == cor)) return true;
    else return false;
}

int Map::EncounterCheck(int x, int y, int* npcID)
{

    for (int e_id = 0; e_id < MAX_NPCS; e_id++) {
        if (npcs[e_id].IsAlive()) {
            int enemy_x;
            int enemy_y;
            npcs[e_id].GetPos(&enemy_x, &enemy_y); //Sets enemy x and y to those of npc[No] through pointers

            *npcID = e_id;


            //encounter 
            if (((x == enemy_x and y == enemy_y) || (x == enemy_x and y == enemy_y))) {
                if (npcs[e_id].IsAlive()) {
                    return 1;
                }
                else return 2; //dead body 

            }

        }

    }

    return 0;
}

int Map::MoveCharacter(Character* c, int x, int y)
{

    int pos_x;
    int pos_y;

    c->GetPos(&pos_x, &pos_y);

    //First checks for no movement, then for a diagonal (x and y changed))
    if (((x == pos_x) && (y == pos_y)) | (x != pos_x) && (y != pos_y)) {
        return 99;
    }

    if (IsTraversable(x, y)) {
        c->SetPos(x, y);
        return 0;
    }

    switch (EnvironmentCheck(x, y)) {
    case 1:
        //door!
        return 1;


    case 2:
        //trap!!
        return 2;

    }

    return 99;
}

/**
 * @param npcID - an ID pointer for incase an NPC is encountered
 * @return code guide --
 * 0 - moved successfully
 * 1 - door found
 * 2 - trap found
 * 
 * 3 - Enemy found
 * 4 - Dead body found
 * 
 * 99 - Cannot move
 * Other - determined by EncounterCheck()'s return value
 */
int Map::MovePlayer(int x, int y, int* npcID)
{
    if ((x < 0) || (y < 0) || (x > GRID_X - 1) || (y > GRID_Y - 1)) {
        switch (EncounterCheck(x, y, npcID)) {
        case 1:
            return 3;
            break;

        case 2:
            return 4;
            break;
        }
    }

    return MoveCharacter(player, x, y);
}

/** Returns a movement code for each npc
 * 
 * @return 
 */
int Map::MoveNPCS()
{
    for (int i = 0; i < MAX_NPCS; i++) {

        int move_x;
        int move_y;

        npcs[i].Move(&move_x, &move_y); //sets move_x and y to AI chosen values

        Character* npc = &npcs[i];

        MoveCharacter(npc, move_x, move_y);

    }

    return 1;

}

/* Makes an area/dead body that contains the characters possesions*/
void Map::DropCharacterItems(Character* c)
{

    Area* body = c->DropItems();

    int x, y;
    c->GetPos(&x, &y);

    SetArea(x, y, body);
}

int Map::DropPlayerItem(Player* p, Item* thisItem, int invX, int invY)
{
    int x, y;
    p->GetPos(&x, &y);

    //if the player is at an area where items can be placed, add the item
    if (CanPlaceItems(x, y)) {
        AddToArea(x, y, thisItem); //replace the map tile with the item

        p->SetInventoryTile(invX, invY, new Item(item_library[no_item])); //clear the inventory tile

        return 0;
    }

    else {
        return 1;
    }

}

tile Map::GetTile(int x, int y)
{
    return game_grid[y][x];
}

void Map::SetTile(int x, int y, tile t)
{
    game_grid[y][x] = t;
};

Item* Map::GetItem(int x, int y)
{
    return container_grid[y][x]->GetItem(0, 0);
}

Item* Map::GetContainerItem(int containerX, int containerY, int x, int y)
{
    return container_grid[containerY][containerX]->GetItem(x, y);
}

//Adds an item to the area

void Map::AddToArea(int x, int y, Item* i)
{
    container_grid[y][x]->AddItem(i);
}

Area* Map::GetArea(int x, int y)
{
    return container_grid[y][x];
}

void Map::SetArea(int x, int y, Area* a)
{
    container_grid[y][x] = a;
}

int Map::GetGridX()
{
    return gridX;
}

int Map::GetGridY()
{
    return gridY;
}

void Map::InitAreas()
{
    for (int x = 0; x < GRID_X; x++) {
        for (int y = 0; y < GRID_Y; y++) {
            container_grid[y][x] = new Area(99, "Empty", " ", 0, 0, false);
        }
    }
}

/*
void DrawInv(area* a)
{
  for (int y = 0; y<3; y++)
    {
      //draw each item on this row to the screen
      for(int x = 0; x<3; x++)
        {	 
          WINDOW* thisWindow = invwins_front[y][x];
         
          werase(thisWindow);
          wmove(thisWindow,0,0); 
      	 
          item* itm = a->GetItem(x,y);      
       
          //int colour = itm->colour;
          std::string str = itm->name;
         
          const char* symbol = str.c_str();

          wprintw_col (thisWindow, symbol, 0);
          wrefresh(thisWindow);
        }; 
    };  
  return;
}
 */


int Map::AreaProc(int x, int y)
{
    return GetArea(x, y)->HasItems();
}

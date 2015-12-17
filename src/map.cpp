#include "map.h"
#include "room.h"

bool Map::CanPlaceRoom(int newRoomStartX, int newRoomStartY, int newRoomSize)
{ 
    if (roomCount == 0) {
        return true;
    }
    
    int newRoomEndX = newRoomStartX + newRoomSize;
    int newRoomEndY = newRoomStartY + newRoomSize;
    
  
    if (newRoomStartX<0 || newRoomEndX>GRID_X || newRoomStartY<0 || newRoomEndY>GRID_Y) {
        return false;
    } else if (roomCount < MAX_ROOMS) {
        Room newRoom = Room(newRoomStartX, newRoomStartY, newRoomSize);
        
        for (int i=0; i < roomCount; i++) {
            if (rooms[i].intersects(newRoom)) return false;
        }

        return true;
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
    if (x >= 0 && x < GRID_X - size && y >= 0 && y < GRID_Y-size && size >= 3) {

        if (CanPlaceRoom(x, y, size)) {
            //Create first half/corner
            CreateWall(x, y, size, true);
            CreateWall(x, y, size, false);

            //Create the opposite half/opposing corner of the room
            CreateWall(x + 1, y + size - 1, size - 1, true);
            CreateWall(x + size - 1, y + 1, size - 2, false);

            rooms[roomCount] = Room(x, y, size); //add our new room and increase the count
            room = &rooms[roomCount];
            roomCount++;
            
            //Door addition algorithm
            int doorNo = rand() % 4 + 1; //rand number from 1 to 3
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
void Map::CreateMap(int roomChance)
{
    int randChance = 0;
    int roomArea =0;
    int tryCount=0;
    
    //Inefficient, try to set all tiles by default
    while(roomCount<7) {
       // srand(time(NULL));
        
   // for (int x = 0; x < GRID_X; x++) {
     //   for (int y = 0; y < GRID_Y; y++) {
            //game_grid[y][x] = cor; //make every tile a corridor to begin with
            
        int x = rand() % GRID_X;
        int y = rand() % GRID_Y;
            
            randChance = rand() % 100+1; //rand 1-100
            int size = rand() % 10+3;
            
            if (roomCount<MAX_ROOMS && randChance > 40 ) {
                if (CreateRoom(x, y, size, NULL)) {
                    roomArea += size;
                }
                
            }
              
        if (tryCount>50) {
            for (int x=0; x<GRID_X; x++) {
                for (int y=0; y<GRID_Y; y++) {
                    game_grid[y][x] = ntl;
                }
            }
            roomCount=0;
            roomArea=0;
            tryCount = 0;
           
        } else {
            tryCount++;
        }

    }

    PathRooms();
    //testing room creation

}

/** Subfunction for PathRooms that checks that a suitable path exists for the player to traverse the level
 * 
 */
void Map :: LevelPathValid() {
    //Plot a level path? (from the first room to the last, and include an exit?)
    
}

void Map :: PaveRoom(Room r) {
    std::pair<int,int> startPos = r.GetStartPos();
    std::pair<int,int> endPos = r.GetEndPos();
    
    for (int y=startPos.second+1; y<endPos.second-1; y++) {
        for (int x=startPos.first+1; x<endPos.first-1; x++) {
            
            if (game_grid[y][x] == ntl) {
                game_grid[y][x] = rom;
            }
        }
    }
}

void Map :: PathRooms(){
    for (int i=0; i<roomCount; i++) {
        PaveRoom(rooms[i]);
        
    }
    //start at rooms[0]
    
    //Link to every room if possible
    //Pathfind to one of the room's doors
    
}

bool Map::IsTraversable(int x, int y)
{
    if ((x < 0) || (x > gridX) || (y < 0) || (y > gridY)) {
        return false;
    } else {
        tile t = game_grid[y][x];
        return tile_library[t].traversible;
    }
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

/** Checks every NPC's pos to see if you've walked into them, also checks if they're alive
 * 
 * @param x
 * @param y
 * @param npcID
 * @return 
 */
int Map::EncounterCheck(int x, int y, int* npcID) {

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
   
    if ((x > 0) || (y > 0) || (x < GRID_X) || (y < GRID_Y)) {
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

    Container body = c->DropItems();

    int x, y;
    c->GetPos(&x, &y);

    SetContainer(x, y, body);
}

int Map::DropPlayerItem(Player* p, Item* thisItem, int index)
{
    int x, y;
    p->GetPos(&x, &y);

    //if the player is at an area where items can be placed, add the item
    if (CanPlaceItems(x, y)) {
        AddToContainer(x, y, thisItem); //replace the map tile with the item

        p->GetInventory()->RemoveItem(index); //clear the inventory tile

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
}

const Item* Map::GetItem(int x, int y)
{
    int size = container_grid[y][x].GetSize();
    if(size>0) {
          return &container_grid[y][x];
    } else {
        return &item_library[0];
    }

}    

const Item* Map::GetContainerItem(int containerX, int containerY, unsigned long int index)
{
    return container_grid[containerY][containerX].GetItem(index);
}

//Adds an item to the area

void Map::AddToContainer(int x, int y, const Item* i)
{
    container_grid[y][x].AddItem(i);
}

Container Map::GetContainer(int x, int y)
{
    return container_grid[y][x];
}

void Map::SetContainer(int x, int y, Container c)
{
    container_grid[y][x] = c;
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


int Map::ContainerProc(int x, int y)
{
    return GetContainer(x, y).HasItems();
}

#include <vector>
#include <algorithm>
#include <limits>

#include "map.h"
#include "room.h"
#include "doors.h"

void Map :: SetEntryPositions(Position entry, Position exit) {
    entryPosition = entry;
    exitPosition = exit;
}

Position Map :: GetEntryPosition() {
    return entryPosition;
}

Position Map :: GetExitPosition() {
    return exitPosition;
}

std::list<Position> Map :: GetNeighbors(int x, int y) {
    std::list<Position> possibleNeighbors;
    std::list<Position> neighbors = *new std::list<Position>();    

    //Adding the main 4 cardinal directions as neighbors
    possibleNeighbors.push_back(Position(x+1, y));
    possibleNeighbors.push_back(Position(x-1, y));
    possibleNeighbors.push_back(Position(x, y+1));
    possibleNeighbors.push_back(Position(x, y-1));
		
    //Only add those neighbors within the map bounds to the neighbors list
    for (Position p : possibleNeighbors) {
        if (IsInBoundaries(p.x,p.y)) neighbors.push_back(p);
    }
    
    return neighbors;
}

bool Map::CanPlaceRoom(Room* room)
{
  if ( IsInBoundaries(room -> GetStartPos()) && IsInBoundaries(room -> GetEndPos()) ) {
    if (roomCount < MAX_ROOMS) {
      for (int i=0; i < roomCount; i++) {
        if (rooms[i].intersects(*room)) return false;
      }
      return true;
    }
  }
  return false;
}

void Map::CreateWalls(Room* room)
{
  for(unsigned int x = room->GetStartPos().x; x < room->GetEndPos().x; x++) {
    game_grid[room->GetStartPos().y][x] = wa1;
    game_grid[room->GetEndPos().y-1][x] = wa1;
  }
  
  for(unsigned int y = room->GetStartPos().y + 1; y < room->GetEndPos().y - 1; y++) {
    game_grid[y][room->GetStartPos().x] = wa1;
    game_grid[y][room->GetEndPos().x-1] = wa1;
  }
}

side OppositeSide(side s) {
    switch(s) {
        case TOP : return BOTTOM;
        case BOTTOM : return TOP;
        case LEFT :  return RIGHT;
        case RIGHT : return LEFT;
    }
    
    return TOP;
}

void Map :: AddOppositeDoors(Room* room, Door door, side one, side two) {
    Door sideOneDoor = room->AddDoor(door,OppositeSide(one));
    game_grid[sideOneDoor.posY][sideOneDoor.posX] = sideOneDoor.doorTile;

    Door sideTwoDoor = room->AddDoor(door,OppositeSide(two));
    game_grid[sideTwoDoor.posY][sideTwoDoor.posX] = sideTwoDoor.doorTile;
}

void Map :: AddDoor(Room* room, Door door, side doorSide) {
    Door newDoor = room->AddDoor(door, doorSide);
    game_grid[newDoor.posY][newDoor.posX] = newDoor.doorTile;
}


/**
 * 
 * @param x
 * @param y
 */
bool Map::CreateRoom(int x, int y, int size, Room* room)
{
  Room newRoom = Room(x, y, size);
  
  //min size 3x3 acceptable
  //Min area for a room = 9
  if (size < 3 || !CanPlaceRoom(&newRoom)) return false;
  
  CreateWalls(&newRoom);
  
  rooms[roomCount] = newRoom; //add our new room and increase the count
  room = &rooms[roomCount++];
  
  //Door addition algorithm
  Door door = new Door(WOODEN_DOOR);
  
  //enum side={TOP,BOTTOM,LEFT,RIGHT}
  //Don't add doors to any side touching the edge of the map!!
  //Check the 4 corners
  
  //TOP RIGHT
  if (x == GetGridX()-size && y == 0) {
      AddOppositeDoors(room, door, TOP, RIGHT);
      return true;
  }
  //BOTTOM LEFT
  if (x == 0 && y == GetGridY()-size) {
      AddOppositeDoors(room, door, BOTTOM, LEFT);
      return true;
  }
  //TOP LEFT
  if (x == 0 && y == 0) {
      AddOppositeDoors(room, door, TOP, LEFT);
      return true;
  }
  //BOTTOM RIGHT
  if (x == GetGridX()-size && y == GetGridY()-size) {
      AddOppositeDoors(room, door, BOTTOM, RIGHT);
      return true;
  }
      
  //TOP
  if (y == 0) {
      AddDoor(room, door,BOTTOM);
      return true;
  }
  //LEFT
  if (x == 0) {
      AddDoor(room, door,RIGHT);
      return true;
  }
  
  //Right
  if (x == GetGridX()-size) {
      AddDoor(room, door,LEFT);
      return true;
  }
  
  //Bottom
  if (y == GetGridY()-size) {
      AddDoor(room, door,TOP);
      return true;
  }
  
  // Otherwise pick a random door count of max sides
  int doorCount = (rand() % 4 + 1);
  for (int doorNo = doorCount; doorNo > 0; doorNo--) {
    side randomDoorSide = static_cast<side>(rand()%4);
    AddDoor(room, door, randomDoorSide);
  }
  return true;
}


/** Generates a layer of the dungeon, creating rooms and corridors procedurally.*/
void Map::CreateMap(int roomChance) {
    const int MAX_ROOM_SIZE = 10;
    const int MIN_ROOM_SIZE = 3;

    int roomArea = 0;
    int possibleRoomArea = (gridX-2)*(gridY-2); //-2 for external walls
    int roomQuota = possibleRoomArea/2;

    std::set<Position> possibleRoomPositions;
	
    //Adding every position on the map to our set of possible positions
    //Leaving a 1 tile gap out of our room positions to allow for corridors
    //Min room size allows for smaller rooms to generate near map edges
    //Greatly increases map space utilisation
    for (int x=1; x < gridX-MIN_ROOM_SIZE-1; x++) {
        for (int y=1; y < gridY-MIN_ROOM_SIZE-1; y++) {
            possibleRoomPositions.insert(Position(x,y));
        }
    }

    while(roomArea < (roomQuota - MIN_ROOM_SIZE) 
	    && !(possibleRoomPositions.empty())) {
        // std::cout << "Filling map.. "<< roomArea << " of " << roomQuota << "\n";
        
        //Pick a random position out of what's available
        int randPosNo = rand() % possibleRoomPositions.size();

        Position roomPos = *std::next(possibleRoomPositions.begin(), randPosNo);

        int x = roomPos.x;
        int y = roomPos.y;
        int attempts = 1;
        
	      int targetMaxSize = std::min(MAX_ROOM_SIZE, roomQuota - roomArea); // Prevent exceeding quota
        int size = (rand() % (targetMaxSize - MIN_ROOM_SIZE + 1)) + MIN_ROOM_SIZE; // Random size in range [MIN_ROOM_SIZE, targetMaxSize] (Inclusive)
        bool roomPlaced = false;
        
        roomPlaced = CreateRoom(x, y, size, NULL);

        while(!roomPlaced && attempts < 3 && size > MIN_ROOM_SIZE) {
            size--; //Try a smaller room
            attempts++;
            roomPlaced = CreateRoom(x,y,size,NULL);
        }
            
        if (roomPlaced) {
            int xEnd = x+size+1; //+1 to leave a gap
            int yEnd = y+size+1;

            for (; x < xEnd; x++) {
                for (; y < yEnd; y++) {
                    possibleRoomPositions.erase(Position(x,y));
                }
            }
                
            roomArea += size*size;
        } else {
            possibleRoomPositions.erase(roomPos);
        }
    }

    PaveRooms();
    AddEntryPoints();
}

std::vector<Position> Map :: GetPossibleSpawns() {
    return possibleSpawns;
}

void Map :: AddEntryPoints() {
    if( possibleSpawns.size() > 0 ) {
      Position chosenEntry = possibleSpawns[rand()%possibleSpawns.size()]; //Pick a random position
      
      SetTile(chosenEntry.x,chosenEntry.y,ent);
      
      Position chosenExit = chosenEntry;
      while (chosenExit == chosenEntry) {
          chosenExit = possibleSpawns[rand()%possibleSpawns.size()]; //pick another for our exit
      }
      
      SetTile(chosenExit.x,chosenExit.y,ext);
      SetEntryPositions(chosenEntry,chosenExit);
    }
}

void Map :: PaveRoom(Room r) {
    Position startPos = r.GetStartPos();
    Position endPos = r.GetEndPos();
    
    if(IsInBoundaries(startPos) && IsInBoundaries(endPos)) {
        //Paves the inside of a room    
        for (unsigned int y=startPos.y+1; y<endPos.y-1; y++) {
            for (unsigned int x=startPos.x+1; x<endPos.x-1; x++) {
                if (game_grid[y][x] == ntl) {
                    game_grid[y][x] = rom;
                    possibleSpawns.push_back(Position(x,y));
                }
            }
        }
    } else {
        logging -> logline("Cannot pave, room positions out of bounds.");
    }
}

void Map :: PaveRooms() {
      for (unsigned short int i=0; i<roomCount; i++) {
        PaveRoom(rooms[i]);
      }
}

bool Map::IsInBoundaries(int x, int y) {
    if ((x < 0) || (x >= GetGridX()) || (y < 0) || (y >=  GetGridY())) {
        return false;
    } 
    
    return true;
}

bool Map::IsInBoundaries(Position p) {
    return IsInBoundaries(p.x,p.y);
}

bool Map::IsWall(int x, int y) {
    if (!IsInBoundaries(x,y)) {
        return false;
    }
    
    tile t = game_grid[y][x];
    if (t == wa1 || t == wa2 || t == wa3 || t == win) return true;
    else return false;
}

bool Map::IsWall(Position p) {
    return IsWall(p.x, p.y);
}

bool Map::IsPaveable(Position p) {
    if (!IsInBoundaries(p)) {
        return false;
    }

    bool isWall = IsWall(p);
    tile t = game_grid[p.y][p.x];
    bool paveableTile = (t == ntl || t == cor || t == rom || t == dor);
    return !isWall && paveableTile;
}

bool Map::IsTraversable(int x, int y) {
    if (!IsInBoundaries(x,y)) {
        return false;
    } else {
        tile t = game_grid[y][x];
        return tile_library[t].traversible;
    }
}

int Map::EnvironmentCheck(int x, int y)
{
    
    tile t = GetTile(x, y);
    if (t == dor) {
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
 * @return 3 for alive enemy, 4 for dead body (to conform to MovePlayer)
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
                    return 4;
                } else return 4; //dead body 

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
    } else return (EnvironmentCheck(x, y)); 
}

/**
 * @param npcID - an ID pointer for incase an NPC is encountered
 * @return code guide --
 * Movement
 * 0 - moved successfully
 * 1 - door found
 * 2 - trap found
 * 
 * Encounters
 * 3 - Enemy found
 * 4 - Dead body found
 * 
 * Entryways
 * 5 - entrance
 * 6 - exit
 * 
 * 99 - Cannot move
 * Other - determined by EncounterCheck()'s return value
 */
int Map::MovePlayer(int x, int y, int* npcID)
{
    if ((x > 0) || (y > 0) || (x < GetGridX()) || (y < GetGridY())) {
      //Run an encounter check and convert it's return values if needed
      int encounterCode = EncounterCheck(x, y, npcID); 
      if (encounterCode) return encounterCode; //Retun any non-zero value 
    }
    
    tile t = GetTile(x, y);
    if (t == ent) return 5;
    else if (t == ext) return 6;
    else if (t == ded) return 2;

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

tile Map::GetTile(Position p)
{
    return game_grid[p.y][p.x];
}

bool Map::TileIsVisible(int x, int y) 
{
	return visible_grid[y][x];
}

void Map::SetTileVisible(int x, int y, bool b) 
{
	visible_grid[y][x] = b;
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

Door* Map::GetDoor(int x, int y)
{
    return &door_grid[y][x];
}

void Map::SetDoor(int x, int y, Door door)
{
    door_grid[y][x] = door;
}

int Map::GetGridX()
{
    return gridX;
}

int Map::GetGridY()
{
    return gridY;
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
  return;DoorProc
}
 */

int Map::ContainerProc(int x, int y)
{
    return GetContainer(x, y).HasItems();
}

void Map::UnlockDoorTile(int x, int y) {
  int map_tile = GetTile(x, y);
  if (map_tile == dor)
  {
      GetDoor(x, y) -> Unlock();
      // Check to see if the door spans multiple tiles and open those too 
      for (Position neighbor : GetNeighbors(x,y)) {
        if (GetTile(neighbor) == dor) {
            GetDoor(neighbor.x, neighbor.y) -> Unlock();
        }
      }
  };
}

void Map::OpenDoorTile(int x, int y) {
  int map_tile = GetTile(x, y);
  // Check for door tile
  if (map_tile == dor)
  {
      GetDoor(x, y) -> Open();
      // Check to see if the door spans multiple tiles and open those too 
      for (Position neighbor : GetNeighbors(x,y)) {
        if (GetTile(neighbor) == dor) {
            GetDoor(neighbor.x, neighbor.y) -> Open();
        }
      }
  };
}

void Map::AddPath(Path path) {
  for (Position pos : path) {
    tile tile_type = GetTile(pos.x, pos.y);
    if (tile_type != cor && tile_type != dor) {
      logging -> logline("Adding path tile at: " + std::to_string(pos.x) + ", " + std::to_string(pos.y));
      SetTile(pos.x, pos.y, cor);
    } else {
      return;
    }
  }
}

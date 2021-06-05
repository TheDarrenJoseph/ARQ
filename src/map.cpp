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
  if ( !(room->GetEndPos().x>GRID_X || room->GetEndPos().y>GRID_Y) ) {
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
  if (x == GRID_X-size && y == 0) {
      AddOppositeDoors(room, door, TOP, RIGHT);
      return true;
  }
  //BOTTOM LEFT
  if (x == 0 && y == GRID_Y-size) {
      AddOppositeDoors(room, door, BOTTOM, LEFT);
      return true;
  }
  //TOP LEFT
  if (x == 0 && y == 0) {
      AddOppositeDoors(room, door, TOP, LEFT);
      return true;
  }
  //BOTTOM RIGHT
  if (x == GRID_X-size && y == GRID_Y-size) {
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
  if (x == GRID_X-size) {
      AddDoor(room, door,LEFT);
      return true;
  }
  
  //Bottom
  if (y == GRID_Y-size) {
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

/** Subfunction for PathRooms that checks that a suitable path exists for the player to traverse the level
 * 
 */
bool Map :: LevelPathValid() {
    Path* levelPath = new Path();
    bool isValid = AStarSearch(this->GetEntryPosition(),this->GetExitPosition(),levelPath);
    delete levelPath;
    return isValid;
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
                if (game_grid[y][x] == ntl) 
                    game_grid[y][x] = rom;
		possibleSpawns.push_back(Position(x,y));
            }
        }
    }
}

int Map :: ManhattanPathCostEstimate(Position startPos, Position endPos) {
    //straight-line cost is the minimum absolute distance between the two
    int absoluteCost = (abs(endPos.x-startPos.x)+abs(endPos.y-startPos.y));
    //std::cout << "abs cost of \n" << absoluteCost;
    return absoluteCost;
}

/** Part of A* Implementation. Evaluates a single neighbor node, defined as a position in our game grid.
 * 
 * @param neighbor The position of the node to evaluate
 * @param endPos   The target position for the path
 * @param currentNode the current node, which the neighbor is adjacent to (costs go between these two)
 * @param visitedNodes a set of all visited nodes, this function will add the neighbor to this set if not already in there (The node will only be evaluated if it's not in the set)
 * @param unvisitedNodes a set of all nodes available to be evaluated, this function will add the neighbor to it so that it can become the currentNode for later evaluations (assuming currentNode is passed as the lowest cost position in the unvisited set/the start position)
 * @param navigatedNodes a map of all nodes that have been evaluated and their distances saved, acting as a linked list from any point in the map back to the original currentNode
 * @param nonHeuristicCostMap a map of integer costs from our start position (based purely on distance) (requires all nodes with int max/infinity values by default, and a value of 0 for our start position)
 * @param heuristicCostMap a map of costs using our heuristic (requires all nodes with int max/infinity values by default, and a heuristic cost applied to our start position)
 */
void Map :: EvaluatePathNeighborNode(Position neighbor, Position endPos, Position currentNode, std::set<Position>* visitedNodes, std::set<Position>* unvisitedNodes, std::map<Position,Position>* navigatedNodes, std::map<Position,int>* nonHeuristicCostMap,std::map<Position,int>* heuristicCostMap ) {
   
    //If the node has not been "visited"/evaluated, evaluate it's cost
    if (visitedNodes->count(neighbor) == 0) {
        visitedNodes->insert(neighbor); //add this node straight to the visited list

        //std::cout << "evaluating " << neighbor.x << ","<< neighbor.y << "\n";
        
        int nonHeuristicScore = (nonHeuristicCostMap->at(currentNode))+(abs(currentNode.x-neighbor.x)+abs(currentNode.y-neighbor.y)); //add a constant factor of 1 tile distance to the current path score
        if (IsWall(neighbor.x,neighbor.y) || IsWall(currentNode.x,currentNode.y)) return; //nonHeuristicScore = std::numeric_limits<int>::max(); //Increase cost for walls
        
        //If neighbor isn't in the unvisited list, add it, allows this function to evaluate the node later
        if (unvisitedNodes->count(neighbor)==0) {
            unvisitedNodes->insert(neighbor);
        } else if (nonHeuristicScore >= nonHeuristicCostMap->at(neighbor)) return; //ignore this node if the fixed path score is greater than it's cost
        
        //Otherwise, save the path data
       // std::cout << "saving " << neighbor.x << neighbor.y << "\n";
        (*navigatedNodes)[neighbor] = currentNode; //sets the neighbors path to backtrace to the start pos
        
        (*nonHeuristicCostMap)[neighbor] = nonHeuristicScore;
        unsigned int heuristicCost = (*nonHeuristicCostMap)[neighbor]+ManhattanPathCostEstimate(neighbor, endPos); //Add the manhattan costing to the non heuristic values
        (*heuristicCostMap)[neighbor] = heuristicCost; //add the cost value to this spot on the heuristic cost map
        
    }

}
 

/** Navigates along the map of navigated nodes from a given position until the path ends, 
 *  it builds a Path of these nodes and returns it.
 * 
 * @param navigated
 * @param pathPosition
 * @return The path followed from the position passed
 */
void ConstructPath(std::map<Position,Position> navigated, Position pathPosition, Path* endPath) {
    while (navigated.count(pathPosition) > 0) {
        //std::cout << "Path node " << pathPosition.x << " " << pathPosition.y << "\n"; 
        pathPosition= navigated.at(pathPosition); //next node navigated to
        (*endPath).push_back(pathPosition);
    }
}


/** Part of A* Implementation.
 * Evaluates all unvisited nodes based on the heuristicCostMap (the lowest heuristic cost nodes are picked and evaluated)
 * 
 * @param currentNode the startPosition 
 * @param previousNode
 * @param endPos
 * @param visitedNodes
 * @param unvisitedNodes
 * @param navigatedNodes
 * @param nonHeuristicCostMap
 * @param heuristicCostMap
 * 
 * @return true if the destination was reached (should mean a complete path), false otherwise
 */
bool Map :: EvaluateNodes(Position currentNode, Position endPos, std::set<Position>* visitedNodes, std::set<Position>* unvisitedNodes,std::map<Position,Position>* navigatedNodes, std::map<Position,int>* nonHeuristicCostMap, std::map<Position,int>* heuristicCostMap ) {
    Position previousNode = Position(-1,-1); //-1 to fail boundary check, and not be checked on the first pass-through
    
    //While we haven't run out of nodes to visit, grab the lowest cost, and try to build our path
    while(!unvisitedNodes->empty()) {
        std::map<Position,int> possibleNewNodes = (*heuristicCostMap); //create a new map as a copy of all heuristic scores for nodes, so we can modify it
        
        bool foundNewNode = false;
        
        while (!foundNewNode) {
			//Check for the lowest node using our CompareMapLessThanCost comparison function
            std::pair<Position, int> lowestCostNode = *std::min_element(possibleNewNodes.begin(), possibleNewNodes.end(), &Map::CompareMapLessThanCost);
            currentNode = lowestCostNode.first; 
            
            //Only care if this node is unvisited
            if (unvisitedNodes->count(currentNode) == 1) {
                foundNewNode = true;
            } else if (possibleNewNodes.empty()){
                std::cout << "Ran out of new nodes.. \n";
                return false;
            } else {
                possibleNewNodes.erase(currentNode);
            }
            
        }
        
        //Check that we've looped once (by the pos now being valid), if so make sure we haven't repeated the node search
        if (IsInBoundaries(previousNode.x,previousNode.y)) {
            if (currentNode == previousNode) {
                std::cout << "Node repeated! Aborting pathing here..\n";
                return false;
            } 
        } 
        
        //Print data about the current node
        //std::cout << "Lowest cost node=" << currentNode.x << " " << currentNode.y << "\n";
        unvisitedNodes->erase(currentNode);
        visitedNodes->insert(currentNode);    
        
        //Check for reaching our goal
        if (currentNode == endPos) { 
            return true;
        } else { //Evaluate all neighbors of the current node
            for (Position p : GetNeighbors(currentNode.x,currentNode.y)) {
                EvaluatePathNeighborNode(p,endPos,currentNode, visitedNodes, unvisitedNodes, navigatedNodes, nonHeuristicCostMap, heuristicCostMap );
            }
        }
        
        previousNode = currentNode;
       
        
    }
    
  return false;  
}

/** Builds a path between two points, avoiding walls, currently only works with startPos<endPos (top-left to bottom-right)
 * 
 * @param startPos The position on the map to path from
 * @param endPos   The position on the map to path to
 * @param endPath a pointer to the finished path from the result of our search
 */
bool Map :: AStarSearch(Position startPos, Position endPos, Path* endPath) {
        if (IsInBoundaries(startPos.x,startPos.y) && IsInBoundaries(endPos.x, endPos.y) && !IsWall(startPos.x,startPos.y) && !IsWall(endPos.x,endPos.y)) {
            std::set<Position> visitedNodes;
            std::set<Position> unvisitedNodes;
          
            std::map<Position,Position> navigatedNodes; //A map of which node was navigated to from each node
            std::map<Position,int> nonHeuristicCostMap;//costs without our heuristic applied (just based on absolute distance) 
            std::map<Position,int> heuristicCostMap; //a map of costs with heuristics applied for estimated total cost to endPos, with infinity default vals
            
            //Initialise our maps
            for     (unsigned int x=0; x<GRID_X; x++) {
                for (unsigned int y=0; y<GRID_Y; y++) {
                    heuristicCostMap[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
                    nonHeuristicCostMap[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
                }
            }
            
            unvisitedNodes.insert(Position(startPos.x,startPos.y));
            heuristicCostMap[startPos] = ManhattanPathCostEstimate(startPos, endPos); //Manhattan cost the startPos default cost (the distance between the two points absolutely)
            nonHeuristicCostMap[startPos] = 0; //assign the starPos to have a cost of 0

            //Start evaluating our nodes
            if (EvaluateNodes(startPos,endPos,&visitedNodes,&unvisitedNodes,& navigatedNodes, &nonHeuristicCostMap, &heuristicCostMap) == true) { 
              ConstructPath(navigatedNodes, endPos, endPath); //Build a path to the current endPos, set as endPath 
              return true;
            } 
        }
    
    return false;
}

void Map :: PaveRooms() {
      for (unsigned short int i=0; i<roomCount; i++) {
        PaveRoom(rooms[i]);
      }
    
//    for (Room r : rooms) {
//         PaveRoom(r);
//    }
}

bool Map :: PathRooms() { 
  Room initialRoom = rooms[0];
  int doorCount=0;
  Door* startDoors = initialRoom.getDoors(&doorCount);

     for (unsigned short int roomNo=0; roomNo<roomCount; roomNo++) {

         unsigned short int startX = startDoors[0].posX;
         unsigned short int startY = startDoors[0].posY;
         unsigned short int targetX = startX;
         unsigned short int targetY = startY;
         
         Position startPos = Position(startX,startY);
         Position targetPos = startPos;
         
         int targetDoorCount;
         Door* targetDoors = rooms[roomNo].getDoors(&targetDoorCount);
         
         for (unsigned short int doorNo=0; doorNo<targetDoorCount; doorNo++) {
             targetX = targetDoors[doorNo].posX;
             targetY = targetDoors[doorNo].posY;
             
             targetPos = Position(targetX,targetY);
             
             Path* doorPath = new Path();
             
             if (AStarSearch(startPos,targetPos,doorPath) == true) {
                 for (Position p : (*doorPath) ) {
                     if(game_grid[p.y][p.x] == ntl) game_grid[p.y][p.x] = cor;
                     //game_grid[p.y][p.x] = cor;
                 }
             }
             
             delete (doorPath);
             
         }
                  
     }
  
  //Return based on whether or not you can reach the exit
  return LevelPathValid();
  //return true;
}

bool Map::IsInBoundaries(int x, int y) {
    if ((x < 0) || (x > GRID_X) || (y < 0) || (y > GRID_Y)) {
        return false;
    } 
    
    return true;
}

bool Map::IsInBoundaries(Position p) {
    int x = p.x;
    int y = p.y;
    
    return IsInBoundaries(x,y);
}

bool Map::IsWall(int x, int y) {
    if (!IsInBoundaries(x,y)) {
        return false;
    }
    
    tile t = game_grid[y][x];
    if (t == wa1 || t == wa2 || t == wa3 || t == win) return true;
    else return false;
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
    if ((x > 0) || (y > 0) || (x < GRID_X) || (y < GRID_Y)) {
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

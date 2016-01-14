#include "map.h"
#include "room.h"

std::list<Position> Map:: GetNeighbors(int x, int y) {
    std::list<Position> possibleNeighbors;
    std::list<Position> neighbors = *new std::list<Position>();    

       possibleNeighbors.push_back(Position(x++, y));
       possibleNeighbors.push_back(Position(x--, y));
       possibleNeighbors.push_back(Position(x, y++));
       possibleNeighbors.push_back(Position(x, y--));

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
  tile doorTile = cd1;
  int doorX, doorY;
  
  for (int doorNo = (rand() % 4 + 1); doorNo > 0; doorNo--) {
    if (room->AddDoor(doorTile, static_cast<side>(rand()%4), &doorX, &doorY)) {
        game_grid[doorY][doorX] = doorTile;
    }
  }
  
  return true;
}

/** Generates a layer of the dungeon, creating rooms and corridors procedurally.*/
void Map::CreateMap(int roomChance) {
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
            
            if (roomCount<MAX_ROOMS && randChance > roomChance ) {
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

}

/** Subfunction for PathRooms that checks that a suitable path exists for the player to traverse the level
 * 
 */
bool Map :: LevelPathValid() {
    //Plot a level path? (from the first room to the last, and include an exit?)
    return false;
}

void Map :: PaveRoom(Room r) {
    Position startPos = r.GetStartPos();
    Position endPos = r.GetEndPos();

    //Paves the inside of a room    
    for (unsigned int y=startPos.y+1; y<endPos.y-1; y++) {
        for (unsigned int x=startPos.x+1; x<endPos.x-1; x++) {
            if (game_grid[y][x] == ntl)  game_grid[y][x] = rom;
        }
    }
}

int ManhattanPathCostEstimate(Position startPos, Position endPos) {
    return (abs(endPos.x-startPos.x)+abs(endPos.y-startPos.y));
}

void EvaluatePathNeighbor(Position neighbor, Position endPos, Position currentNode, int* nonHeuristicScore, std::set<Position>* visitedNodes, std::set<Position>* unvisitedNodes, std::map<Position,Position>* navigatedNodes, std::map<Position,int>* nonHeuristicCostMap,std::map<Position,int>* heuristicCostMap ) {
    
    //A* Search algorithm pseudocode for neighbor evaluation
    //    if neighbor not in VisitedSet   {
    //           tentativeNonHeuristicScore := nonHeuristicMap[current] + 1 // length of this path.
    //          
    //        if (neighbor not in UnvisitedSet) {	// Discover a new node
    //                UnvisitedSet.Add(neighbor)
    //        }
    //      else if tentative_g_score >= g_score[neighbor] return	// This is not a better path.
    //              
    //        // This path is the best until now. Record it!
    //            navigated[neighbor] := current
    //            nonHeuristicScoreMap[neighbor] := tentativeNonHeuristicScore
    //            heuristicScoreMap[neighbor] := nonHeuristicMap[neighbor] + heuristic_cost_estimate(neighbor, goal)
    //    }
    
    
    //If the node has not been "visited"/evaluated, evaluate it's cost
    if (visitedNodes->count(neighbor)==0) {
            (*nonHeuristicScore) = (nonHeuristicCostMap->at(currentNode))+(abs(currentNode.x-neighbor.x)+abs(currentNode.y-neighbor.y)); //add a constant factor of 1 tile distance to the current path score
    
        //If neighbor isn't in the unvisited list, add it, allows this function to evaluate the node later
        if (unvisitedNodes->count(neighbor)==0 ) {
            unvisitedNodes->insert(neighbor);
        } else if (nonHeuristicCostMap->count(neighbor) == 1 && (*nonHeuristicScore) >= nonHeuristicCostMap->at(neighbor)) return; //ignore this node if the fixed path score is greater than it's cost
                
        //Otherwise, save the path data
        (*navigatedNodes)[neighbor] = currentNode; //sets the neightbors path to backtrace to the start pos
        
        (*nonHeuristicCostMap)[neighbor] = (*nonHeuristicScore);
        int heuristicCost = (*nonHeuristicCostMap)[neighbor]+ManhattanPathCostEstimate(neighbor, endPos); //Add the manhattam costing to the non heuristic values
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
       pathPosition= navigated.at(pathPosition); //next node navigated to
        (*endPath).push_back(pathPosition);
    }

}

void Map :: AStarSearch(Position startPos, Position endPos, Path* endPath) {
    
    if (IsInBoundaries(startPos.x,startPos.y) && IsInBoundaries(endPos.x, endPos.y)) {
        std::set<Position> visitedNodes;
        std::set<Position> unvisitedNodes;
        
        std::map<Position,Position> navigatedNodes; //A map of which node was navigated to from each node
        
        std::map<Position,int> nonHeuristicCostMap;//costs without our heuristic applied??? (just based on distance) 
        int nonHeuristicScore=0;
        
        std::map<Position,int> heuristicCostMap; //a map of costs with heuristics applied for estimated total cost to endPos, with infinity default vals
        
        for (int x=0; x<GRID_X; x++) {
            for (int y=0; y<GRID_Y; y++) {
                unvisitedNodes.insert(Position(x,y));
                heuristicCostMap[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
                nonHeuristicCostMap[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
            }
        }
        
        heuristicCostMap[startPos] = ManhattanPathCostEstimate(startPos, endPos); //Manhattan cost the startPos default cost (the distance between the two points absolutely)
        nonHeuristicCostMap[startPos] = 0; //asign the starPos to have a cost of 0
        Position visitingNode = Position(startPos.x,startPos.y);
        
        
        //While we haven't run out of nodes to visit, grab the lowest cost, and try to build our path
       // while(!unvisitedNodes.empty()) {
            
            //Grab the lowest cost node
            //std::pair<Position, int> lowestCostNode = *std::min_element(heuristicCostMap.begin(), heuristicCostMap.end(), &Map::CompareMapCost);
            std::pair<Position, int> lowestCostNode = *std::min_element(heuristicCostMap.begin(), heuristicCostMap.end(), &Map::CompareMapLessThanCost);
            Position currentNode = lowestCostNode.first; //The current node is the position of the lowest cost node in the heuristicCostMap

           // std::cout << "Lowest cost node=" << unvisitedNodes[currentNode];
            
            
                unvisitedNodes.erase(currentNode);
                visitedNodes.insert(currentNode);    
            
         //   if (currentNode!=endPos) { 
                //Evaluate all 4 neighbors of the current node
         //       for (Position p : GetNeighbors(visitingNode.x,visitingNode.y)) {
                   // EvaluatePathNeighbor(p,endPos,currentNode,&nonHeuristicScore, &visitedNodes, &unvisitedNodes, &navigatedNodes, &nonHeuristicCostMap, &heuristicCostMap );
        //        }
                
        //    }
            
            
           
            
     //   }
        
         ConstructPath(navigatedNodes, endPos, endPath); //Build a path to the current endPos, set as endPath
    }
}

void Map :: PathRooms(){
  //Pave our rooms  
  for (int i=0; i<roomCount; i++) {
        PaveRoom(rooms[i]);
  }
  
  Room initialRoom = rooms[0];
  int doorCount=0;
  Door* startDoors = initialRoom.getDoors(&doorCount);

  //int randDoorChoice = rand() % doorCount; //pick a door at random to pathfind from
  
  //Path doorPath;
  
  //while (!LevelPathValid()) {
     for (int i=1; i<roomCount; i++) {      
         int startX = startDoors[0].posX;
         int startY = startDoors[0].posY;
         Position startPos = Position(startX,startY);

         int targetDoorCount;
         Door* targetDoors = rooms[i].getDoors(&targetDoorCount);
         int targetX = targetDoors[0].posX;
         int targetY = targetDoors[0].posY;
         Position targetPos = Position(targetX,targetY);
         
         Path* doorPath = new Path();
         
         AStarSearch(startPos,targetPos,doorPath);
         
         for (Position p : (*doorPath) ) {
             
             game_grid[p.y][p.x] = wa1;
         }
         
         delete (doorPath);
     }
  //}
  
    //start at rooms[0]
    
    //Link to every room if possible
    //Pathfind to one of the room's doors7
}

bool Map::IsInBoundaries(int x, int y) {
    if ((x < 0) || (x > gridX) || (y < 0) || (y > gridY)) {
        return false;
    } else return true;

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

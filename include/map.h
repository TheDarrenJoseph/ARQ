#ifndef GRID_H
#define GRID_H

#define GRID_X 100 //Fallback/default size values
#define GRID_Y 30

#include "position.h"
#include "tile.h"
#include "containers.h"
#include "characters.h"
#include "room.h"
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <algorithm> 
#include <vector>

class Map
{
private:
    tile game_grid[GRID_Y][GRID_X];
    bool visible_grid[GRID_Y][GRID_X];
    Container container_grid[GRID_Y][GRID_X];
    
    static const int MAX_ROOMS= ((GRID_X*GRID_Y)/9); //assuming a room should at minimum use 9 tiles of space, divide the total map size by this
    Room rooms[MAX_ROOMS];
    
    void initMap(int sizeX,int sizeY, int maxNPCS, NPC* npcs, Player* p) {
		if (sizeX<=GRID_X && sizeY<=GRID_Y) {
            gridX = sizeX;
            gridY = sizeY;
        } else  Map();
        
        for (int x=0; x < gridX; x++) {
            for (int y=0; y < gridY; y++) {
                game_grid[y][x] = ntl; //set all tiles to no tile by default to avoid segfaults
                visible_grid[y][x] = false;
            }
        }
        
        MAX_NPCS = maxNPCS;
        this->npcs = npcs;
        
        player = p;
        
        roomCount = 0;
        
        int roomChance = rand()%100-50; //Between 0-49% chance of rooms
        CreateMap(roomChance);
        
//        //Initialise container spaces
//        for (int x = 0; x < GRID_X; x++) {
//            for (int y = 0; y < GRID_Y; y++) {
//                container_grid[y][x] =Container();
//            }
//         }
	}
    
    //Compares the pairs within a map based on their rvalues, returns true if lval is < than the rval
    static bool CompareMapLessThanCost(std::pair<Position,int> lval, std::pair<Position,int> rval) {
        return (lval.second < rval.second); //compares the int costs of the two PositionCosts, allowing sorting of positions by their movement cost
    }
    
    int gridX=0;
    int gridY=0;
    
    int MAX_NPCS=0;
  
    int roomCount=0;
    
    Player* player=NULL;
    NPC* npcs     =NULL;
    
    std::vector<Position> possibleSpawns;
    
    Position entryPosition = Position(0,0);
    Position exitPosition = Position(0,0);
    
   
    public:
    void PaveRooms();
        
    void SetEntryPositions(Position entry, Position exit);
    void AddEntryPoints();
    
    std::vector<Position> GetPossibleSpawns();
    Position GetEntryPosition();
    Position GetExitPosition();    
    
    
    bool IsInBoundaries(int x, int y);
    bool IsInBoundaries(Position p);
    bool IsWall(int x, int y);
    bool IsTraversable(int x, int y);
    
    /**
     * Tile codes --
     * 0 - fine
     * 1 - Door encountered
     * 2 - trap encountered
     * 3 - entrance
     * 4 - exit
     * 
     * @return returns an integer code for if an environment object is encounterred
     */
    int EnvironmentCheck(int x, int y);

    int GetRoomCount() const {
        return roomCount;
    }

    Room* GetRooms() {
        return &rooms[0];
    }

    std::list<Position> GetNeighbors(int x, int y);
    
    bool CanPlaceItems(int x, int y);
    
    /** returns an integer code to determine whether a player has encountered a living or dead creature
     *  Code Guide - 
     *  *  0 - Nothing
     *  *  1 - living enemy
     *  *  2 - dead body
     *  */
    int EncounterCheck(int x, int y, int* npcID);
    
    
    /** Attempts to move a given character, calls IsTraversible and EnvironmentCheck and returns their integer codes
     * 
     * @param c
     * @param x
     * @param y
     * @param pos_x
     * @param pos_y
     * 
     * @return Code Guide - 
     * 0 - Successful movement 
     * 1 - Door found
     * 2 - trap found
     */
    int MoveCharacter(Character* c, int x, int y);
    
        
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
     */
    int MovePlayer( int x, int y, int* npcID);

    int MoveNPCS ();
    void DropCharacterItems(Character* c);
    int DropPlayerItem(Player* p, Item* thisItem, int index);
   
    bool  LevelPathValid();
    void PaveRoom(Room r);
    
    int ManhattanPathCostEstimate(Position startPos, Position endPos);
    bool EvaluateNodes(Position currentNode, Position endPos ,std::set<Position>* visitedNodes, std::set<Position>* unvisitedNodes,std::map<Position,Position>* navigatedNodes, std::map<Position,int>* nonHeuristicCostMap, std::map<Position,int>* heuristicCostMap );
    void EvaluatePathNeighborNode(Position neighbor, Position endPos, Position currentNode, std::set<Position>* visitedNodes, std::set<Position>* unvisitedNodes, std::map<Position,Position>* navigatedNodes, std::map<Position,int>* nonHeuristicCostMap,std::map<Position,int>* heuristicCostMap );
    bool AStarSearch(Position startPos, Position endPos, Path* endPath);
    bool PathRooms();
    
        
    tile GetTile(int x, int y);
    tile GetTile(Position p);
    
    bool TileIsVisible(int x, int y);
    void SetTileVisible(int x, int y, bool b);
    
    void SetTile(int x, int y,tile t);

    const Item* GetItem(int x, int y);
    const Item* GetContainerItem(int containerX, int containerY,unsigned long int index);
    
    void AddToContainer(int x, int y,const Item* i);
    
    //Returns whether there is an area at x,y, and if it contains items
    int ContainerProc (int x ,int y); 

    Container GetContainer(int x, int y);
    void SetContainer(int x, int y, Container a);
    
    int GetGridX();
    int GetGridY();
    
    bool CanPlaceRoom(Room* room);
    void CreateWalls(Room* room);
    
    void AddOppositeDoors(Room* room, tile doorTile, side one, side two);
    
    /**
     *  x - the x of the top-left of the room
     *  y - the y pos of the top-left of the room
     *  size - the square size of the room
     *  room - Room pointer that will point to the allocated room if true is returned
     */
    bool CreateRoom(int x, int y, int size, Room* room);
    
    //creates a new level for the dungeon, seed influences certain random attributes
    void CreateMap(int seed);
    
    void InitAreas();

    //Attempts at default initialisation, !FIX NULL USAGE HERE, OR make it safe!
    Map() : Map(4,NULL,NULL) {
        
    }
    
	//Default map size constructor
    Map (int maxNPCS, NPC* npcs, Player* p) {
		gridX=GRID_X;
        gridY=GRID_Y;  
        
		initMap(gridX,gridY,maxNPCS,npcs,p);  	
	}
    
    //Custom map size constructor
    Map(int x,int y, int maxNPCS, NPC* npcs, Player* p) {
		initMap(x,y,maxNPCS,npcs,p);    
    }
    
    void copyMap(const Map& m) {
        this->MAX_NPCS = m.MAX_NPCS;

        //Copying containers and tiles from the address given,
        for (int x=0; x<GRID_X; x++) {
            for(int y=0; y<GRID_Y; y++) {
                this->container_grid[y][x] = m.container_grid[y][x];
                this->game_grid[y][x] = m.game_grid[y][x];
            }
        }
        
        this->npcs = m.npcs;
        this->player = m.player;
    }
    
    //Overriding assignment operator
    Map& operator=(const Map& m) {  
        copyMap(m);
        return *this;
    }
    
    //Copy constructor  
    Map(const Map& m) {  
        copyMap(m);
    }
    
    ~Map()
    {
//        //Delete memory and references for our containers
//        for (int x=0; x<GRID_X; x++) {
//            for (int y=0; y<GRID_Y; y++) {
//                delete container_grid[y][x];
//            }
//        }

    }


};

#endif

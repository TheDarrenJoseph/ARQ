#ifndef GRID_H
#define GRID_H

#define GRID_X 50 //Fallback/default size values
#define GRID_Y 15

#include "tile.h"
#include "containers.h"
#include "characters.h"
#include "room.h"
#include <cstdlib>

class Map
{
    private:
    
    tile game_grid[GRID_Y][GRID_X];
    Container container_grid[GRID_Y][GRID_X];
    
    static const int MAX_ROOMS= ((GRID_X*GRID_Y)/9); //assuming a room should at minimum use 9 tiles of space, divide the total map size by this
    Room rooms[MAX_ROOMS];
    
    //Room list
    //Sort rooms by co-ordinates?
    //When creating a room, you can check against co-ordinates and size
    
    int gridX=0;
    int gridY=0;
    
    int MAX_NPCS=0;
  
    int roomCount=0;
    
    Player* player=NULL;
    NPC* npcs     =NULL;
    
    public:

    bool IsTraversable(int x, int y);
    
    /**
     * Tile codes --
     * 0 - fine
     * 1 - Door encountered
     * 2 - trap encountered
     * 
     * @return returns an integer code for if an environment object is encounterred
     */
    int EnvironmentCheck(int x, int y); 
    
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
    
        
    tile GetTile(int x, int y);
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
    
    bool CanPlaceRoom(int x, int y, int size);
    
    void CreateWall(int x, int y, int size, bool horizontal);
    
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
    Map() : Map(GRID_X,GRID_Y,4,NULL,NULL) {
        gridX=GRID_X;
        gridY=GRID_Y;  
    }
    
    
    Map(int x,int y, int maxNPCS, NPC* npcs, Player* p) {
       
        if (x<=GRID_X && y<=GRID_Y) {
            gridX = x;
            gridY = y;
        } else {
           Map();
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
    
    void copyMap(const Map& m) {
         this->MAX_NPCS = m.MAX_NPCS;

        //container grid
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

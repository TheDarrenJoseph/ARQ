// ---- The ASCII Roguelike Quester -- Experimental project
// Author: Darren Joseph             
// Created: Dec 16, 2012           
// LICENSE -- See the "LICENSE" file (in root directory of ARQ) for details

#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>

#include <cmath>
#include <sstream>
#include <curses.h>

#include "game.h"
#include "logging.h"

bool running = true;

void Quit()
{
  running = false;
}

int main()
{   
    Logging* logging = &logging -> getInstance();

    logging -> logline("Starting ARQ...");
    srand(time(NULL));
    
    const int MAX_NPCS = 1; //Number-1 to allow for 0 indexing
    
    NPC npcs[MAX_NPCS] = {Goblin()};

    Player* thisPlayer = new Warrior();
    
    //WARNING - if using a container*, watch for losing the pointer when the player drops/moves it!
    Container* bag = new Container(98,"Bag","X",2,2,0,true); //inventory testing
    Container* box = new Container(98,"Box","X",2,2,0,true); //inventory testing
    thisPlayer->AddToInventory(bag);
    thisPlayer->AddToInventory(box);
            
    Item* testItem = new Item(98,"Test Item","X",2,2,0,true); //inventory testing
    thisPlayer->AddToInventory(testItem);
    
    
    //Map and playerUI are new initialised as the GameEngine will delete all of these when done (these pointers get modified for new maps)
    Map* thisMap = new Map(MAX_NPCS,&npcs[0],thisPlayer);
    logging -> logline("Created initial map of size: " + std::to_string(thisMap -> GetGridX()+1) + ", " + std::to_string(thisMap -> GetGridY()+1));


    CursesUI thisUI = CursesUI();
    PlayerUI* playerUI = new PlayerUI(MAX_NPCS,&thisUI,thisMap,thisPlayer,&npcs[0]);
    
    GameEngine thisGame = GameEngine(thisPlayer,&npcs[0], MAX_NPCS, thisMap, &thisUI, playerUI);
    
    thisGame.StartGame();
    
    logging -> logline("Closing ARQ...");
    delete(thisPlayer);
    
    //Clean up our container mess
    delete(bag);
    delete(box);
    delete(testItem);
    
    //Free each npc?
    //for (int npcNo=0; npcNo<MAX_NPCS; npcNo++) {
     //   delete(&npcs[npcNo]);
    //}
            
    
    return 0;
}

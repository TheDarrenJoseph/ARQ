// ---- The ASCII Roguelike Quester -- Experimental project
// Author: Darren Joseph             
// Created: Dec 16, 2012           
// LICENSE -- See the "LICENSE" file (in root directory of ARQ) for details

#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <list>

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
    Container* bag = new Container(2, OBJECT, "Bag","X",2,2,100, 5,true); //inventory testing
    Container* box = new Container(3, OBJECT,"Box","X",2,2,100, 5,true); //inventory testing
    thisPlayer->AddToInventory(bag);
    //thisPlayer->AddToInventory(box);
    

    std::list<Item*> testItems;
    //inventory testing
    for (int i=0; i<50; i++) {
      Item* testItem = new Item(3+i,"Test Item " + std::to_string(i),"X",2,2, 100 + i,true); 
      thisPlayer->AddToInventory(testItem);
      testItems.push_back(testItem);
    }

    CursesUI thisUI = CursesUI();

    // TODO fix assignment operator (Segfault)
    //Container* testChest = new Container(4, OBJECT,"Tester's Chest","O",2,2,100, 5,true); //inventory testing
    //Position playerPos = thisPlayer -> GetPosition();
    //thisMap -> SetContainer(playerPos.x+1, playerPos.y+1, *testChest);
    
    Map* thisMap = NULL;
    PlayerUI* playerUI = new PlayerUI(MAX_NPCS,&thisUI,thisMap,thisPlayer,&npcs[0]);
    GameEngine thisGame = GameEngine(thisPlayer,&npcs[0], MAX_NPCS, &thisUI, playerUI);
    thisMap = thisGame.GetMap();

    thisGame.StartGame();
    
    logging -> logline("Closing ARQ...");
    delete(thisPlayer);
    
    //Clean up our container mess
    delete(bag);
    delete(box);
    for (Item* testItem : testItems) {
      delete(testItem);
    }
    //Free each npc?
    //for (int npcNo=0; npcNo<MAX_NPCS; npcNo++) {
     //   delete(&npcs[npcNo]);
    //}
            
    
    return 0;
}

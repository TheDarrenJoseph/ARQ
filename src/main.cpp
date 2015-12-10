/////////////The ASCII Roguelike Quester -- Experimental project////////
/////////////////////////////////LINUX BUILD////////////////////////////
//                                                                    //
//  Author: Rave Kutsuu              Version : 0.89                   //
//  Created: Dec 16, 2012            Last Modified: 20 July, 2014     //
//                                                                    //
////////////////////////////////////////////////////////////////////////
//LICENSE
// See the "LICENSE" file (in root directory of ARQ) for details
////////////////////////////////////////////////////////////////////////
//CREDITS                                                            
// 1. The Beginner's Guide to Roguelike Development in C/C++ --       
// http://www.kathekonta.com/rlguide/index.html //(28/08/2013)        
                                                                 
// 3. NCURSES Programming HOWTO --                                    
//http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/               
                                                       
////////////////////////////////////////////////////////////////////////

//BULD COMMANDS
//Linux G++ -- "g++ -Wall -I../include -o "%e" "%f" -lncurses -std=gnu++11 characters.cpp ui.cpp grid.cpp items.cpp"

#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>

#include <cmath>
#include <sstream>
#include <curses.h>

#include "game.h"

bool running = true;

void Quit()
{
  running = false;
}

int main()
{
    /*Main initialisation of the game, use of new is avoided, instead creating every element for the game
    as a local variable in this scope so that we can save on memory management.
     */
    
    const int MAX_NPCS = 1; //Number-1 to allow for 0 indexing
    
    NPC npcs[MAX_NPCS] = Goblin();

    Player* thisPlayer = new Warrior();
    
    thisPlayer->SetPos (1,1);
    
    //WARNING - if using a container*, watch for losing the pointer when the player drops/moves it!
    Container* c = new Container(98,"Bag","X",2,2,0,true); //inventory testing
    thisPlayer->AddToInventory(c);
    
    Container* ct = new Container(98,"Box","X",2,2,0,true); //inventory testing
    thisPlayer->AddToInventory(ct);
            
    Item* i = new Item(98,"Swoop","X",2,2,0,true); //inventory testing
    thisPlayer->AddToInventory(i);
    
    Map thisMap =  Map(50,15,MAX_NPCS,&npcs[0],thisPlayer);
    CursesUI thisUI = CursesUI();
    PlayerUI playerUI = PlayerUI(MAX_NPCS,&thisUI,&thisMap,thisPlayer,&npcs[0]);
    
    GameEngine thisGame = GameEngine(thisPlayer,&npcs[0], MAX_NPCS, &thisMap, &thisUI, &playerUI);
    
    //npcs[MAX_NPCS].Kill();
    
    thisGame.StartGame();
    
    //Why does a delete fail here??
   // free(thisPlayer);
    delete(thisPlayer);
    
    //Clean up our container mess
    delete(c);
    delete(ct);
    delete(i);
    
    //Free each npc?
    //for (int npcNo=0; npcNo<MAX_NPCS; npcNo++) {
     //   delete(&npcs[npcNo]);
    //}
            
    
    return 0;
};

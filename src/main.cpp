//////////////////////The ASCII Roguelike Quester -- Experimental project/////////////////
/////////////////////////////////LINUX BUILD//////////////////////////////////////////////
//                                                                                      //
//  Author: Rave Kutsuu                     Version : 0.89                              //
//  Created: Dec 16, 2012                   Last Modified: 20 July, 2014                //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREDITS                                                                                                                //
// 1. The Beginner's Guide to Roguelike Development in C/C++ -- http://www.kathekonta.com/rlguide/index.html (28/08/2013) //
// 3. NCURSES Programming HOWTO --  http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/                                  // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//BULD COMMANDS
//Linux G++ -- "g++ -Wall -I../include -o "%e" "%f" -lncurses -std=gnu++11 characters.cpp ui.cpp grid.cpp items.cpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>

#include <cmath>
#include <sstream>
#include <curses.h>

#include "game.h" 
#include "grid.h"

#include "ui.h"


using namespace std;

bool running;
const std::string VERSION = "0.89 Linux Native";

Player * player;

void SetRunning(bool state)
{
  running = state;
};
 
string GetVersion()
{
  return VERSION;
}

void GameLoop()
{
  //Draw main elements
  DrawMap (); 
  DrawItems (); 
  DrawContainers();
  DrawNPCS();
 
  //Draw player UI elements
  DrawPlayerInv(player);
  DrawPlayerEquip(player);
  DrawPlayerStats (player);  

  player->Draw (); //Draw the player

  UpdateUI(); //refresh all windows
  
  GetPlayerInput(player); //take input from the player
}
 

int main()
{
  srand(time(NULL)); //set time for randomiser
  
  //create the player
  player = new Warrior();
  player->SetPos (1,1);
  
  InitNpcs(); //Fill NPC slots
  
  //GenerateItems(mediumLoot); 
 
  init_screen (); //prep display
 
  InitWindows(); //generate the UI windows
  DecorateWindows(); //Box/label the UI
  
  player->AddToInventory(new outfit(outfit_library[goblin]));
  container* c = new container(1,"Bag","X",2,0,true);
  c->AddItem(new weapon(weapon_library[sword]));
  player->AddToInventory(c);

  //container* thisContainer = new container(1,"Dead Body","X",1,0,true);
  //thisContainer->ReplaceItem(1,1,new item(item_library[1]));
  //SetContainer(4,1,thisContainer);

  //container* mapContainer = GetContainer(4,1);
  //mapContainer->AddItem(new item(item_library[2]));  
 
  running = (true);
 
  while ((running==true)) //Main game loop
    {
      GameLoop();
    };
  
  //End-game cleanup
  DestroyWindows();  
  endwin ();
  
  return (0);
};

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
#include "items.h"
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
  //  DrawAreas();
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

  InitAreas();
  //GenerateItems(mediumLoot); 
 
  init_screen (); //prep display
 
  InitWindows(); //generate the UI windows
  DecorateWindows(); //Box/label the UI
  
  player->AddToInventory(new outfit(outfit_library[goblin]));
  container* c = new container(98,"Bag","X",2,0,true);
  c->AddItem(new weapon(weapon_library[sword]));
  player->AddToInventory(c);

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

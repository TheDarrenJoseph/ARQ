                  
//////////////////////The ASCII Roguelike Quester -- Experimental project/////////////////
/////////////////////////////////LINUX BUILD//////////////////////////////////////////////
//                                                                                      //
//  Author: Rave Kutsuu                     Version : 0.83 -- New Item System           //
//  Created: Dec 16, 2012                   Last Modified: 15 June, 2014                //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREDITS                                                                                                                //
// 1. The Beginner's Guide to Roguelike Development in C/C++ -- http://www.kathekonta.com/rlguide/index.html (28/08/2013) //
// 3. NCURSES Programming HOWTO --  http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/                                  // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include <curses.h>

//---------grid sizes---------//
const int grid_x = 30;
const int grid_y = 15;
const int inv_x = 3;
const int inv_y = 5;

const int max_npcs = 5;
const int maxs = 1;

using namespace std;

//--TODO--
//1. Fighting (Finish Player/NPC conflicts, utilize character weapons)
//2. Health System (Doors have health, etc)
//3. NPCs (fighting(including environment),bosses)
//4. Items (lootable corpses, weapons, potions, scrolls, etc)
////////////

//--Final TODO--//
//1. Load game/item maps from files
//2. Level Progression
//3. Full ending

///////////////////////////////////////////////////////
//4. Extras (Windows, Fog of War, Menu, Sound? (Ambient music)
////////////

//--------running toggle---------//
bool running;
bool selection;

//--------Other--------//
string version = "0.82 Win32";

//------------ Cell Type/Game Guide ------------//
//                                              
//--------Environment/Gamespace---------//      
/////////////////////////////////////////////   
//these cell types constitute the main     //  
//gamespace of the dungeon and provide the //   
//player with a reactive environment       //
/////////////////////////////////////////////
//
// non - nothing
// cor - corridor
// rom - room
// wa1 - wall type 1
// wa2 - wall type 2
// wa3 - wall type 3
// win - window 
// ded - deadly cell
//
//---------doors---------//
/////////////////////////////////////////////////
//These cell types provide the player          //  
//with environmental barriers that can be      //   
//interacted with, these provide a barrier to  //
//game progress in some cases, and need to be  //
//"toggled" to another doortype to pass via    //
//player interaction (items may be needed such //
//as lockpicks or keys to pass some doors)     //
/////////////////////////////////////////////////
//
// od0 - open door type 0 -main office door, no lock-
// cd0 - closed door type 0
//
// od1 - open door type 1 -main lockable door, has 1 lock-
// cd1 - closed door type 1
// ld1 - locked door type 1
//
// od2 - open door type 2 -secondary lockable door, has 2 tile_locks-
// cd2 - closed door type 2
// ld2 - locked door type 2 
//
// ent - level entrance 
// ext - level exit
//
//---------Items(placeable)---------//
/////////////////////////////////////////////
//Placeable items are specific celltypes   //
//that create placeable loot in the dungeon//
//that can either help the player progress //
//or increase their score.                 //
/////////////////////////////////////////////
//
// key - a key, can be used to open any "ld"
// lop - lockpick, can be used to open any "lc"
//

//tile types
enum tile {ntl,cor,rom,wa1,wa2,wa3,win,od0,cd0,od1,cd1,ld1,od2,cd2,ld2,ent,ext,ded};

enum itemIndex {nit,lop,key,sta,gcn,scn,bcn,gbr,sbr,bbr};

enum weaponIndex {none,fist,claw,acid,sword,axe,dagger};

struct tile_details 
 {
  int id;
  bool destroyable;
  const char* symbol;
  int color;
  string name;
  int locks;
  int health;
 };
 
struct item
 {
  int id;
  string name;
  const char* symbol;
  int colour;
  int value;
  
  item()
   {
    id = 0;
    name = "None";
    symbol = " ";
    colour = 0;
    value = 0;
   }
   
  item(int i, string n, const char* s, int col, int val)
   {
    id = i;
    name = n;
    symbol = s;
    colour = col;
    value = val;
   }
       
 };
 
struct weapon : public item
 {
  int damage;
  
  weapon() : item(0,"None"," ",0,0)
   {
    damage=0;
   }
   
  weapon(int i, string n, const char* s, int col, int val,int d) : item(i,n,s,col,val)
   {
    damage = d;
   }
 };

tile_details tile_library[] =
{
 /*Index*/ /*Destroyable*/ /*Symbol*/ /*Colour*/ /*Name*/             /*Locks*/
 {0,       0,              " ",       0,         "Empty     ",        0,            /*Hitpoints*/ 0}, //non
 {1,       0,              "-",       4,         "Corridor",          0,            /*Hitpoints*/ 0 }, //cor
 {2,       0,              "-",       4,         "Room",              0,            /*Hitpoints*/ 0 }, //rom
 {3,       0,              "#",       3,         "Concrete Wall",     0,            /*Hitpoints*/ 0 }, //wa1
 {4,       0,              "#",       3,         "Stone Wall",        0,            /*Hitpoints*/ 0 }, //wa2
 {5,       0,              "#",       3,         "Brick Wall",        0,            /*Hitpoints*/ 0 }, //wa3
 {6,       0,              "%",       6,         "Window",            0,            /*Hitpoints*/ 0},
 {7,       0,              "-",       0,         "Open Office Door",  0,            /*Hitpoints*/ 100},
 {8,       1,              "=",       2,         "Closed Office Door",0,            /*Hitpoints*/ 100},
 {9,       0,              "-",       0,         "Open Metal Door",   1,            /*Hitpoints*/ 150},
 {10,      1,              "=",       2,         "Closed Iron Door",  1,            /*Hitpoints*/ 150 },
 {11,      1,              "=",       1,         "Locked Iron Door",  1,            /*Hitpoints*/ 150 },
 {12,      0,              "-",       0,         "Open Steel Door",   2,            /*Hitpoints*/ 200 },
 {13,      1,              "=",       2,         "Closed Steel Door", 2,            /*Hitpoints*/ 200 },
 {14,      1,              "=",       1,         "Locked Steel Door", 2,            /*Hitpoints*/ 200},
 {15,      0,              "@",       1,         "Entrance",          0,            /*Hitpoints*/ 0},
 {16,      0,  	       	   "@",       2,         "Exit",              0,            /*Hitpoints*/ 0 },
 {17,      0,		   	   "!",       7,         "Deadly Floor",      0,            /*Hitpoints*/ 0 },
};

item item_library[] = 
{ 
 {0,"None      "," ",0,0},
 {1,"Lockpick ","!",2,1}, 
 {2,"Door Key ","!",2,5}, 
 {3,"Statue   ","$",2,500},
 {4,"Gld Coin ","$",2,50},
 {5,"Sil Coin ","$",2,25},
 {6,"BrnzCoin ","$",2,5},
 {7,"GoldBar  ","$",2,100},
 {8,"SilvrBar ","$",2,50},
 {9,"BrnzeBar ","$",2,25},
};

weapon weapon_library[] = 
{
 {0,"None"," ",0,0,10},
 {1,"Claw","W",0,0,10},
 {2,"Acid","W",0,0,10},
 {3,"Sword","W",0,0,10},
 {4,"Axe","W",0,0,10},
 {5,"Dagger","W",0,0,10},
};

class Character
 {
  protected:
  int x;
  int y;
  
  char symbol;
  int colour;
  string name;
  int health;

  bool alive;

  weapon weps [3];

  public:
  
  bool IsAlive();

  void kill();

  void set(char char_choice, int colour_choice, string name_choice, int health_choice);

  void getPos (int *x, int *y);

  string getName();
  int getHealth ();

  void setWeps(weaponIndex one, weaponIndex two, weaponIndex three);
  weapon* getWeps();
  void setPos(int x, int y);
  void setHealth(int);
  void draw(WINDOW* winchoice);
   
  virtual int move() =0; /*Do nothing by default, abstract*/
     
  Character (char c, int col, string name, int health) 
   {
   	set(c,col,name,health);
   };
  
 };

class Player : public Character
{
 private:
 
 int loot;
  
 public :
 
 int drawInv (WINDOW* winchoice);
 
 
 int move ();
 int move (WINDOW * winchoice, WINDOW* mainwin, int y, int x);

 int Inventory (WINDOW* input_win, WINDOW* inv_win);

 int invproc (tile itm);
 int itmproc (WINDOW* winchoice, tile itm, int y, int x);
 int lockproc (WINDOW * winchoice, int door_y, int door_x, tile doortype, int doortile, string doorname );
 int doorproc (WINDOW * winchoice, int y, int x, tile doortype);
 
 int battleTurn (int e_id);
 
 void setLoot(int x);
 int getLoot();
 
 int input (WINDOW* winchoice, WINDOW* inv_win, WINDOW* main_win);
 int stats(WINDOW* winchoice);

 int lootcount ();
 
 Player (char c, int col, string name, int health) : Character(c,col,name,health)
  {
  };
};

class Warrior : public Player
 {
  public:

  Warrior () : Player('@',6,"Warrior",100)
   {
    setWeps(none,fist,sword); //Setweps segfault
   };
  
 };
 
class NPC : public Character
{
 public :
 int battleTurn ();
 int move();

 NPC (char c, int col, string name, int health) : Character(c,col,name,health)
  {
   this->setPos(6,6);
  };
};


class Goblin : public NPC
{
 public:
 Goblin() : NPC ('@',2,"Wild Goblin",30)
  {
   setWeps (none,claw,sword);
  };
  
 private:
};

class Slime : public NPC
{
 public:
 Slime () : NPC ('*',1,"Oozy Slime",10)
  {
   setWeps (none,acid,claw);
  };

};

//main map
tile game_grid[grid_y][grid_x] = 
{ 
    {wa1,wa1,ent,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1},
    {wa1,cor,cor,cor,cor,cor,cor,cor,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,cd0,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,ded,wa2,wa2,cd0,cd0,wa2,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,wa2,wa2,wa2,wa2,wa2,cor,wa2,wa2,wa2,cd0,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,cd0,wa2,wa2,wa2,wa2,wa1},
    {wa1,cor,cor,cor,cor,cor,cor,cor,cor,wa2,cor,cor,cor,ded,ded,ded,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,wa1},
    {wa1,wa2,wa2,wa2,wa2,wa2,wa2,wa2,cd1,wa2,cor,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,cd0,wa2,wa2,wa2,cd0,wa2,wa2,wa2,wa1},
    {wa1,rom,ded,rom,rom,rom,ded,wa2,cor,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,rom,ded,rom,rom,rom,ded,wa2,cor,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,ded,ded,ded,rom,ded,ded,wa2,cor,ld1,cor,wa2,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,ded,ded,rom,rom,rom,rom,rom,cor,ld1,cor,cd0,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,ext,wa1,wa1,wa1},
};

item * item_grid[grid_y][grid_x];

tile inventory[inv_y][inv_x];

Player * player;
NPC * npcs[maxs];


int wprintw_col (WINDOW* winchoice, const char *text, int color_choice) //allows ncurses window and colour selection when outputting characters
 {
  wattron(winchoice,COLOR_PAIR(color_choice)); //turns on the chosen colour pair
  wprintw (winchoice, text); 
  wattroff(winchoice,COLOR_PAIR(color_choice)); //turns off the chosen colour pair
  return (0);
 };             

int wprint_at (WINDOW* winchoice, const char *text, int pos_y, int pos_x)
 {
  wmove (winchoice,pos_y,pos_x);
  wprintw (winchoice, text); 
  wrefresh (winchoice);
  return (0);
 }  
//////////////////Global Game Functions/////////////////////

void battle (WINDOW* main_win, WINDOW* input_win, int npc_id)
{
  int p_health = player->getHealth();
  int npc_health = npcs[npc_id]->getHealth();
 
  string p_name = player->getName();
  string npc_name = npcs[npc_id]->getName();
  
  werase (input_win); //Clears both the input and map windows to fit the battle scenario
  werase (main_win);
  
  //display the info of both combatants
  wmove (input_win,0,0);
  wprintw (input_win,"%s Health: %d \n",p_name.c_str(),p_health);
  wprintw (input_win,"%s Health: %d \n",npc_name.c_str(),npc_health);
  
  
  //allow both combatants to make a move
 // int p_move = player->battleTurn(npc_id); 
 // int npc_move = npcs[npc_id]->battleTurn();
  
 // weapon* p_weps = player->getWeps();
 // weapon* npc_weps = npcs[npc_id]->getWeps();
  
  string out;
  
  for (int i = 0; i<2; i++)
   {
    //weapon wep = p_weps[i];
    //out += wep.name;
   }
    
  wprintw (input_win, out.c_str());
  
  wgetch (input_win);
  
  //check the move choice against weapons 0-2, along with special actions
  //use the int to find the item in weps, if > weps, special action.
  
 
}

///////////////////////////////////////
bool Character :: IsAlive ()
 {
  if (health<=0)
   {
    this->kill();
    return false;
   }
  
  else
   {
    return true;
   }
  
 };



void Character :: set (char char_choice, int colour_choice, string name_choice, int health_choice)
 {
  alive = true;
  this->symbol = char_choice;
  this->colour = colour_choice;
  this->name = name_choice;
  this->health = health_choice;
  
  void set_pos(int x, int y);
 };

void Character :: kill()
 {
  alive = false;
 };

string Character :: getName()
 {
  return this->name;
 } 

void Character :: getPos (int *x, int *y)
 {
  (*x = this->x);
  (*y = this->y);
 };

int Character :: getHealth ()
 {
  return this->health;
 };
  
void Character :: setHealth(int i)
 {
  this->health = i;
  return;
 }

void Character :: setPos(int x, int y)
 {
  if ((x < 0) || (x > grid_x))
   {
    return;
   }
  
  else if ((y < 0) || (y > grid_y))
   {
    return;
   }
  
  else
   {
    this->x = x;
    this->y = y;
   };
  return;
 };
 
void Character :: draw (WINDOW* winchoice)
 {
  if ((this->x < 0) || (this->x > grid_x))
   {
    return;
   }
  
  else if ((this->y < 0) || (this->y > grid_y))
   {
    return;
   }
   
  else
   {
    wmove (winchoice,this->y,this->x);
  
    wattron(winchoice,COLOR_PAIR(this->colour)); //turns on the chosen colour pair
    wprintw (winchoice, "%c", this->symbol);
    wattroff(winchoice,COLOR_PAIR(this->colour));
   };
   
  return;
 };

////////////////////////////////////////////////

int Player :: Inventory (WINDOW* input_win, WINDOW* inv_win)
 {
  tile inv_tile;
  int invtile_colour;
  int loc_x,loc_y,scr_x,scr_y;
  string str;
  string slc_string;
  char slc_char[20];
  
  loc_x = 0;
  loc_y = 0;
  scr_x = 9;
  scr_y = 0;
  
  //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
  selection = true;

  while (selection == true) 
   {
    mvwchgat (inv_win, scr_y, scr_x, 9, A_NORMAL, 0, NULL);
    wrefresh (inv_win);
    
    scr_x = loc_x*9;
    scr_y = loc_y;
    
    mvwchgat (inv_win, scr_y, scr_x, 9, A_BLINK, 1, NULL);
    wrefresh (inv_win);
    
    werase (input_win);
    wprint_at (input_win, "Select item to drop with WASD.. 'Drop' to drop the selected item, 'Exit' to cancel", 0,0);
    
    wprint_at (input_win,"ARQ:~$ ",2,0);
    
    wgetstr (input_win, slc_char);
    slc_string = slc_char;
    
    if ((slc_string == "W") || (slc_string == "w")) 
     {
      if (loc_y == 0)
       {
        loc_x = loc_x;
        loc_y = loc_y;
       }
      
      else
       {
        loc_x = loc_x;
        loc_y = loc_y-1;
       };
     }
  
    else if ((slc_string == "A") || (slc_string == "a")) 
     {
      if (loc_x == 0)
       {
        loc_x = loc_x;
        loc_y = loc_y;
       }
      
      else
       {
        loc_x = loc_x-1;
        loc_y = loc_y;
       };
     }
     
    else if ((slc_string == "S") || (slc_string == "s")) 
     {
      if (loc_y == 4)
       {
        loc_x = loc_x;
        loc_y = loc_y;
       }
      
      else
       {
        loc_x = loc_x;
        loc_y = loc_y+1;
       };
     }
  
    else if ((slc_string == "D") || (slc_string == "d")) 
     {
      if (loc_x == 2)
       {
        loc_x = loc_x;
        loc_y = loc_y;
       }
      
      else
       {
        loc_x = loc_x+1;
        loc_y = loc_y;
       };
     }
    
    else if ((slc_string == "Exit") || (slc_string == "exit") || (slc_string == "EXIT")) 
     {
      selection = false;
      return (0);
     }
     
    else if ((slc_string == "Drop") || (slc_string == "drop") || (slc_string == "DROP")) 
     {
       if ((game_grid[this->y][this->x] == ntl) || (game_grid[this->y][this->x] == rom) || (game_grid[this->y][this->x] == cor))
        {
		     
         if (inventory[loc_y][loc_x] != ntl)
           {
             inv_tile = inventory[loc_y][loc_x]; //grab the current inventory tile
     
             game_grid[this->y][this->x] = inv_tile; //replace the map tile with the item
             
             inventory[loc_y][loc_x] = ntl; //clear the inventory tile
                    
             invtile_colour = tile_library[inv_tile].color;
             str = tile_library[inv_tile].name;
           
             const char *invtile_char = str.c_str();
        
             wmove (inv_win,scr_y,scr_x);
             wprintw_col (inv_win, invtile_char, invtile_colour);
             wrefresh (inv_win);
             werase (input_win);
             wprintw (input_win, "Item dropped.");
             wgetch(input_win);
            }
            
           else
            {
		     werase (input_win);
             wprintw (input_win, "No item selected.");
             wgetch(input_win);
	        };
         }
         
         else
          {
		    werase (input_win);
            wprintw (input_win, "There's already an item here.");
            wgetch(input_win);
	       };
     }
     
    else if ((slc_string == "Use") || (slc_string == "use") || (slc_string == "USE")) 
     {
      if (inventory[loc_y][loc_x] != ntl)
       {
        inv_tile = inventory[loc_y][loc_x]; //grab the current inventory tile
                    
        invtile_colour = tile_library[inv_tile].color;
        str = tile_library[inv_tile].name;
           
        const char *invtile_char = str.c_str();
        
        wmove (inv_win,scr_y,scr_x);
        wprintw_col (inv_win, invtile_char, invtile_colour);
        wrefresh (inv_win);
        werase (input_win);
        
        wprintw (input_win, "%s selected.",str.c_str());
        wgetch(input_win);
       }
            
      else
       {
        werase (input_win);
        wprintw (input_win, "No item selected.");
        wgetch(input_win);
       };
        
     }
    
    else 
     {
      werase (input_win);
      wprint_at (input_win, "Not a correct selection, try again.", 0, 0);
      wgetch (input_win);
     };
    cout << "\n " << loc_x << " " << loc_y;
   };
  return (0);
 };
 
int Player :: invproc (tile itm) 
{
 for (int y = 0; y < inv_y; y++)
  {
   for(int x = 0; x < inv_x; x++)
    {
     if (inventory[y][x] == ntl) //If slot is empty
      { 
	   inventory[y][x] = itm; //Place the item in empty slot
      return (0);
      };
	}; 
  };
 return(1);	
};

int Player :: itmproc (WINDOW* winchoice, tile itm, int y, int x)
{
 string answer;
 char answerchar[20];
 string itm_name;
 itm = game_grid[y][x];
 itm_name = tile_library[itm].name; 
 
 werase (winchoice);
 wmove (winchoice,0,0);
 wprintw (winchoice,"There's a %s on the floor..", itm_name.c_str());
 wrefresh (winchoice);
 wgetch (winchoice); 
 wmove (winchoice,0,0);
 wprintw (winchoice,"Would you like to pick up the %s? ",itm_name.c_str());
 wgetstr (winchoice, answerchar);
 answer = (answerchar);
      
  if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
   {
    if (invproc (itm) == (1)) //If invproc unsuccessful
     {
      werase (winchoice);
      wprint_at (winchoice,(const char *)"Inventory full..",0,0);
      wgetch (winchoice);
      return (1);
     }
    
    else
     {
      wmove (winchoice,0,0);
      werase (winchoice);
      wprintw (winchoice,"You pick up the %s..",itm_name.c_str());
      wgetch (winchoice);
      game_grid[y][x] = rom; //Removes the item from the map 

      setPos(x,y);

      return (0);
     };
   }
   
   else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
    {
     wmove (winchoice,0,0);
     werase (winchoice);
     wprintw (winchoice,"You leave the %s untouched..",itm_name.c_str());
     wgetch (winchoice);
     return (0);
    }
    
   else 
    {
     wprint_at (winchoice, "Incorrect choice, please answer yes or no.. ",0,0);
     wgetch (winchoice);
     itmproc (winchoice,itm,y,x);
    };
    
 return (0);
};

int Player :: drawInv (WINDOW* winchoice)
 {
  wmove (winchoice,0,0);
 	for (int y = 0; y < inv_y; y++)
	 {
	  wmove (winchoice,y,0);
	  for(int x = 0; x < inv_x; x++)
		{
     int inv_tile;
     int invtile_colour;
     string str;
         
     inv_tile = inventory[y][x];
     invtile_colour = tile_library[inv_tile].color;
     str = tile_library[inv_tile].name;
         
     const char *invtile_char = str.c_str();
        
     wprintw_col (winchoice, invtile_char, invtile_colour);
		}; 
	  };  
 return (0);
};

void Character :: setWeps (weaponIndex one, weaponIndex two, weaponIndex three)
 {
  this->weps[0] = weapon_library[one];
  this->weps[1] = weapon_library[two];
  this->weps[2] = weapon_library[three];
 };
 
weapon* Character :: getWeps()
 {
  return this->weps;
 }
 
int Player :: stats (WINDOW* winchoice)
{
 wmove (winchoice,0,1);
 wattron(winchoice,A_UNDERLINE); //turns on the chosen colour pair
 wprintw (winchoice,"%s",this->getName().c_str());
 wattroff(winchoice,A_UNDERLINE); //turns off the chosen colour pair
 
 wmove (winchoice,1,0);
 wprintw_col (winchoice,"Health: ",4); 
 wprintw (winchoice,"%d",this->getHealth());
 
 wmove (winchoice,2,0);
 wprintw_col (winchoice,"Loot:   ",4); 
 wprintw (winchoice,"%d",this->getLoot());
 
 for( int a = 0; a < max_npcs; a++ )
     {
      wmove (winchoice,a+3,0);
      //LIST HEALTH OF NPCS?
     };
 return (0);
}

int Player :: lockproc (WINDOW * winchoice, int door_y, int door_x, tile doortype, int doortile, string doorname )
{
 string answer;
 char answerchar[20];
 werase (winchoice);
 wprint_at (winchoice,"How would you like to unlock the door? ",0,0);
 wprint_at (winchoice,"1. Using Door Keys, 2. Using Lockpicks ",1,0);
 wprint_at (winchoice,"Enter a choice to continue, or 'exit' to cancel ",2,0);
 wprint_at (winchoice,"lock_proc:~$  ",3,0);
 wgetstr (winchoice, answerchar);
 answer = (answerchar);
      
   if ((answer == "1") || (answer == "Key") || (answer == "Keys") || (answer == "key") || (answer == "keys") || (answer == "Door Key") || (answer == "Door Keys") || (answer == "door key") || (answer == "door keys"))
    {
     int inv_tile;
     int key_count;
     int count;
     count = 0;
     key_count = 0;
      
      for (int y = 0; y < inv_y; y++)
       {
        for(int x = 0; x < inv_x; x++)
         {
          inv_tile = inventory[y][x];
          
          if (inv_tile == (key))
           {
            key_count = (key_count+1);
           };
          
         };
       }; 
     
     if (key_count == (tile_library[doortile].locks))
      {
            
       for (int y = 0; y < inv_y; y++)
        {
         for(int x = 0; x < inv_x; x++)
          {
           inv_tile = inventory[y][x];
           
           if (inv_tile == (key))
            {
             inventory[y][x] = ntl;
             count = (count+1);
            };
                 
           if (count == (tile_library[doortile].locks))
            {
             werase (winchoice);
             wmove (winchoice,0,0);
             wprintw (winchoice,(const char *)"You insert %d keys into the door and open it .. ",tile_library[doortile].locks);
             wgetch (winchoice);
       
             if (doortype == ld1) 
              {
               game_grid[door_y][door_x]=(od1);
               
               if (game_grid[door_y+1][door_x] == (ld1)) //Checking for surrounding door tiles
                {
                 game_grid[door_y+1][door_x]=(od0);
                }
          
               else if (game_grid[door_y-1][door_x] == (ld1)) 
                {
                 game_grid[door_y-1][door_x]=(od0);
                }
          
               else if (game_grid[door_y][door_x+1] == (ld1)) 
                {
                 game_grid[door_y][door_x+1]=(od0);
                }
            
               else if (game_grid[door_y][door_x-1] == (ld1)) 
                {
                 game_grid[door_y][door_x-1]=(od0);
                };
              }
                  
             else if (doortype == ld2)
              {
               game_grid[door_y][door_x]=(od2);
               
               if (game_grid[door_y+1][door_x] == (ld2)) //Checking for surrounding door tiles
                {
                 game_grid[door_y+1][door_x]=(od2);
                }
          
               else if (game_grid[door_y-1][door_x] == (ld2)) 
                {
                 game_grid[door_y-1][door_x]=(od2);
                }
          
               else if (game_grid[door_y][door_x+1] == (ld2)) 
                {
                 game_grid[door_y][door_x+1]=(od2);
                }
            
               else if (game_grid[door_y][door_x-1] == (ld2)) 
                {
                 game_grid[door_y][door_x-1]=(od2);
                };
              };
             
             this->setPos(door_x,door_y);               
             return (0);
            };
            
          };
        }; 
             
      };
           
     if (key_count != (tile_library[doortile].locks))
      {
       werase (winchoice);
       wmove (winchoice,0,0);
       wprintw (winchoice,"You need %d keys to open this door.. ",tile_library[doortile].locks);
       wgetch (winchoice);
       return (0);
      };
      
     return (0);
    }
    
   else if ((answer == "2") || (answer == "Lockpick") || (answer == "Lockpicks") || (answer == "lockpick") || (answer == "lockpicks") || (answer == "Lock Pick") || (answer == "Lock Picks") || (answer == "lock pick") || (answer == "lock picks"))
    {
     int inv_tile;
     int lockpick_count;
     int count;
     count = 0;
     lockpick_count = 0;
      
      for (int y = 0; y < inv_y; y++)
       {
        for(int x = 0; x < inv_x; x++)
         {
          inv_tile = inventory[y][x];
          
          if (inv_tile == (lop))
           {
            (lockpick_count = (lockpick_count+1));
           };
          
         };
       };
       
      if (lockpick_count >= (tile_library[doortile].locks))
       {
         
        for (int y = 0; y < inv_y; y++)
         {
          for(int x = 0; x < inv_x; x++)
           {
            inv_tile = inventory[y][x];
            
            if (inv_tile == (lop))
             {
              int chance = rand() %100+1;
              int lockno;
              
              lockno = 1;
              werase (winchoice);
              wmove (winchoice,0,0);
              wprintw (winchoice,"This door has %d lock(s).. ",tile_library[doortile].locks);
              wmove (winchoice,1,0);
              wprintw (winchoice,"You attempt to pick lock %d.. ", lockno);
              wgetch (winchoice);
              
              if (chance > 50)
               {
                werase (winchoice);
                wprint_at (winchoice,"You manage to open the lock with the lockpick.. ",0,0);
                wgetch (winchoice);
                 
                inventory[y][x] = ntl;
                count = (count+1);
                chance = rand() %100+1;
                lockno = (lockno+1);
               }
              
              else if (chance <= 50)
               {
                werase (winchoice);
                wprint_at (winchoice,"Your lockpick breaks as you attempt to open the lock.. ",0,0);
                wgetch (winchoice);
                    
                inventory[y][x] = ntl;
                chance = rand() %100+1;
               };
                  
             };
                 
            if (count == (tile_library[doortile].locks))
             {
              werase (winchoice);
              wprint_at (winchoice, "You manage to unlock the door.. ",0,0);
              wgetch (winchoice);
                  
              if (doortype == ld1) 
               {
                game_grid[door_y][door_x]=(od1);
               }
                  
              else if (doortype == ld2)
               {
                game_grid[door_y][door_x]=(od2);
               };
                                    
               this->setPos(door_x,door_y);  
               return (0);
             };
                 
           };
         }; 
             
       } 
     
     else if (lockpick_count != (tile_library[doortile].locks))
      {
       werase (winchoice);
       wmove (winchoice,0,0);
       wprintw (winchoice,(const char *)"You need %d lock picks to attempt to open this door.. ",tile_library[doortile].locks);
       wgetch (winchoice);
       return (0);
      };
     
     if (count != (tile_library[doortile].locks))
      {
       werase (winchoice);
       wprint_at (winchoice, "You have run out of lockpicks..",0,0);
       wgetch (winchoice);
       return (0);
      };
     
     return (0);
    }
            
   else if ((answer == "Exit") || (answer == "EXIT") || (answer == "exit"))
    {
     werase (winchoice);
     wmove (winchoice,0,0);
     wprintw (winchoice,(const char *)"You leave the %s untouched. ",doorname.c_str());
     wgetch (winchoice);
     return (0);
    }
	    
   else
    {
     wprint_at (winchoice,(const char *)"Not a correct choice, try again.. ",0,0);
     this->lockproc (winchoice,door_y,door_x,doortype,doortile,doorname);
    };
 return (0);
};

int Player :: battleTurn (int e_id)
 {
  //PACEHOLDER
  return 0;
 }

int Player :: doorproc (WINDOW * winchoice, int y, int x, tile doortype)
{
 int map_tile = game_grid[y][x];
 string door_name = tile_library[map_tile].name;
 
    if ((doortype == od0) || (doortype == od1) || (doortype == od2))
     {
        wmove (winchoice,0,0);
        wprintw (winchoice,"You enter the doorway of the %s.. ",door_name.c_str());
        wgetch (winchoice);
        this->setPos(x,y);  
   
        return (0);
     }
     
    else if ((doortype == cd0) || (doortype == cd1) || (doortype == cd2))
     {
	   string answer;
      char answerchar[20];
      wmove (winchoice,0,0);
      wprintw (winchoice,"Would you like to open the %s? ",door_name.c_str());
      wgetstr (winchoice, answerchar);
      answer = (answerchar);
      
       if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
        {
         wmove (winchoice,0,0);
         wprintw (winchoice,"You open the %s and step into the doorway. ",door_name.c_str());
         wgetch (winchoice);
         
         if (game_grid[y][x] == (cd0)) //Checking main tile
          {
           game_grid[y][x]=(od0);
           
           if (game_grid[y+1][x] == (cd0)) //Checking for surrounding door tiles
            {
             game_grid[y+1][x]=(od0);
            }
          
           else if (game_grid[y-1][x] == (cd0)) 
            {
             game_grid[y-1][x]=(od0);
            }
          
            else if (game_grid[y][x+1] == (cd0)) 
            {
             game_grid[y][x+1]=(od0);
            }
            
            else if (game_grid[y][x-1] == (cd0)) 
            {
             game_grid[y][x-1]=(od0);
            };
          };
         
         if (game_grid[y][x] == (cd1))
          {
           game_grid[y][x]=(od1);
           
           if (game_grid[y+1][x] == (cd1)) //Checking for surrounding door tiles
            {
             game_grid[y+1][x]=(od1);
            }
          
           else if (game_grid[y-1][x] == (cd1)) 
            {
             game_grid[y-1][x]=(od1);
            }
          
            else if (game_grid[y][x+1] == (cd1)) 
            {
             game_grid[y][x+1]=(od1);
            }
            
            else if (game_grid[y][x-1] == (cd1)) 
            {
             game_grid[y][x-1]=(od1);
            };
          };
         
         if (game_grid[y][x] == (cd2))
          {
           game_grid[y][x]=(od2);
           
           if (game_grid[y+1][x] == (cd2)) //Checking for surrounding door tiles
            {
             game_grid[y+1][x]=(od2);
            }
          
           else if (game_grid[y-1][x] == (cd2)) 
            {
             game_grid[y-1][x]=(od2);
            }
          
            else if (game_grid[y][x+1] == (cd2)) 
            {
             game_grid[y][x+1]=(od2);
            }
            
            else if (game_grid[y][x-1] == (cd2)) 
            {
             game_grid[y][x-1]=(od2);
            };
          };
          
         this->setPos(x,y);  
         return (0);
	    }
	    
	   else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
	    {
       werase (winchoice);
	     wmove (winchoice,0,0);
       wprintw (winchoice,"You leave the %s untouched. ",door_name.c_str());
       wgetch (winchoice);
       return (0);
	    }
	   
	   else
        {
         werase (winchoice);
         wprint_at (winchoice,"Not a yes or no answer, try again..",0,0);
         wgetch(winchoice);
         doorproc (winchoice, y, x, game_grid[y][x]);
        };
        
	   return (0);
      }
      
    else if ((doortype == ld1) || (doortype == ld2))
     {
	   string answer;
      char answerchar[20];
      werase (winchoice);
      wprint_at (winchoice,"Would you like to open the door? ",0,0);
      wgetstr (winchoice, answerchar);
      answer = (answerchar);
      
       if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
        {
         lockproc (winchoice, y,x,doortype,map_tile,door_name);
         return (0);
        }
   
            
       else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
        {
         wmove (winchoice,0,0);
         wprintw (winchoice,(const char *)"You leave the %s untouched. ",door_name.c_str());
         wgetch (winchoice);
         return (0);
        }
	    
       else
        {
         wprint_at (winchoice,(const char *)"Not a yes or no answer, try again..",0,0);
         doorproc (winchoice, y, x, game_grid[y][x]);
        };
      return (0);
     };

 return (0);
};

int Player :: move()
 {
  return 1;
 }

int Player :: move (WINDOW * winchoice, WINDOW* mainwin, int y, int x)
{
 string move_tilename = tile_library[game_grid[y][x]].name;
 int pmove_x = (x);
 int pmove_y = (y);
 
 if ((x < 0) || (x > grid_x))
  {
   return (1);
  }
  
 else if ((y < 0) || (y > grid_y))
  {
   return (1);
  };
 
 //This checks to see if the player has walked into any enemies/characters
 for (int e_id = 0; e_id < max_npcs; e_id++)
  {
   int enemy_x;
   int enemy_y;
   npcs[e_id]->getPos (&enemy_x, &enemy_y); //Sets enemy x and y to those of npc[No] by passing their memory adresses to the function (which in turn sets them to the protected values)
  
      int x;
      int y;
      player->getPos (&x, &y); 
   
       if (x == enemy_x and y == enemy_y)
        {
         string enemy_name = npcs[e_id]->getName();
         
			wmove (winchoice,0,0);
			wprintw (winchoice,"You are confronted by a %s! ",enemy_name.c_str());
		
         
         battle (mainwin, winchoice, e_id);
     
         wgetch (winchoice);
         return (0);
        };
    
  };
 
 /* Change to scan item array instead
 if ((game_grid[y][x] == lop) || (game_grid[y][x] == key) || (game_grid[y][x] == sta) || (game_grid[y][x] == gcn) || (game_grid[y][x] == scn) || (game_grid[y][x] == bcn) || (game_grid[y][x] == gbr) || (game_grid[y][x] == sbr) || (game_grid[y][x] == bbr) || (game_grid[y][x] == swd))
  {
   tile itm = game_grid[y][x];
   itmproc (winchoice, itm, y, x);
   return (0);
  };
 */
 
 if (game_grid[y][x] == ntl)
  {
   return (1);
  }
    
 else if ((game_grid[y][x] == cor) || (game_grid[y][x] == rom))
  {
   wmove (winchoice,0,0);
   wprintw (winchoice,"You walk along the %s.. ",move_tilename.c_str());
   setPos(pmove_x,pmove_y);  
   wgetch (winchoice);
   return (0);  
  }
  
 else if ((game_grid[y][x] == wa1) || (game_grid[y][x] == wa2) || (game_grid[y][x] == wa3))
  {
   wmove (winchoice,0,0);
   wprintw (winchoice,"There's a %s, you cannot pass.",move_tilename.c_str());
   wgetch (winchoice);
   return (0);
  }
 
 else if (game_grid[y][x] == win) 
  {
   wprint_at (winchoice,(const char *)"There's a window here..",0,0);
   wgetch (winchoice);
   return (0);
  }
 
 else if (game_grid[y][x] == od0 || (game_grid[y][x] == od1) || (game_grid[y][x] == od2)) 
  {
   this->doorproc (winchoice, y, x, game_grid[y][x]);
   return (0);
  }
  
 else if (game_grid[y][x] == cd0 || (game_grid[y][x] == cd1) || (game_grid[y][x] == cd2) || (game_grid[y][x] == ld1) || (game_grid[y][x] == ld2)) 
  {
   wmove (winchoice,0,0);
   wprintw (winchoice,"There's a %s here..",move_tilename.c_str());
   wgetch (winchoice);
   this->doorproc (winchoice, y, x, game_grid[y][x]);
   return (0);
  }
  
 else if (game_grid[y][x] == ent) 
  {
   wprint_at (winchoice,(const char *)"The way you came in is locked..",0,0);
   wgetch (winchoice);
   return (0);
  }
 
 else if (game_grid[y][x] == ext) 
  {
   wprint_at (winchoice,(const char *)"You have reached the exit!",0,0);
   wgetch (winchoice);
   return (0);
  }
 
 else if (game_grid[y][x] == ded) 
  {
   wprint_at (winchoice,(const char *)"The floor caves in below one of your feet, injuring you..",0,0);
   setHealth(this->getHealth()-20);
   wgetch (winchoice);
   return (0);
  };

return (0);
};

void Player :: setLoot(int i)
 {
  this->loot = i;
  return;
 }

int Player :: getLoot ()
 {
  return this->loot;
 };

int Player :: input (WINDOW* winchoice, WINDOW* inv_win, WINDOW* main_win)
{
 int x;
 int y;
   
 this->getPos(&x,&y); /*Get current position for movement*/

 string answer;
 char answerchar[20];

 werase (winchoice);

 wmove (winchoice,0,0); //moves the cursor to the window choice inputted into the function, +1 either side to avoid borders
 wprintw (winchoice,"ARQ:~$ "); 
 wgetstr (winchoice, answerchar);
 answer = answerchar;
 
 if (answer == "pos") 
  {
   cout << "player y :" << x;
   cout << "\nplayer x :" << y;
   return (0);
  }
  
 if (answer == "help") 
  {
   wprint_at (winchoice,(const char *)"phelp - player help",0,0);
   wprint_at (winchoice,(const char *)"ihelp - interactions",1,0);
   wprint_at (winchoice,(const char *)"info - game info",2,0);
   wgetch (winchoice);
   return (0);
  }
 
 if (answer == "phelp") 
  {
   wprint_at (winchoice,(const char *)"north - move north",0,0);
   wprint_at (winchoice,(const char *)"east- move east",1,0);
   wprint_at (winchoice,(const char *)"south - move south",2,0);
   wprint_at (winchoice,(const char *)"west - move west",3,0);
   wgetch (winchoice);
   return (0);
  }
  
  if (answer == "ihelp") 
  {
   wprint_at (winchoice,(const char *)"drop - drop item (selection)",0,0);
   wgetch (winchoice);
   return (0);
  }
  
  if (answer == "info") 
  {
	wprint_at (winchoice,(const char *)"Created by Rave Kutsuu",0,0); //Please leave this untouched as proof of origin
    wprint_at (winchoice,(const char *)version.c_str(),1,0); 
	wprint_at (winchoice,(const char *)"ARQ Learner project/Tech demo",2,0);
	wprint_at (winchoice,(const char *)"Made using C++ and ncurses",3,0);
	wgetch (winchoice);
   return (0);
  }
  
  if ((answer == "north") || (answer == "North") || (answer == "NORTH") || (answer == "n") || (answer == "N")) 
  {
   this->move (winchoice, main_win, y-1,x);
   return (0);
  }
  
  if ((answer == "east") || (answer == "East") || (answer == "EAST") || (answer == "e") || (answer == "E"))
  {
   this->move (winchoice, main_win, y, x+1);
   return (0);
  }
  
  if ((answer == "south") || (answer == "South") || (answer == "SOUTH") || (answer == "s") || (answer == "S")) 
  {
   this->move (winchoice, main_win, y+1, x);
   return (0);
  }
  
  if ((answer == "west") || (answer == "West") || (answer == "WEST") || (answer == "w") || (answer == "W"))
  {
   this->move (winchoice, main_win, y, x-1);
   return (0);
  }
  
  if ((answer == "exit") || (answer == "Exit") || (answer == "EXIT") || (answer == "quit") || (answer == "Quit") || (answer == "QUIT"))
  {
   wprint_at (winchoice,(const char *)"Quitting.. ",0,0);
   wgetch (winchoice);
   running = (false);
   return(0);
  }
  
  if ((answer == "Inventory") || (answer == "INVENTORY") || (answer == "inventory"))
  {
   Inventory (winchoice, inv_win);
   return(0);
  }
  
 else
  wprint_at (winchoice,(const char *)"unrecognised input, please input a command, or use 'help' for a list. ",0,0);
  wgetch (winchoice);
  input (winchoice, inv_win, main_win);
  return (0);

return (0);
};

int Player :: lootcount ()
{
 int total = 0;

 for (int y = 0; y < inv_y; y++)
  {
    for(int x = 0; x < inv_x; x++)
     {
      int itm =  inventory[y][x];
      int value = item_library[itm].value;
      total+=value;
     };
   };

 this->setLoot(total);
 return (0);
};
 
/////////////////////////////////////////////////

int NPC :: battleTurn ()
 {
  return (0);
 };

int NPC :: move ()
 {
  int x = ((rand() % 3 - 1));
  int y = ((rand() % 3 - 1));
  
  int pos_x;
  int pos_y;
    
  this->getPos(&pos_x,&pos_y);  

  int move_x = (pos_x+x);
  int move_y = (pos_y+y);
 
  if ((move_x == pos_x) and (move_y == pos_y))
   {
    return 1;
   }
  
  if ((move_x != pos_x) and (move_y != pos_y))
   {
    return 1;
   }
  
  for (int No = 0; No < maxs; No++)
  {
   for (int NPCNo = 0; NPCNo < max_npcs; NPCNo++)
    {
     int player_x;
     int player_y;
     int character_x;
     int character_y;
   
     npcs[No]->getPos (&player_x, &player_y);
     npcs[NPCNo]->getPos (&character_x, &character_y);
     
     
     if (move_x == player_x and move_y == player_y)
      {
       //Battle
      };
     
     if (move_x == character_x and move_y == character_y)
      {
       return 0;
      };
     
    };
  };
  
  if ((game_grid[move_y][move_x] != ntl) and (game_grid[move_y][move_x] != wa1) and (game_grid[move_y][move_x] != wa2) and (game_grid[move_y][move_x] != wa3) and (game_grid[move_y][move_x] != ent) and (game_grid[move_y][move_x] != ext))
   {
    this->setPos (move_x,move_y);
    return 0;  
   }
    
  else if (game_grid[move_y][move_x] == win) 
   {
    return 1;
   }
  
  else if (game_grid[move_y][move_x] == cd0 || (game_grid[move_y][move_x] == cd1) || (game_grid[move_y][move_x] == cd2) || (game_grid[move_y][move_x] == ld1) || (game_grid[move_y][move_x] == ld2)) 
   {
    return 2;
   }
   
  else if (game_grid[move_y][move_x] == ded) 
   {
    return 3;
   };
  
 return 1;
};

////////////////////////////////


int init_screen ()
{
 initscr();

 if (has_colors() == FALSE)
   {  
	  endwin();
	  printf("Your terminal does not support color\n");
     getch();
	  return(1);
	 }
 start_color();	
 
 //---------colour pairs---------//
 init_pair(0, COLOR_BLACK, COLOR_BLACK);
 init_pair(1, COLOR_RED, COLOR_BLACK);
 init_pair(2, COLOR_GREEN, COLOR_BLACK);
 init_pair(3, COLOR_YELLOW, COLOR_BLACK);
 init_pair(4, COLOR_BLUE, COLOR_BLACK);
 init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
 init_pair(6, COLOR_CYAN, COLOR_BLACK);
 init_pair(7, COLOR_WHITE, COLOR_BLACK);

 return (0);
};


int drawMap (WINDOW* winchoice)
{
 wmove (winchoice,0,0);
 	for (int y = 0; y < grid_y; y++)
	 {
	  wmove (winchoice,y,0);
	  for(int x = 0; x < grid_x; x++)
		 {
          int maptile_tile;
          int maptile_colour;
          const char *maptile_char;
      
          maptile_tile = game_grid [y][x];
          maptile_colour = tile_library[maptile_tile].color;
          maptile_char = tile_library[maptile_tile].symbol;
     
		  wmove (winchoice,y,x);
		  wprintw_col (winchoice, maptile_char, maptile_colour);
		 }; 
   }; 
 return (0);
};

void generateItems()
 {
  for (int y = 0; y < grid_y; y++)
   {
    for (int x = 0; x < grid_x; x++)
	 {
      int itemsSize = sizeof(itemIndex); //amount of item enums
      int wepsSize = sizeof(weaponIndex); //amount of weapon enums

      //add items to room tiles
	  if (game_grid[y][x] == rom)
	   {
	    //for each of the items in the library
	    for (int  i=0; i<itemsSize; i++)
	     {
	      //generate a random chance out of 100 for each item
	      int chance = rand() %100+1;
	      
	      //if the random chance < the number of items (e.g 10, 10%)
	      if (chance<=itemsSize) 
	       {
            //convert the current index in the library to an item enum 
            //then add it to the item grid
            
            item thisItem = item_library[i];
            
	        item_grid[y][x] = new item(thisItem);
	        
	        //break the for loop
	        break; 
	       }
	     }
	    
	    //spawn each weapon based on a chance out of 100
	    for (int  i=0; i<wepsSize; i++)
	     {
	      int chance = rand() %100+1;
	      
	      //use the number of weapons as the chance of appearing
	      if (chance<=wepsSize) 
	       {
	        weapon thisWeapon = weapon_library[i];
	        item_grid[y][x] = new weapon(thisWeapon);
	        break;
	       }
	     }
	    
	    
	     
	   }
																	
	  else
	   {
	    item_grid[y][x] = new item(item_library[nit]);
	   }
	 }
   }
  return;
 }

void drawItems(WINDOW* winchoice)
 {
  //iterate through the rows of the grid/map
  for (int y = 0; y < grid_y; y++)
   {	  
    for(int x = 0; x < grid_x; x++)
     {
      //create variables to hold item info
      int colour;
      const char *symbol;
        
      //grab the info for each item from the library
      
      if (item_grid[y][x] != NULL)
       {
        item* thisItem = item_grid[y][x]; 
        int itemId = item_grid[y][x]->id; //grab Id for comparison
      
      
      item itm = (item)item_library[nit];
      weapon wpn = (weapon)weapon_library[none];
     
      
      if (itemId != itm.id && itemId != wpn.id)
       {
        colour = thisItem->colour;
        symbol = thisItem->symbol;
        
        //draw the tile to the screen
        wmove (winchoice,y,x);
	    wprintw_col (winchoice, symbol, colour);
	   }
	  
	  } 
     };  
   }; 
   
  return;
 }

int main()
 {
  srand(time(NULL));
  
  //generate characters
  
  player = new Warrior();
  player->setPos (1,1);
  
  for(int a = 0; a < max_npcs; a++) 
   { 
    npcs[a] = NULL;
    npcs[a] = new Goblin(); //Fills the array with the information of the specific class
    npcs[a]->setPos (3,6); //Sets the position of the templated NPC.
   };
  
  cout << "Characters generated"; 
  
  generateItems(); 
  
  cout << "Items generated";
  
  //start ncurses
  cout << "Starting ncurses";
  init_screen ();
 
 //segmenting display into game UI // +2 to positioning to allow for bordering
 WINDOW*titlewin=newwin(1,37,0,0); //Creates the stats window for content
 
 WINDOW*mainwin_rear=newwin(grid_y+2,grid_x+2,1,1); //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
 WINDOW*mainwin_front=newwin(grid_y,grid_x,2,2); //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.
 
 WINDOW*consolewin_rear=newwin(6,grid_x*2+2,grid_y+3,2); //Creates the console window for deco
 WINDOW*consolewin_front=newwin(4,grid_x*2,grid_y+4,3); //Creates the console window for content
 WINDOW*statwin_rear=newwin(10,grid_y+2,1,grid_x+4); //Creates the stat window for deco
 WINDOW*statwin_front=newwin(8,grid_y,2,grid_x+5); //Creates the stats window for content

 WINDOW*invwin_rear=newwin(7,30,11,grid_x+4); //Creates the inventory window for deco
 WINDOW*invwin_front=newwin(5,28,12,grid_x+5); //Creates the inventory window for content
 
 box (mainwin_rear,0,0);
 box (consolewin_rear,0,0); //Puts borders around the finalised screen image
 box (statwin_rear,0,0);
 box (invwin_rear,0,0);

 wprint_at (titlewin,(const char *)"||ARQ -- ASCII Roguelike Quester||",0,3);
 wprint_at (invwin_rear, "Inventory",0,9);
 
 wrefresh (mainwin_rear);
 wrefresh (consolewin_rear);
 wrefresh (statwin_rear);
 wrefresh (invwin_rear);

 keypad(consolewin_front, TRUE);
 
 running = (true);
 
 while ((running==true)) //Main game loop
  {
   //adding/adjusting screen content
   
   drawMap (mainwin_front); //Adds map content to the main window
   
   //SEGFAULT!
   drawItems (mainwin_front); //Adds the map items to the main window

   for(int a = 0; a < maxs; a++)
    {
     if (npcs[a] != NULL)
      {
       npcs[a]->move();
       npcs[a]->draw (mainwin_front);
      }
    };

   player->draw (mainwin_front);
   player->drawInv (invwin_front);
   player->lootcount ();
   player->stats (statwin_front);  //Adds/Updates content to the statistic window
  
      
   //refreshing -- finalises screen image and updates the display
   wrefresh (titlewin);
    
   wrefresh (mainwin_front);
   wrefresh (consolewin_front);
   wrefresh (statwin_front);
   wrefresh (invwin_front);

   player->input (consolewin_front, invwin_front, mainwin_front); //Allows the player to move/take their turn
         
  };
  
 //End of game loop
 werase (titlewin);
 werase (mainwin_rear);
 werase (mainwin_front);
 werase (consolewin_rear);
 werase (consolewin_front);
 werase (statwin_rear);
 werase (statwin_front);
 werase (titlewin);
   
 endwin ();
 return (0);
};

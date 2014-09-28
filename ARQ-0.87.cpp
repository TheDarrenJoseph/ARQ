//////////////////////The ASCII Roguelike Quester -- Experimental project/////////////////
/////////////////////////////////LINUX BUILD//////////////////////////////////////////////
//                                                                                      //
//  Author: Rave Kutsuu                     Version : 0.88                              //
//  Created: Dec 16, 2012                   Last Modified: 20 July, 2014                //
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

WINDOW*titlewin; //Creates the stats window for content
 
WINDOW*mainwin_rear; //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
WINDOW*mainwin_front; //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.
 
WINDOW*consolewin_rear; //Creates the console window for deco
WINDOW*consolewin_front; //Creates the console window for content
 
WINDOW*statwin_rear; //Creates the stat window for deco
WINDOW*statwin_front; //Creates the stats window for content

WINDOW* invwins_front[3][3];
WINDOW* invwins_rear; 
  
WINDOW* equipwin_rear; 
WINDOW* equipwin_outfit; 
WINDOW* equipwin_front[3];

const int grid_x = 30;
const int grid_y = 15;
const int inv_x = 3;
const int inv_y = 5;

const int max_npcs = 5;

using namespace std;

bool running;
bool selection;

string version = "0.82 Win32";

//enum for indexing fixed instances of tile_details
enum tile {ntl,cor,rom,wa1,wa2,wa3,win,od0,cd0,od1,cd1,ld1,od2,cd2,ld2,ent,ext,ded};

//enums for reference to detail structs (for dynamic items, their size is controlled by the last enum)
enum itemIndex {no_item,lockpick,key,statue,gold_coin,silver_coin,bronze_coin,gold_bar,silver_bar,bronze_bar,item_size};
enum weaponIndex {no_weapon,fist,claw,acid,sword,axe,dagger,spear,weapon_size};
enum outfitIndex {no_outfit,warrior,goblin,outfit_size};

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
    this->damage=0;
   }
   
  weapon(int i, string n, const char* s, int col, int val,int d) : item(i,n,s,col,val)
   {
    damage = d;
   }
 };
 
struct outfit : public item
 {
  int armourPoints;
    
  outfit() : item(0,"None"," ",0,0)
   {
    this->armourPoints=0;
   }
   
  outfit(int i, string n, const char* s, int col, int val,int AP) : item(i,n,s,col,val)
   {
    armourPoints = AP;
   }
 };

//creates detail for each tile, must be the same size as the enum
//these only ever have one instance, which are used as reference.
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

item item_library[item_size] = 
{ 
 // {id}{Name(12 cha)}{Symbol}{Col}{Val}
    {0,  "None"      , " ",    0,   0  },
    {1,  "Lockpick"  , "!",    2,   1  }, 
    {2,  "Door Key"  , "!",    2,   5  }, 
    {3,  "Statue"    , "$",    2,   500},
    {4,  "Gld Coin"  , "$",    2,   50 },
    {5,  "Silv Coin" , "$",    2,   25 },
    {6,  "BrnzCoin"  , "$",    2,   5  },
    {7,  "GoldBar"   , "$",    2,   100},
    {8,  "SilvrBar"  , "$",    2,   50 },
    {9,  "BrnzeBar"  , "$",    2,   25 },
};

weapon weapon_library[weapon_size] = 
{
 // {id}{Name(12 cha)}{Symbol}{Col}{Val}{Dam}
 {0,"None"      ," ",1,0,0},
 {1,"Fist"      ,"W",0,0,5},
 {2,"Claw"      ,"W",0,0,7},
 {3,"Acid"      ,"W",0,0,20},
 {4,"Sword"     ,"W",2,0,15},
 {5,"Axe"       ,"W",2,0,12},
 {6,"Dagger"    ,"W",2,0,8},
 {7,"Spear"     ,"W",2,0,10},
};

outfit outfit_library[outfit_size] =
{
 // {id}{Name(12 cha)}   {Symbol}{Col}{Val}{AP}
 {0,"None"               ,"O"    ,0   ,0   ,0},
 {1,"Warrior Armour"     ,"O"    ,0   ,0   ,30},
 {2,"Crude Goblin Armour","O"    ,0   ,0   ,20},
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
  int max_health;
  
  bool alive;

  weapon weps [3];
  outfit currentOutfit;

  public:
  
  bool IsAlive();

  void Kill();
  
  bool CanFlee();
  void SetCharacter(char char_choice, int colour_choice, string name_choice, int health_choice);

  void GetPos (int *x, int *y);

  string GetName();
  int GetHealth ();

  void SetWeps(weaponIndex one, weaponIndex two, weaponIndex three);
  weapon* GetWeps(); //returns a pointer to allow return of a 1D array
  
  void SetOutfit(outfit o);
  outfit GetOutfit();
  
  void SetPos(int x, int y);
  void SetHealth(int);
  void Draw(WINDOW* winchoice);
   
  virtual int Move() =0; /*Do nothing by default, abstract*/
     
  Character (char c, int col, string name, int health) 
   {
    max_health = health;
   	SetCharacter(c,col,name,health);
   	
   	for (int i=0; i<3; i++)
   	 {
   	  weps[i] = weapon(weapon_library[no_weapon]);
   	 }
   	 
   	SetOutfit(outfit_library[no_outfit]);
   };
 };

class Player : public Character
{
 private:
 item* inventory[inv_y][inv_x];
 int loot;
  
 public :
 
 int DrawInv (WINDOW* windows_front[3][3]);
 int DrawEquip (WINDOW* windows_front[3], WINDOW* outfitWin);
 
 bool DropItem(item* itm, int x, int y);
 int Move ();
 int Move (WINDOW * winchoice, WINDOW* mainwin, int y, int x);

 int AccessInventory (WINDOW* input_win, WINDOW* inv_wins[3][3]);

 int AddToInventory (item* itm);
 
 int ItmProc (WINDOW* winchoice, item* itm, int y, int x);
 int LockProc (WINDOW * winchoice, int door_y, int door_x, tile doortype, int doortile, string doorname );
 int DoorProc (WINDOW * winchoice, int y, int x, tile doortype);
 
 int BattleTurn (WINDOW* main_win, WINDOW* input_win, int npc_id);
 
 void SetLoot(int x);
 int GetLootScore();
 
 int Input (WINDOW* winchoice, WINDOW* inv_wins[3][3], WINDOW* main_win);
 int Stats (WINDOW* winchoice);
 
 bool IsLootable(item* i);

 int LootCount ();
 
 Player (char c, int col, string name, int health) : Character(c,col,name,health)
  {
   //initialise the item inventory
   for (y=0; y<inv_y; y++)
    {
     for (x=0; x<inv_x; x++)
      {
       inventory[y][x] = new item(item_library[no_item]);
      }
    }
  };
};

class Warrior : public Player
 {
  public:

  Warrior () : Player('@',6,"Warrior",100)
   {
    SetWeps(no_weapon,fist,sword);
    SetOutfit(outfit_library[warrior]); 
   };
  
 };
 
class NPC : public Character
{
 public :
 int BattleTurn ();
 int Move();
 
 NPC (char c, int col, string name, int health) : Character(c,col,name,health)
  {
   this->SetPos(6,6);
  }; 
};


class Goblin : public NPC
{
 public:

 Goblin() : NPC ('@',2,"Wild Goblin",30)
  {
   SetWeps (no_weapon,claw,sword);
   SetOutfit(outfit_library[goblin]);
  };
  
};

class Slime : public NPC
{
 public:
 Slime () : NPC ('*',1,"Oozy Slime",10)
  {
   SetWeps (no_weapon,acid,claw);
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

Player * player;
NPC * npcs[max_npcs];


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

void Battle (WINDOW* main_win, WINDOW* input_win, int npc_id)
{
 string p_name = player->GetName();
 string npc_name = npcs[npc_id]->GetName();
  
 werase (input_win); //Clears both the input and map windows to fit the battle scenario
 werase (main_win);
 
 while (true)
  {
  //if either has died, kill them and return
  if (player->IsAlive() == false)
   {
    werase (input_win);
    wprintw (input_win, "You have been slain..");
    player->Kill();
    wgetch(input_win);
    return;
   }
  
  else if (npcs[npc_id]->IsAlive() == false)
   {
    werase (input_win);
    wprintw (input_win, "Your foe has been slain..");
    
    npcs[npc_id]->Kill();
    wgetch(input_win);
    
    return;
   }
  
  else
   {
    //display the info of both combatants
    wmove (input_win,0,0);
    wprintw (input_win,"%s Health: %d \n",p_name.c_str(),player->GetHealth());
    wprintw (input_win,"%s Health: %d \n",npc_name.c_str(),npcs[npc_id]->GetHealth());
    wgetch (input_win);
  
    //Get both combatants weapons
    weapon* p_weps = player->GetWeps();
    weapon* npc_weps = npcs[npc_id]->GetWeps();
  
    //allow both combatants to make a move
    int p_move = player->BattleTurn(main_win,input_win,npc_id); 
    int npc_move = npcs[npc_id]->BattleTurn();  
    
    if ( (p_move<=2) && (npc_move<=2) && (npcs[npc_id]->IsAlive()) && (player->IsAlive()) ) 
     {
      //select the weapon using their move choice
      weapon p_choice = p_weps[p_move];  
      weapon npc_choice = npc_weps[npc_move];
  
      //show each players choices
      werase (input_win);
      
      if (p_move != 0)
       {
        wprintw(input_win,"You use your %s and strike the enemy for %i damage\n",p_choice.name.c_str(),p_choice.damage);
       }
      
      else if (p_move == 0 && npc_move != 0)
       {
        wprintw(input_win,"You do nothing, and are struck for %i damage", npc_choice.damage);
       };
       
      if (npc_move != 0 && p_move != 0)
       {
        wprintw(input_win,"Your enemy uses their %s and deals %i damage \n",npc_choice.name.c_str(),npc_choice.damage);
       }
       
      else if (npc_move == 0 && p_move != 0)
       {
        wprintw(input_win,"Your enemy does nothing and takes %i damage \n",p_choice.damage);
       }
             
      else
       {
        wprintw(input_win,"You both do nothing.");
       };
      
      wgetch (input_win);
      player->SetHealth(player->GetHealth()-10);
      npcs[npc_id]->SetHealth(npcs[npc_id]->GetHealth()-p_choice.damage);
     }
     
    else if (p_move == 3)
     {
	  wprintw(input_win,"You attempt to flee."); 
	  //flee
	  wgetch (input_win);
	 }
   }
 }
  
}

///////////////////////////////////////
bool Character :: IsAlive ()
 {
  if (health<=0)
   {
    this->Kill();
    return false;
   }
  
  else
   {
    return true;
   }
  
 };
 
bool Character :: CanFlee()
 {
  if (health>(health/100)*25)
   {
	return true;
   }
  
  else
   {
	return false;
   }
 }


void Character :: SetCharacter (char char_choice, int colour_choice, string name_choice, int health_choice)
 {
  alive = true;
  this->symbol = char_choice;
  this->colour = colour_choice;
  this->name = name_choice;
  this->health = health_choice;
  
  void SetPos(int x, int y);
 };

void Character :: Kill()
 {
  alive = false;
  this->symbol = 'X';
  this->colour = 1;
 };

string Character :: GetName()
 {
  return this->name;
 } 

void Character :: GetPos (int *x, int *y)
 {
  (*x = this->x);
  (*y = this->y);
 };

int Character :: GetHealth ()
 {
  return this->health;
 };
  
void Character :: SetHealth(int i)
 {
  this->health = i;
  return;
 }

void Character :: SetPos(int x, int y)
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
 
void Character :: Draw (WINDOW* winchoice)
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

bool Player :: DropItem(item* itm, int x, int y)
 {
  if (IsLootable(inventory[y][x]))
   {    
    item_grid[this->y][this->x] = itm; //replace the map tile with the item
    
    inventory[y][x] = new item(item_library[no_item]); //clear the inventory tile
    return true;
   }
            
  else
   {
    return false;
   }; 
 }

int Player :: AccessInventory (WINDOW* input_win, WINDOW* inv_wins[3][3])
 {
  item* inv_tile;
  int invtile_colour;
  
  string str;
  string slc_string;
  char slc_char[20];
  
  int loc_x = 0;
  int loc_y = 0;
  
  //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
  selection = true;

  while (selection == true) 
   {
   
    for (int x=0; x<3; x++)
     {
      for (int y=0; y<3; y++)
       {
        WINDOW* thisWin = inv_wins[x][y];
        
        mvwchgat (thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
        wrefresh (thisWin);
       }
     }
     
    WINDOW* selWin = inv_wins[loc_x][loc_y]; //create a holder for the currently selected window
    
    mvwchgat (selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
    wrefresh (selWin);
     
    werase (input_win);
    wprint_at (input_win, "Select an item with WASD.. 'Drop' to drop the selected item, 'Exit' to cancel", 0,0);
    
    wprint_at (input_win,"ARQ:~$ ",2,0);
    
    wgetstr (input_win, slc_char);
    slc_string = slc_char;
    
    if ((slc_string == "W") || (slc_string == "w")) 
     {
      if (loc_y != 0)
       {
        loc_y--;
       };
     }
  
    else if ((slc_string == "A") || (slc_string == "a")) 
     {
      if (loc_x != 0)
       {
        loc_x--;
       };
     }
     
    else if ((slc_string == "S") || (slc_string == "s")) 
     {
      if (loc_y != 2)
       {
        loc_y++;
       };
     }
  
    else if ((slc_string == "D") || (slc_string == "d")) 
     {
      if (loc_x != 2)
       {
        loc_x++;
       }
     }
    
    else if ((slc_string == "Exit") || (slc_string == "exit") || (slc_string == "EXIT")) 
     {
      selection = false;
      return (0);
     }
     
    else if ((slc_string == "Drop") || (slc_string == "drop") || (slc_string == "DROP")) 
     {
      werase (input_win);
      tile playerTile = game_grid[this->y][this->x];
      item* thisItem = inventory[loc_y][loc_x];
      
      if ((playerTile == ntl) || (playerTile == rom) || (playerTile == cor))
       {	     
        if (DropItem(thisItem,loc_x,loc_y) == true)
         {
          //update the current item details 
          item* thisItem = inventory[loc_y][loc_x];
          const char* invtile_char = thisItem->name.c_str();
          
          WINDOW* thisWin = inv_wins[loc_x][loc_y];
          
          //update the inventory tile
          wmove (thisWin,0,0);
          wprintw_col (thisWin, invtile_char, thisItem->colour);
          wrefresh (thisWin);
          
          wprintw (input_win, "Item dropped.");
         }
         
        else
         {
          wprintw (input_win, "No item selected.");
         }
       }
      
      else
       {
        wprintw (input_win, "Cannot drop an item here.");
       };
      
      //wrefresh (inv_win);
      wgetch(input_win);
     }
     
    else if ((slc_string == "Use") || (slc_string == "use") || (slc_string == "USE")) 
     {
      if (IsLootable (inventory[loc_y][loc_x]) == false)
       {
        inv_tile = inventory[loc_y][loc_x]; //grab the current inventory tile
                    
        invtile_colour = inv_tile->colour;
        str = inv_tile->name;
        
        WINDOW* thisWin = inv_wins[loc_x][loc_y];
           
        const char *invtile_char = str.c_str();
        
        wmove (thisWin,0,0);
        wprintw_col (thisWin, invtile_char, invtile_colour);
        wrefresh (thisWin);
        
        werase (input_win);
        wprintw (input_win, "%s selected.",str.c_str());
        wgetch(input_win);
        
        for (int i=0; i<outfit_size; i++)
         {
          //if this item matches an outfit, assume it is and equip it
          if (str == outfit_library[i].name)
           {
            //store the old outfit
            outfit oldOutfit = currentOutfit;
            
            wprintw (input_win, "You change from %s into %s",currentOutfit.name.c_str(),str.c_str());
            wgetch(input_win);
            
            //change into the new outfit
            SetOutfit (outfit_library[i]);
            
            
            wprintw (input_win, "You put your old %s into your inventory",oldOutfit.name.c_str());
            wgetch(input_win);
            
            //set the current inventory tile to the old outfit
            inventory[loc_y][loc_x] = new outfit(oldOutfit); //instanciates a new outfit to fix polymorphism issues
            
       
           }
         }
        
        
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
 
int Player :: AddToInventory (item* itm) 
{
 for (int y = 0; y < inv_y; y++)
  {
   for(int x = 0; x < inv_x; x++)
    {
     if (IsLootable(inventory[y][x]) == false) //If slot is empty
      { 
	   inventory[y][x] = itm; //Place the item in empty slot
       return (0);
      };
	}; 
  };
 return(1);	
};

bool Player :: IsLootable(item* i)
 {
  if (i != NULL)
   { 
    int itemId = i->id; //grab Id for comparison
    int itemCol = i->colour; //grab colour for non-npc item check
    
    if (itemId != 0 && itemCol != 0)
     {
      return true;
     }
  
    else
     {
      return false;
     }
   }
  
  else
   {
    return false;
   }
 }

int Player :: ItmProc (WINDOW* winchoice, item* itm, int y, int x)
{
 string answer;
 char answerchar[20];
 string itm_name = itm->name; 
 
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
    if (AddToInventory (itm) == (1)) //If addToInventory unsuccessful
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
      item_grid[y][x] = new item(item_library[no_item]); //removes the item from the map
      
      SetPos(x,y);

      return (0);
     };
   }
   
  else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
    {
     wmove (winchoice,0,0);
     werase (winchoice);
     wprintw (winchoice,"You leave the %s untouched..",itm_name.c_str());
     wgetch (winchoice);
     
     SetPos(x,y); 
     return (0);
    }
    
   else 
    {
     wprint_at (winchoice, "Incorrect choice, please answer yes or no.. ",0,0);
     wgetch (winchoice);
     ItmProc (winchoice,itm,y,x);
    };
    
 return (0);
};

int Player :: DrawEquip (WINDOW* windows[3],WINDOW* outfitWin)
 {
  //draw weapons
  for(int x = 0; x < inv_x; x++)
   {
    WINDOW* thisWindow = windows[x];
    
    werase(thisWindow);
	wmove(thisWindow,0,0); 
		   
    int colour = this->weps[x].colour;
    string str = this->weps[x].name;
         
    const char* symbol = str.c_str();
    wprintw_col (thisWindow, symbol, colour);         
     
   }; 
   
    
  werase(outfitWin);
  wmove(outfitWin,0,0); 
		   
  int colour = this->currentOutfit.colour;
  string str = this->currentOutfit.name;
         
  const char* nameChars = str.c_str();

  wprintw_col (outfitWin, nameChars, colour);         
  wrefresh (outfitWin);
  
  return (0);
 };

int Player :: DrawInv (WINDOW* windows_front[3][3])
 {
 	for (int y = 0; y < 3; y++)
	 {
	  //draw each item on this row to the screen
	  for(int x = 0; x < 3; x++)
		{
         WINDOW* thisWindow = windows_front[x][y];
         werase(thisWindow);
		 wmove(thisWindow,0,0); 
		 
		 if (inventory[x][y] != NULL)
		  {
		   item* itm = inventory[x][y];
		   
           int colour = itm->colour;
           string str = itm->name;
         
           const char* symbol = str.c_str();

           wprintw_col (thisWindow, symbol, colour);         
          }
		}; 
	  };  
 return (0);
};

void Character :: SetWeps (weaponIndex one, weaponIndex two, weaponIndex three)
 {  
  this->weps[0] = weapon_library[one];
  this->weps[1] = weapon_library[two];
  this->weps[2] = weapon_library[three];
 };
  
weapon* Character :: GetWeps()
 {
  return this->weps;
 }
 
void Character :: SetOutfit(outfit o)
 {
  //remove the current AP buff
  this->max_health -= currentOutfit.armourPoints;
  cout << max_health;
  
  //remove any extra health gained from the armour if it exceeds the buff
  if (GetHealth()>max_health)
   {
    SetHealth(health-currentOutfit.armourPoints);
    cout <<health << "-" << currentOutfit.armourPoints; 
   }
   
  this->currentOutfit = o;
  
  //increase max health to suit the outfit
  this->max_health += o.armourPoints;
 
  //add the AP to the current health
  SetHealth(health+o.armourPoints);
  return;
 }
 
outfit Character :: GetOutfit()
 {
  return currentOutfit;
 }
 
int Player :: Stats (WINDOW* winchoice)
{
 werase (winchoice);
 wmove (winchoice,0,1);
 wattron(winchoice,A_UNDERLINE); //turns on the chosen colour pair
 wprintw (winchoice,"%s",this->GetName().c_str());
 wattroff(winchoice,A_UNDERLINE); //turns off the chosen colour pair
 
 wmove (winchoice,1,0);
 wprintw_col (winchoice,"Health: ",4); 
 wprintw (winchoice,"%d",this->GetHealth());
 
 wmove (winchoice,2,0);
 wprintw_col (winchoice,"Loot:   ",4); 
 wprintw (winchoice,"%d",this->GetLootScore());
 
 for( int a = 0; a < max_npcs; a++ )
     {
      wmove (winchoice,a+3,0);
      //LIST HEALTH OF NPCS?
     };
 return (0);
}

int Player :: LockProc (WINDOW * winchoice, int door_y, int door_x, tile doortype, int doortile, string doorname )
{
 string answer;
 item* inv_tile;
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
     int key_count;
     int count;
     count = 0;
     key_count = 0;
      
      for (int y = 0; y < inv_y; y++)
       {
        for(int x = 0; x < inv_x; x++)
         {
          inv_tile = inventory[y][x];
          
          if (inv_tile->id == key)
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
           
           if (inv_tile->id == (key))
            {
             inventory[y][x] = new item(item_library[no_item]);
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
             
             this->SetPos(door_x,door_y);               
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
     int lockpick_count;
     int count;
     count = 0;
     lockpick_count = 0;
      
      for (int y = 0; y < inv_y; y++)
       {
        for(int x = 0; x < inv_x; x++)
         {
          inv_tile = inventory[y][x];
          
          if (inv_tile->name == item_library[lockpick].name)
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
            
            if (inv_tile->name == item_library[lockpick].name)
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
                 
                inventory[y][x] = new item (item_library[no_item]);
                count = (count+1);
                chance = rand() %100+1;
                lockno = (lockno+1);
               }
              
              else if (chance <= 50)
               {
                werase (winchoice);
                wprint_at (winchoice,"Your lockpick breaks as you attempt to open the lock.. ",0,0);
                wgetch (winchoice);
                    
                inventory[y][x] = new item (item_library[no_item]);
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
                                    
               this->SetPos(door_x,door_y);  
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
     this->LockProc (winchoice,door_y,door_x,doortype,doortile,doorname);
    };
    
 return (0);
};

int Player :: BattleTurn (WINDOW* main_win, WINDOW* input_win, int npc_id)
 {
  werase(input_win);
  
  for (int i=0; i<3; i++)
   {
    string s = weps[i].name;
    wprint_at(input_win,s.c_str(),0,9*i);
   }
   
  wprint_at(input_win,"Flee",0,9*3);
   
  ////input
  int index = 0;
  int scr_x = 9;
  int scr_y = 0;
  
  bool selection = true;
  
  while (selection == true) 
   {
    scr_x = index*9; //calculate the start of the item name
   
    mvwchgat (input_win, scr_y, 0, 27, A_NORMAL, 0, NULL); //remove fancy effects
    mvwchgat (input_win, scr_y, scr_x, 9, A_BLINK, 1, NULL); //add red blink to the current item
    wrefresh (input_win);
    
    wprint_at (input_win,"Use A and D to choose a weapon, 'use' to select",1,0);
    wprint_at (input_win,"ARQ:~$ ",2,0); //give us a prompt
    
    string choice; 
    char input[20];

    wgetstr (input_win, input); //store our grabbed chars
    choice = input; //store the chars as a string for comparison
     
    if ((choice == "a") && (index>0))
     {
      index--;
     }
   
    else if ((choice=="d") && (index<3))
     {
      index++;
     }
     
    else if (choice=="use")
     {
      werase(input_win);
      wprintw(input_win,"Weapon selected.");
      return index;
     }
   
   } 
   
   return 0;
  }

int Player :: DoorProc (WINDOW * winchoice, int y, int x, tile doortype)
{
 int map_tile = game_grid[y][x];
 string door_name = tile_library[map_tile].name;
 
    if ((doortype == od0) || (doortype == od1) || (doortype == od2))
     {
        wmove (winchoice,0,0);
        wprintw (winchoice,"You enter the doorway of the %s.. ",door_name.c_str());
        wgetch (winchoice);
        this->SetPos(x,y);  
   
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
          
         this->SetPos(x,y);  
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
         
         DoorProc (winchoice, y, x, game_grid[y][x]);
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
         LockProc (winchoice, y,x,doortype,map_tile,door_name);
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
         
         DoorProc (winchoice, y, x, game_grid[y][x]);
        };
      return (0);
     };

 return (0);
};

int Player :: Move()
 {
  return 1;
 }

int Player :: Move (WINDOW * winchoice, WINDOW* mainwin, int y, int x)
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
   if (npcs[e_id] != NULL)
    {
     int enemy_x;
     int enemy_y;
     npcs[e_id]->GetPos (&enemy_x, &enemy_y); //Sets enemy x and y to those of npc[No] through pointers
  
     int x;
     int y;
     player->GetPos (&x, &y); 
   
     if ( ((pmove_x == enemy_x and pmove_y == enemy_y) || (x == enemy_x and y == enemy_y)) && (npcs[e_id]->IsAlive()))
      {
       string enemy_name = npcs[e_id]->GetName();
         
	   wmove (winchoice,0,0);
	   wprintw (winchoice,"You are confronted by a %s! ",enemy_name.c_str());
	   wgetch (winchoice);
		        
       Battle (mainwin, winchoice, e_id);
     
       return (0);
      }
      
     else if (pmove_x == enemy_x and pmove_y == enemy_y)
      {
       string enemy_name = npcs[e_id]->GetName();

	   wmove (winchoice,0,0);
	   wprintw (winchoice,"There is the corpse of a %s here..",enemy_name.c_str());   
	   
	   //Lootproc? (loot the dead body?)
	     
       wgetch (winchoice);
      };
    }
  };

 if (IsLootable(item_grid[y][x]))
  {
   item* itm = item_grid[y][x];
   
   ItmProc (winchoice, itm, y, x);
   
   return (0);
  }
      
 if (game_grid[y][x] == ntl)
  {
   return (1);
  }
    
 else if ((game_grid[y][x] == cor) || (game_grid[y][x] == rom))
  {
   wmove (winchoice,0,0);
   
   wprintw (winchoice,"You walk along the %s.. ",move_tilename.c_str());
   wgetch (winchoice);
   
   SetPos(pmove_x,pmove_y);  
   
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
   this->DoorProc (winchoice, y, x, game_grid[y][x]);
   return (0);
  }
  
 else if (game_grid[y][x] == cd0 || (game_grid[y][x] == cd1) || (game_grid[y][x] == cd2) || (game_grid[y][x] == ld1) || (game_grid[y][x] == ld2)) 
  {
   wmove (winchoice,0,0);
   
   wprintw (winchoice,"There's a %s here..",move_tilename.c_str());
   wgetch (winchoice);
   
   this->DoorProc (winchoice, y, x, game_grid[y][x]);
   
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
   wgetch (winchoice);
   
   SetHealth(this->GetHealth()-20);
  
   return (0);
  };

return (0);
};

void Player :: SetLoot(int i)
 {
  this->loot = i;
  return;
 }

int Player :: GetLootScore ()
 {
  return this->loot;
 };

int Player :: Input (WINDOW* winchoice, WINDOW* inv_wins[3][3], WINDOW* main_win)
{
 int x;
 int y;
   
 this->GetPos(&x,&y); /*Get current position for movement*/

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
   this->Move (winchoice, main_win, y-1,x);
   return (0);
  }
  
  if ((answer == "east") || (answer == "East") || (answer == "EAST") || (answer == "e") || (answer == "E"))
  {
   this->Move (winchoice, main_win, y, x+1);
   return (0);
  }
  
  if ((answer == "south") || (answer == "South") || (answer == "SOUTH") || (answer == "s") || (answer == "S")) 
  {
   this->Move (winchoice, main_win, y+1, x);
   return (0);
  }
  
  if ((answer == "west") || (answer == "West") || (answer == "WEST") || (answer == "w") || (answer == "W"))
  {
   this->Move (winchoice, main_win, y, x-1);
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
   AccessInventory (winchoice, inv_wins);
   return(0);
  }
  
 else
  wprint_at (winchoice,(const char *)"unrecognised input, please input a command, or use 'help' for a list. ",0,0);
  wgetch (winchoice);
  
  Input (winchoice, inv_wins, main_win);
  
  return (0);

return (0);
};

int Player :: LootCount ()
{
 int total = 0;

 for (int y = 0; y < inv_y; y++)
  {
    for(int x = 0; x < inv_x; x++)
     {
      int value = inventory[y][x]->value;
      total+=value;
     };
   };

 this->SetLoot(total);
 return (0);
};
 
/////////////////////////////////////////////////

//returns an int to represent a weapon choice from weps[] based on health
int NPC :: BattleTurn ()
 {
  if (health>50)
   {
    return 1;
   }
   
  else if (health<50 && health>25)
   {
    return 2;
   }
   
  else
   {
    return 0;
   }
 };
 
//randomly moves within the 6 local directions
int NPC :: Move ()
 {
  int x = ((rand() % 3 - 1));
  int y = ((rand() % 3 - 1));
  
  int pos_x;
  int pos_y;
    
  this->GetPos(&pos_x,&pos_y);  

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
  
  for (int No = 0; No < max_npcs; No++)
  {
   for (int NPCNo = 0; NPCNo < max_npcs; NPCNo++)
    {
     if ((npcs[No] != NULL) && (npcs[NPCNo] != NULL))
      {
       int player_x;
       int player_y;
       int character_x;
       int character_y;
    
       npcs[No]->GetPos (&player_x, &player_y);
       npcs[NPCNo]->GetPos (&character_x, &character_y);
     
      
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
  };
  
  if ((game_grid[move_y][move_x] != ntl) and (game_grid[move_y][move_x] != wa1) and (game_grid[move_y][move_x] != wa2) and (game_grid[move_y][move_x] != wa3) and (game_grid[move_y][move_x] != ent) and (game_grid[move_y][move_x] != ext))
   {
    this->SetPos (move_x,move_y);
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
 cout << "Starting ncurses";
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


int DrawMap (WINDOW* winchoice)
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

void DrawItems(WINDOW* winchoice)
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
      
      if (player->IsLootable(item_grid[y][x]))
       {
        colour = item_grid[y][x]->colour;
        symbol = item_grid[y][x]->symbol;
        
        //draw the tile to the screen
        wmove (winchoice,y,x);
	    wprintw_col (winchoice, symbol, colour);
	   }
	   
     };  
   }; 
   
  return;
 }

bool IsLootable(item i)
 {
  int itemId = i.id; //grab Id for comparison
  int itemCol = i.colour; //grab colour for non-npc item check
    
  if (itemId != 0 && itemCol != 0)
   {
    return true;
   }
  
  else
   {
    return false;
   };
 }
 
 
void GenerateItems()
 {
  for (int y = 0; y < grid_y; y++)
   {
    for (int x = 0; x < grid_x; x++)
	 {
      //add items to room tiles
	  if (game_grid[y][x] == rom)
	   {
	    //for each of the items in the library
	    for (int  i=0; i<item_size; i++)
	     {
	      //generate a random chance out of 100 for each item
	      int chance = rand() %100+1;
	      
	      //convert the current index in the library to an item enum 
	      item thisItem = item_library[i];
	      
	      //if the random chance < the number of items (e.g 10, 10%)
	      if (chance<=item_size)  
	       {
            //then add it to the item grid
            
	        item_grid[y][x] = new item(thisItem);
	        
	        //break the for loop
	        break; 
	       }
	     }
	    
	    //spawn each weapon based on a chance out of 100
	    for (int  i=0; i<weapon_size; i++)
	     {
	      weapon thisWeapon = weapon_library[i];
	      
	      int chance = rand() %100+1;
	      
	      //use the number of weapons as the chance of appearing
	      
	      if (chance<=weapon_size && IsLootable(thisWeapon)) 
	       {
	        item_grid[y][x] = new weapon(thisWeapon);
	        cout << thisWeapon.name;
	        break;
	       };
	     }
	  
	   }
																	
	  else
	   {
	    item_grid[y][x] = new item(item_library[no_item]);
	   }
	 }
   }
   
  cout << "Items generated\n"; 
  return;
 }
 
void InitNpcs()
 {
  for(int a = 0; a < max_npcs; a++) 
   { 
    npcs[a] = NULL;
    npcs[a] = new Goblin(); //Fills the array with the information of the specific class
    npcs[a]->SetPos (3,6); //Sets the position of the templated NPC.
   };
   
  cout << "Characters generated\n"; 
  return;
 }

void InitWindows()
 {
  titlewin=newwin(1,37,0,0); //Creates the stats window for content
 
  mainwin_rear=newwin(grid_y+2,grid_x+2,1,1); //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
  mainwin_front=newwin(grid_y,grid_x,2,2); //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.
 
  consolewin_rear=newwin(6,grid_x*2+7,grid_y+3,1); //Creates the console window for deco
  consolewin_front=newwin(4,grid_x*2+5,grid_y+4,2); //Creates the console window for content
 
  statwin_rear=newwin(5,grid_x+4,1,grid_x+4); //Creates the stat window for deco
  statwin_front=newwin(3,grid_x-2,2,grid_x+5); //Creates the stats window for content

  invwins_rear = newwin(7,grid_x+4,11,grid_x+4); 
 
  for (int x=0; x<3; x++)
   {
    for (int y=0; y<3; y++)
     {
      invwins_front[x][y] = newwin(1,10, 12+(y*2), grid_x+5+(x*11)); //first half as content window
     }
   }
  
  equipwin_rear = newwin(5,grid_x+4,6,grid_x+4); 
  equipwin_outfit = newwin(1,30,9,grid_x+5); 
  
  for (int x=0; x<3; x++)
   {
    for (int y=0; y<2; y++)
     {
      equipwin_front[x] = newwin(1,10, 7, grid_x+5+(x*11)); //first half as content window
     }
   }
  
  cout << "Display Initialised\n";
 }
 
void DestroyWindows()
 {
  werase (titlewin);
  
  werase (mainwin_rear);
  werase (mainwin_front);
  
  werase (consolewin_rear);
  werase (consolewin_front);
  
  werase (statwin_rear);
  werase (statwin_front);
  
  werase (invwins_rear);
  
  for (int x=0; x<inv_x; x++)
   {
	for (int y=0; y<inv_y; y++)
	 {
	  werase (invwins_front[y][x]);
	 }
   }
  
  werase (equipwin_rear);
  
  for (int x=0; x<3; x++)
   {
	for (int y=0; y<3; y++)
	 {
	  WINDOW * thisWin = &equipwin_front[y][x];
	  werase (thisWin);
	 }
   }
   
  werase (equipwin_outfit);
 }
 
void DecorateWindows()
 {
  wprint_at (titlewin,(const char *)"||ARQ -- ASCII Roguelike Quester||",0,3);
   
  //draw borders around the main windows
  box (mainwin_rear,0,0);
  box (consolewin_rear,0,0); //Puts borders around the finalised screen image
  box (statwin_rear,0,0); 
  box (invwins_rear,0,0);
  box (equipwin_rear,0,0);
   
  //add titles to those that need them
  wprint_at(statwin_rear,"Player Stats",0,10);  
  wprint_at(equipwin_rear,"Equipment",0,11);  
  wprint_at(invwins_rear,"Inventory",0,11);  
  
  wrefresh (mainwin_rear);
  wrefresh (consolewin_rear);
  wrefresh (statwin_rear);
  wrefresh (invwins_rear);
  wrefresh (equipwin_rear);
 }

void GameLoop()
 {
  //adding/adjusting screen content
   
  DrawMap (mainwin_front); //Adds map content to the main window
  DrawItems (mainwin_front); //Adds the map items to the main window

  for(int a = 0; a < max_npcs; a++)
   {      
    if ((npcs[a] != NULL) && (npcs[a]->IsAlive()))
     {
      npcs[a]->Move();
      npcs[a]->Draw (mainwin_front);
     }
     
    else if (npcs[a] != NULL)
     {
      npcs[a]->Draw (mainwin_front);
     };
   };
   
  player->Draw (mainwin_front);
  player->DrawInv (invwins_front); 
  player->DrawEquip (equipwin_front,equipwin_outfit);
  //player->lootcount ();
   
  player->Stats (statwin_front);  //Adds/Updates content to the statistic window
       
  //refreshing -- finalises screen image and updates the display
  wrefresh (titlewin);
    
  wrefresh (mainwin_front);
  wrefresh (consolewin_front);
  wrefresh (statwin_front);
  wrefresh (equipwin_outfit);
   
  for (int x=0; x<3; x++)
   {
    for (int y=0; y<3; y++)
     {
      wrefresh (invwins_front[x][y]);
     }
       
    wrefresh (equipwin_front[x]);  
   }

  player->Input (consolewin_front, invwins_front, mainwin_front); //Allows the player to move/take their turn
         
 }
 

int main()
 {
  //set time for randomiser
  srand(time(NULL));
  
  //prep data
  player = new Warrior();
  player->SetPos (1,1);
  
  InitNpcs();
  
  GenerateItems(); 
  
  //prep display
  init_screen ();
 
  InitWindows();
  DecorateWindows();
 
  keypad(consolewin_front, TRUE);
 
  running = (true);
 
  while ((running==true)) //Main game loop
   {
    GameLoop();
   };
  
  //End of game loop
  DestroyWindows();  
  endwin ();
  
  return (0);
 };

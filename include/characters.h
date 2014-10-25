#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <string>
#include <curses.h>

#include "items.h"
#include "tile.h"

const int inv_x = 3;
const int inv_y = 3;

const int max_npcs = 1;

class Character
{
 protected:
  int x;
  int y;
  
  char symbol;
  int colour;
  std::string name;
  int health;
  int max_health;
  
  bool alive;

  weapon weps [3];
  outfit currentOutfit;

 public:
  
  bool IsAlive();

  void Kill();
    
  bool IsTraversable(tile t);
  bool CanFlee();
  int Flee();
  
  void Draw();
  
  void SetCharacter(char char_choice, int colour_choice, std::string name_choice, int health_choice);

  void GetPos (int *x, int *y);
  
  int GetColour();
  char GetSymbol();

  std::string GetName();
  int GetHealth ();

  void SetWeps(weaponIndex one, weaponIndex two, weaponIndex three);
  weapon* GetWeps(); //returns a pointer to allow return of a 1D array
  
  void SetOutfit(outfit o);
  outfit GetOutfit();
  
  void SetPos(int x, int y);
  void SetHealth(int);
   
  virtual int Move() =0; /*Do nothing by default, abstract*/
  
  void DropItems();
   
  Character (char c, int col, std::string name, int health) 
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

class NPC : public Character
{
 public :
  int BattleTurn ();
  int Move();
 
  NPC (char c, int col, std::string name, int health) : Character(c,col,name,health)
    {
      //this->SetPos(6,6);
    }; 
};

class Player : public Character
{
 private:

  area* inventory = new area(1,"Inventory","%",1,0,true);

  int loot;
  
  public :
 
  bool CanDropItem(item* thisItem);
  int DropItem(item* thisItem, int invX, int invY);

  int Move ();
  int Move (WINDOW * winchoice, WINDOW* mainwin, int y, int x);
 
  int AccessContainer(WINDOW* input_win, WINDOW* inv_wins[3][3], container* c); 
  int AccessArea(WINDOW* input_win, WINDOW* inv_wins[3][3], area* a);
  int AccessInventory(WINDOW* input_win, WINDOW* inv_wins[3][3]);

  item* GetFromInventory(WINDOW* input_win, WINDOW* inv_wins[3][3]);

  void AddToInventory(item* i);
  void SetInventoryTile(int x, int y, item* i);
  item* GetFromInventory(int x, int y);  

  area* GetInventory();
 
  int ItemProc (WINDOW* winchoice, item* itm, int y, int x);
  int LockProc (WINDOW* winchoice, int door_y, int door_x, tile doortype, int doortile, std::string doorname );
  int DoorProc (WINDOW* winchoice, int y, int x, tile doortype);
 
  void TileProc(WINDOW* winchoice, int y, int x, tile t);
  int AreaProc(WINDOW* winchoice, int y, int x);

  int BattleTurn (WINDOW* main_win, WINDOW* input_win, int npc_id);
 
  void SetLoot(int x);
  int GetLootScore();
 
  int Input (WINDOW* winchoice, WINDOW* inv_wins[3][3], WINDOW* main_win);

  int LootCount ();
 
  Player (char c, int col, std::string name, int health) : Character(c,col,name,health)
    {
      //initialise the item inventory
      for (y=0; y<inv_y; y++)
	{
	  for (x=0; x<inv_x; x++)
	    {
	      inventory->AddItem(new item(item_library[no_item]));
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
 
class Goblin : public NPC
{
 public:

 Goblin() : NPC ('@',2,"Wild Goblin",30)
    {
      SetWeps (no_weapon,claw,sword);
      SetOutfit(outfit_library[goblin]);
    };

 Goblin(char c, int col, std::string name, int health) : NPC (c,col,name,health)
    {
      SetWeps (no_weapon,claw,sword);
      SetOutfit(outfit_library[goblin]);
    };
  
};

class HornyGoblin : public Goblin
{
 public:
  
 HornyGoblin() : Goblin ('@',1,"Horny Goblin",5) 
    {
      SetWeps (no_weapon,claw,axe);
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

void InitNpcs();
void DrawNPCS();

#endif

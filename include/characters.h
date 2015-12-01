#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <string>

#define INV_SIZE 3

#include "tile.h"
#include "items.h"
#include "containers.h"



class Character
{
 protected:
  int i;
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
  int Flee();
 
  void SetCharacter(char char_choice, int colour_choice, std::string name_choice, int health_choice);

  void GetPos (int *x, int *y);
  
  int GetColour();
  char GetSymbol();

  std::string GetName();
  int GetHealth ();

  void SetWeps(weaponIndex one, weaponIndex two, weaponIndex three);
  weapon* GetWeps(); //returns a pointer to a 1D array of weapons

  void SetOutfit(outfit o);
  outfit GetOutfit();
  
  void SetPos(int x, int y);
  void SetHealth(int);

  Container* DropItems(); //Drops the players inv on death as a dead body
   
  Character (char c, int col, std::string name, int health) 
    {
      i=0;
      y=0;
           
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
 private:
    NPC* npcs;
    
 public :
  int BattleTurn ();
  void Move(int* x, int* y); //returns chosen movement co-ordinates for the NPC

  NPC (char c, int col, std::string name, int health) : Character(c,col,name,health)
    {
      //this->SetPos(6,6);
    }; 
};

class Player : public Character
{
 private:

  Inventory* inventory;
  NPC* npcs;
  
  int loot;
  
  
  public :
  int DropItem(Item* thisItem, int invX, int invY);
  
  int AddToInventory(Item* i);
  void SetInventory(int index, Item* i);

  Inventory* GetInventory();
  Item* GetFromInventory(int index);
  
  void SetLoot(int x);
  int GetLootScore();

  int GetLootCount ();
  int GetKeyCount();
  void RemoveKeyCount(int keyCount);
  
  //int AreaProc (int x ,int y);
 
  Player (char c, int col, std::string name, int health) : Character(c,col,name,health)
    {
      //initialise the item inventory
      inventory = new Inventory();
      
	  for (i=0; i<INV_SIZE; i++)
	    {
	      inventory->AddItem(new Item(item_library[no_item]));
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

class GoblinBeserker : public Goblin
{
 public:
  
    GoblinBeserker() : Goblin ('@',1,"Goblin Beserker",5) 
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

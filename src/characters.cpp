#include <iostream>
#include <string>
#include <curses.h>

#include "characters.h"

using std::string;
using std::cout;

/** Commented out until I can fix the scope for it's implementation */ 
int Character :: Flee()
{
  //int thisX = this->x;
 // int thisY = this->y;
  
  /* Checks all 4 cardinal directions for a traversable tile to flee into
   */
  /*
  if (map->IsTraversable(thisX+1,y) )
    {
      SetPos(x+1,y);
      return 0;
    }
   
	else if (map->IsTraversable (thisX-1,y) ) 
    {
      SetPos(x-1,y);
      return 0;
    }
   
  else if (map->IsTraversable(x,thisY+1) ) 
    {
      SetPos(x,y+1);
      return 0;
    }
   
  else if (map->IsTraversable(x,thisY-1) ) 
    {
      SetPos(x,y-1);
      return 0;
    }
 
  else
    {
      return 1;
    }
  */
          
  return 1;
}

void Character :: SetCharacter (char char_choice, int colour_choice, std::string name_choice, int health_choice)
{
  alive = true;
  this->symbol = char_choice;
  this->colour = colour_choice;
  this->name = name_choice;
  this->health = health_choice;
  
  void SetPos(int x, int y);
}

bool Character :: IsAlive(){
    return alive;
}

void Character :: Kill()
{
  alive = false;
  DropItems();
}

int Character :: GetColour() {
    return colour;
}

char Character :: GetSymbol() {
    return symbol;
}

std::string Character :: GetName()
{
  return this->name;
} 

void Character :: GetPos (int* x, int* y)
{
  (*x = this->x);
  (*y = this->y);
}

int Character :: GetHealth ()
{
  return this->health;
}
  
void Character :: SetHealth(int i)
{
  this->health = i;
  return;
}

void Character :: SetPos(int x, int y)
{
      this->x = x;
      this->y = y;
}

/* Returns an area/dead body that contains the characters possesions*/
area* Character :: DropItems()
{
  string name = this->name.c_str();
  string thisName = "Dead " + name;
  area* body = new area(1,thisName,"X",1,0,true);
  
  body->AddItem(new weapon(this->weps[1]));
  body->AddItem(new weapon(this->weps[2]));

  //thisContainer->ReplaceItem(1,1,new outfit(this->currentOutfit));

  return body;
}
 
void Character :: SetWeps (weaponIndex one, weaponIndex two, weaponIndex three)
{  
  this->weps[0] = weapon_library[one];
  this->weps[1] = weapon_library[two];
  this->weps[2] = weapon_library[three];
}
  
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

/////////////////////////////////////////////////

int Player :: AddToInventory(item* i)
{
  return this->inventory->AddItem(i);
}

void Player :: SetInventoryTile(int x, int y, item* i)
{
  this->inventory->ReplaceItem(x,y,i); 
}

item* Player :: GetFromInventory(int x, int y)
{
  return this->inventory->GetItem(x,y);
}

area* Player :: GetInventory()
{
  return this->inventory;
}  


void Player :: SetLoot(int i)
{
  this->loot = i;
  return;
}

int Player :: GetLootScore ()
{
  return this->loot;
}

int Player :: LootCount ()
{
  int total = 0;

  for (int y = 0; y < INV_Y; y++)
    {
      for(int x = 0; x < INV_X; x++)
	{
	  int value = inventory->value;
	  total+=value;
	}
    }

  this->SetLoot(total);
  return (0);
}



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
 
}

//randomly moves within the 6 local directions
void NPC :: Move(int* x, int* y) {
        int xOffset = ((rand() % 3 - 1)); //generates a random number to offset current position by
        int yOffset = ((rand() % 3 - 1));
        
        int pos_x = this->x;
        int pos_y = this->y;
       
        //Sets the pointed-to values of the passed pointers to our new value
        (*x) = (pos_x+xOffset); 
        (*y) = (pos_y+yOffset);
        
                          
 }

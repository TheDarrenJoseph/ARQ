#ifndef ITEMS_H
#define ITEMS_H

#include <string>

//enums for reference to detail structs (for dynamic item lists, their size is controlled by the last enum)
enum itemIndex {no_item,lockpick,key,statue,gold_coin,silver_coin,bronze_coin,gold_bar,silver_bar,bronze_bar,item_size};
enum weaponIndex {no_weapon,fist,claw,acid,sword,axe,dagger,spear,weapon_size};
enum outfitIndex {no_outfit,warrior,goblin,outfit_size};

enum lootChance {noLoot,lowLoot,mediumLoot,highLoot};

struct Item
{
  int id;
  std::string name;
  const char* symbol;
  int colour;
  int value;
  bool lootable;

  Item()
  {
    id = 0;
    name = "None";
    symbol = " ";
    colour = 0;
    value = 0;
    lootable = false;
  }
   
  Item(int i, std::string n, const char* s, int col, int val, bool lootable)
  {
    id = i;
    name = n;
    symbol = s;
    colour = col;
    value = val;
    this->lootable = lootable;
  }
       
};
 
struct weapon : public Item
{
  int damage;
 
 weapon() : Item(0,"None"," ",0,0,false)
    {
      this->damage=0;
    }
   
 weapon(int i, std::string n, const char* s, int col, int val,bool lootable,int d) : Item(i,n,s,col,val,lootable)
    {
      damage = d;
    }
};
 
struct outfit : public Item
{
  int armourPoints;
    
 outfit() : Item(0,"None"," ",0,0,false)
    {
      this->armourPoints=0;
    }
   
 outfit(int i, std::string n, const char* s, int col, int val,bool lootable,int AP) : Item(i,n,s,col,val,lootable)
    {
      armourPoints = AP;
    }
};



bool IsLootable(Item* i);

bool CanDropItem(Item* thisItem);

const Item item_library[item_size] =
  { 
    // {id}{Name(12 cha)}{Symbol}{Col}{Val}{Lootable}
    {0,  "None"      , " ",    0,   0,   false},
    {1,  "Lockpick"  , "!",    2,   1,   true}, 
    {2,  "Door Key"  , "!",    2,   5,   true}, 
    {3,  "Statue"    , "$",    2,   500, true},
    {4,  "Gld Coin"  , "$",    2,   50,  true},
    {5,  "Silv Coin" , "$",    2,   25,  true},
    {6,  "BrnzCoin"  , "$",    2,   5,   true},
    {7,  "GoldBar"   , "$",    2,   100, true},
    {8,  "SilvrBar"  , "$",    2,   50,  true },
    {9,  "BrnzeBar"  , "$",    2,   25 , true},
  };

const weapon weapon_library[weapon_size] = 
  {
    // {id}{Name(12 cha)}{Symbol}{Col}{Val}{Lootable}{Dam}
    {0,"None"         ," "    ,1   ,0   ,false    ,0},
    {1,"Fist"         ,"W"    ,0   ,0   ,false    ,5},
    {2,"Claw"         ,"W"    ,0   ,0   ,false    ,7},
    {3,"Acid"         ,"W"    ,0   ,0   ,false    ,20},
    {4,"Sword"        ,"W"    ,2   ,0   ,true     ,15},
    {5,"Axe"          ,"W"    ,2   ,0   ,true     ,12},
    {6,"Dagger"       ,"W"    ,2   ,0   ,true     ,8},
    {7,"Spear"        ,"W"    ,2   ,0   ,true     ,10},
  };

const outfit outfit_library[outfit_size] =
  {
    // {id}{Name(12 cha)}   {Symbol}{Col}{Val}{Lootable}{AP}
    {0,"None"               ,"O"    ,0   ,0  ,false    ,0},
    {1,"Warrior Armour"     ,"O"    ,0   ,0  ,true    ,30},
    {2,"Crude Goblin Armour","O"    ,0   ,0  ,true    ,20},
  };

#endif

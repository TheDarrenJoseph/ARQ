#ifndef ITEMS_H
#define ITEMS_H

#include <string>

//enums for reference to detail structs (for dynamic item lists, their size is controlled by the last enum)
enum itemIndex {no_item,lockpick,key,statue,gold_coin,silver_coin,bronze_coin,gold_bar,silver_bar,bronze_bar,item_size};
enum weaponIndex {no_weapon,fist,claw,acid,sword,axe,dagger,spear,weapon_size};
enum outfitIndex {no_outfit,warrior,goblin,outfit_size};

enum lootChance {noLoot,lowLoot,mediumLoot,highLoot};

enum itemType{ITEM,CONTAINER};

struct Item
{ 
  private:
    int id=0;
    std::string name="";
    char symbol= ' ';
    int colour=0;
    int weight=0;
    int value=0;
    bool lootable=false;
  
  public:
    static  long long int latestId;
    virtual itemType getType() const { return ITEM; } 
    virtual long long int GetId() const { return id; } 
    virtual std::string GetName() const { return name; } 
    void SetName(std::string name);
    virtual char GetSymbol() const { return symbol; } 
    virtual int GetColour() const { return colour; } 
    virtual int GetWeight() const { return weight; } 
    virtual int GetValue() const { return value; } 
    virtual bool IsLootable() { return lootable; } 

  bool IsLootable() const {
    return this -> lootable;
  }

  Item()
  {
    id = latestId++;
    name = "None";
    symbol = ' ';
    colour = 0;
    weight = 0;
    value = 0;
    lootable = false;
  }

  Item(Item* toCopy)
  {
    this -> id = toCopy -> id;
    this -> name = toCopy -> name;
    this -> symbol = toCopy -> symbol;
    this -> colour = toCopy ->  colour;
    this -> weight = toCopy -> weight;
    this -> value = toCopy -> value;
    this -> lootable = toCopy -> lootable;
  }

   
  Item(int id, std::string name, const char* symbol, int colour, int weight, int value, bool lootable)
  {
    this -> id = id;
    this -> name = name;
    this -> symbol = symbol[0];
    this -> colour = colour;
    this -> weight = weight;
    this -> value = value;
    this -> lootable = lootable;
  }
  
  virtual ~Item() {
      
  }
       
};
 
struct weapon : public Item
{
  int damage=0;
 
 weapon() : Item(0,"None"," ",0,0,0,false)
    {
      this->damage=0;
    }
   
 weapon(int i, std::string n, const char* s, int col, int weight, int val,bool lootable,int d) : Item(i,n,s,col,weight,val,lootable)
    {
      damage = d;
    }
};
 
struct outfit : public Item
{
  int armourPoints=0;
    
 outfit() : Item(0,"None"," ",0,0,0,false)
    {
      this->armourPoints=0;
    }
   
 outfit(int i, std::string n, const char* s, int col,int weight, int val,bool lootable,int AP) : Item(i,n,s,col,weight,val,lootable)
    {
      armourPoints = AP;
    }
};


class item_library_t {
  public:
    Item item_library[item_size] =
    { 
      // {id}{Name(12 cha)}{Symbol}{Col}{Weighy}{Val}{Lootable}
      {0,  "None"      , " ",      0,   0,       0,   false},
      {1,  "Lockpick"  , "!",      2,   1,       1,   true}, 
      {2,  "Door Key"  , "!",      2,   2,       5,   true}, 
      {3,  "Statue"    , "$",      2,   4,       500, true},
      {4,  "Gld Coin"  , "$",      2,   2,       50,  true},
      {5,  "Silv Coin" , "$",      2,   2,       25,  true},
      {6,  "BrnzCoin"  , "$",      2,   2,       5,   true},
      {7,  "GoldBar"   , "$",      2,   2,       100, true},
      {8,  "SilvrBar"  , "$",      2,   2,       50,  true },
      {9,  "BrnzeBar"  , "$",      2,   2,       25 , true},
    };

    weapon weapon_library[weapon_size] = 
    {
      // {id}{Name(12 cha)}{Symbol}{Col}{Weight}{Val}{Lootable}{Dam}
      {0,"None"         ," "       ,1   ,0      ,0   ,false    ,0},
      {1,"Fist"         ,"W"       ,0   ,0      ,0   ,false    ,5},
      {2,"Claw"         ,"W"       ,0   ,0      ,0   ,false    ,7},
      {3,"Acid"         ,"W"       ,0   ,0      ,0   ,false    ,20},
      {4,"Sword"        ,"W"       ,2   ,4      ,0   ,true     ,15},
      {5,"Axe"          ,"W"       ,2   ,4      ,0   ,true     ,12},
      {6,"Dagger"       ,"W"       ,2   ,2      ,0   ,true     ,8},
      {7,"Spear"        ,"W"       ,2   ,3      ,0   ,true     ,10},
    };

    outfit outfit_library[outfit_size] =
    {
      // {id}{Name(12 cha)}   {Symbol}{Col}{weight}{Val}{Lootable}{AP}
      {0,"None"               ,"O"    ,0  ,0       ,0  ,false    ,0},
      {1,"Warrior Armour"     ,"O"    ,0  ,3       ,0  ,true    ,30},
      {2,"Crude Goblin Armour","O"    ,0  ,1       ,0  ,true    ,20},
    };
};

extern item_library_t ItemLibrary;

#endif

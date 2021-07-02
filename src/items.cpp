#include "items.h"

 long long int Item::latestId = 0;

void Item::SetName(std::string name) { 
  this -> name = name; 
} 

item_library_t ItemLibrary;

/**
class ItemLibrary{
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
};**/

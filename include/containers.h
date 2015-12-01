#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <string>
#include <list>
#include "items.h"


//enum containerIndex {no_container,body,chest,container_size};

//Essentially an item type that stores other items
//Container inventory, only allows items
//id 99 to denote container
struct Container : public Item
{  
  std::list<Item*> inv;

  Item* GetInv();

  std::list<Item*>::iterator indexToIterator(int i);
  int AddItem(Item* i);
  void ReplaceItem(int it,Item* i);
  
  void RemoveItem(int i);
 
  Item* GetItem(int i);
  
  int HasItems();
 
  Container() : Item (99,"None"," ",0,0,false)
    {
      id = 99;
    };
  
  Container(int i, std::string n, const char* s, int col, int val,bool lootable) : Item(i,n,s,col,val,lootable)
    {
    };

};

//A special type to represent rooms and inventories.
struct Inventory 
{
  std::list<Item*>::iterator indexToIterator(int i);  
  std::list<Item*> inv; //Use the supertype Item so that we can polymorphically store Containers, etc

  int AddItem(Item* i);
  void ReplaceItem(int i,Item* item);
  void RemoveItem(int i);
  Item* GetItem(int index);
  
  int HasItems();
 
  Inventory()
    {
      
    };  
};


/*
  const container container_library[container_size]=
  {
  // {id}{Name(12 cha)}{Symbol}{Col}{Val}{Lootable}
  {0,  "None"      , " ",    0,   0,   false},
  {1,  "Dead Body" , "X",    1,   0,   true}, 
  {2,  "Chest"     , "C",    2,   0,   true}, 
  };
*/

#endif

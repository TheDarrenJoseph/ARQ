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
    
    private:
        std::list<Item*>::iterator indexToIterator(long unsigned int i);

    public:
  std::list<Item*> inv;

  Item* GetInv();

  
  int AddItem(Item* i);
  void ReplaceItem(long unsigned int it,Item* i);
  
  void RemoveItem(long unsigned int i);
 
  Item* GetItem(long unsigned int i);
  long unsigned int GetSize();
  
  int HasItems();
 
  Container() : Item (99,"None"," ",0,0,0,false)
    {
      id = 99;
    };
  
  Container(int i, std::string n, const char* s, int col, int weight, int val,bool lootable) : Item(i,n,s,col, weight, val,lootable)
    {

    };

};

//A special type to represent rooms and inventories.
//struct Inventory 
//{
//  std::list<Item*>::iterator indexToIterator(int i);  
//  std::list<Item*> inv; //Use the supertype Item so that we can polymorphically store Containers, etc
//
//  int AddItem(Item* i);
//  void ReplaceItem(int i,Item* item);
//  void RemoveItem(int i);
//  Item* GetItem(int index);
//  int GetSize();
//  
//  int HasItems();
// 
//  Inventory()
//    {
//      
//    };  
//};


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

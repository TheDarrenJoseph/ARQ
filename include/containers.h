#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <algorithm>
#include <string>
#include <list>
#include "items.h"
#include "logging.h"

enum ContainerType{OBJECT,AREA};

//enum containerIndex {no_container,body,chest,container_size};

//Essentially an item type that stores other items
//Container inventory, only allows items
struct Container : public Item
{  
    
  private:
    ContainerType containerType;
    int weightLimit = 0;
    Logging* logging = &logging -> getInstance();
    std::list<Item*>::iterator indexToIterator(long int i);
    bool sizeCheck(long int i);

  public:
  std::list<Item*> inv = std::list<Item*>();

  ContainerType GetContainerType();

  Item* GetInv();
  
 
  void AddItem(Item* i);
 // void AddItem(Item* i);
  void ReplaceItem(long int it, Item* i);
  
  
  void RemoveItem(long int i);
  void  RemoveItem(Item* item);
 
  Item* GetItem(long int i);
  long int GetSize();
  long int GetWeightLimit();
  
  bool IsOpenable();
  bool IsImpassable();

  int HasItems();

  int GetTotalWeight();
  int GetTotalLootScore();
  
  itemType getType()  {
      return CONTAINER;
  }
  
  Container() : Container (latestId++, AREA, "Floor","$",0,0,100,0,true)
    {
      
    };
  
  Container(int id, ContainerType containerType, std::string name, const char* symbol, int colour, int weight, int weightLimit, int value, bool lootable) : Item(id, name, symbol, colour, weight, value, lootable)
    {
      this -> containerType = containerType;
      this -> weightLimit = weightLimit;
    };

  Container(Container* toCopy)  : Item (toCopy)
    {
      this -> containerType = toCopy -> containerType;
      this -> weightLimit = toCopy -> weightLimit;
    };
    
  virtual ~Container() {
//      while (!inv.empty()) {
//          delete(inv.front());
//      }
          
  }

};



#endif

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <algorithm>
#include <string>
#include <vector>
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
    std::vector<Item*>::iterator indexToIterator(long int i);
    bool sizeCheck(long int i);

  public:
  std::vector<Item*> inv = std::vector<Item*>();

  ContainerType GetContainerType();

  Item* GetInv();
  
  long int IndexOf(Item* item);
  void AddItem(Item* i);
  void AddItem(Item* i, long int index);
  void AddItems(std::vector<Item*> items, long int index);

  void ReplaceItem(long int it, Item* i);

  void RemoveItem(long int i);
  void RemoveItem(Item* item);
  void RemoveItems(std::vector<Item*> items);
 
  std::vector<Container*> GetAllContainers() {
    std::vector<Container*> allContainers = this -> GetChildContainers();
    allContainers.push_back(this);
    return allContainers;
  }

  std::vector<Container*> GetChildContainers() {
    std::vector<Container*> containers = std::vector<Container*>();
    for (std::vector<Item*>::iterator it = inv.begin(); it < inv.end(); it++) {
       Item* item = *it;
       Container* container = dynamic_cast<Container*> (item);
       if (container != NULL) {
         containers.push_back(container);
         std::vector<Container*> childContainers = container -> GetChildContainers();
         for (std::vector<Container*>::iterator childContainerIt = childContainers.begin(); childContainerIt < childContainers.end(); childContainerIt++) {
           containers.push_back(*childContainerIt);
         }
       }
    }
    return containers;
  }

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
  
  bool operator==(const Container& other) {
    return this -> GetId() == other.GetId();
  }

  Container() : Container (AREA, "Floor","$",0,0,100,0,true)
    {
      
    };
  
  Container(ContainerType containerType, std::string name, const char* symbol, int colour, int weight, int weightLimit, int value, bool lootable) : Item(name, symbol, colour, weight, value, lootable)
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

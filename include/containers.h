#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <string>
#include <list>
#include "items.h"
#include "logging.h"


//enum containerIndex {no_container,body,chest,container_size};

//Essentially an item type that stores other items
//Container inventory, only allows items
struct Container : public Item
{  
    
  private:
    Logging* logging = &logging -> getInstance();
    std::list<const Item*>::iterator indexToIterator(long unsigned int i);
    bool sizeCheck(long unsigned int i);

  public:
  std::list<const Item*> inv = std::list<const Item*>();

  Item* GetInv();
  
 
  void AddItem(const Item* i);
 // void AddItem(Item* i);
  void ReplaceItem(long unsigned int it,const Item* i);
  
  
  void RemoveItem(long unsigned int i);
 
  const Item* GetItem(long unsigned int i);
  long unsigned int GetSize();
  
  int HasItems();

  int GetTotalLootScore();
  
  itemType getType()  {
      return CONTAINER;
  }
  
  Container() : Container (99,"None"," ",0,0,0,false)
    {
      
    };
  
  Container(int id, std::string name, const char* symbol, int colour, int weight, int value, bool lootable) : Item(id, name, symbol, colour, weight, value, lootable)
    {
     
    };

  Container(const Container* toCopy)  : Item (toCopy)
    {
      
    };
    
  virtual ~Container() {
//      while (!inv.empty()) {
//          delete(inv.front());
//      }
          
  }

};



#endif

#include "containers.h"

std::list<Item*>::iterator Container :: indexToIterator(int i) {
    
    if (i>0 && (long unsigned int)i<inv.size()) { 
        std::list<Item*>::iterator it = inv.begin();
        
        for (long unsigned int i=0; i<inv.size(); it++){} //Iterate
        
        return it;
        
    }
    
    return inv.end();
}

std::list<Item*>::iterator Inventory :: indexToIterator(int i) {
    
    if (i>0 && (long unsigned int)i<inv.size()) { 
        std::list<Item*>::iterator it = inv.begin();
        
        for (long unsigned int i=0; i<inv.size(); it++){} //Iterate
        
        return it;
        
    }
    
    return inv.end();
}

int Container :: AddItem(Item* i) {
  inv.push_front(i);
  return 0;
}

void Container :: ReplaceItem(int i, Item* item) {
    if (i>0 && (long unsigned int)i<inv.size()) {
    (*indexToIterator(i)) = item;
   }
}

void Container :: RemoveItem(int i) {
      //inv[index] = new invItem(inv_item_library[no_item]);
   
    if (i>0 && (long unsigned int)i<inv.size()) {
       
       inv.erase(indexToIterator(i));
   }

}  

Item* Container :: GetItem(int i) {
     return (*indexToIterator(i));
}

//Returns an integer to determine the contents of this area
//Returns 1 for multiple items, and 2 for one
int Container :: HasItems()
{
    long unsigned int count = inv.size();
 
  switch (count)
    {
      //if no items, return 0
    case (0) :
      return 0;
      break;
      
      //if only 1 item, return 1 for standard item
    case (1) :
      return 1;
      break;

      //otherwise, return 2;
    default :
      return 2;
      break;
    }
}

Item* Inventory :: GetItem(int i) {
     return (*indexToIterator(i));
}


int Inventory :: AddItem(Item* item)
{
  inv.push_front(item);
  return 0;
}

void Inventory :: ReplaceItem(int i, Item* item) {
   if (i>0 && (long unsigned int)i<inv.size()) {
    (*indexToIterator(i)) = item;
   }
}

void Inventory :: RemoveItem(int i)
{
  if (i>0 && (long unsigned int)i<inv.size()) {  
      inv.erase(indexToIterator(i));
  }
}




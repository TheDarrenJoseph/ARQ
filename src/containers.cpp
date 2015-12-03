#include "containers.h"
#include <iostream>

/** Creates an iterator for this container's inv, and moves it to the needed position.
 * DO NOT CALL THIS FUNCTION UNLESS THE INV CONTAINS SOMETHING
 * 
 * @param i
 * @return 
 */
std::list<Item*>::iterator Container :: indexToIterator(long unsigned i) {
    
    if (i>0 && i<inv.size()) { 
        std::list<Item*>::iterator it = inv.begin();
        
        for (long unsigned int i=0; i<inv.size(); it++){} //Iterate
        return it;
    } else {
        return inv.begin();
    }
}

long unsigned int Container :: GetSize() {
    return inv.size();
}

//std::list<Item*>::iterator Inventory :: indexToIterator(int i) {
//    
//    if (i>0 && (long unsigned int)i<inv.size()) { 
//        std::list<Item*>::iterator it = inv.begin();
//        
//        for (long unsigned int i=0; i<inv.size(); it++){} //Iterate
//        
//        return it;
//        
//    }
//    
//    return inv.end();
//}

int Container :: AddItem(Item* i) {
  inv.push_front(i);
  return 0;
}

void Container :: ReplaceItem(long unsigned int i, Item* item) {
    if (i>0 && (long unsigned int)i<inv.size()) {
    (*indexToIterator(i)) = item;
   }
}

void Container :: RemoveItem(long unsigned int i) {
      //inv[index] = new invItem(inv_item_library[no_item]);
   
    if (i>0 && (long unsigned int)i<inv.size()) {
       inv.erase(indexToIterator(i));
   }

}  

Item* Container :: GetItem(long unsigned int i) {
    //std::cout << "ui int" << (unsigned int) i;
   // return (Item*) &item_library[0];
    
    if (i>0 && (unsigned int)i<inv.size()) {
        return (*indexToIterator(i));
    } else {
        return (Item*) &item_library[0];
    }
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





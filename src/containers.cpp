#include "containers.h"
#include <iostream>

/** Creates an iterator for this container's inv, and moves it to the needed position.
 * DO NOT CALL THIS FUNCTION UNLESS THE INV CONTAINS SOMETHING!!
 * 
 * @param i
 * @return 
 */
std::list<const Item*>::iterator Container :: indexToIterator(long unsigned i) {
    
    if (i<inv.size()) { 
        std::list<const Item*>::iterator it = inv.begin();
        
        for (long unsigned int index=0; index<i ; index++){
            it++;
        } //Iterate
        return it;
        
    } else {
        return inv.begin();
    }
}

long unsigned int Container :: GetSize() {
    return inv.size();
}

bool Container :: sizeCheck(long unsigned int i) {
    return ((!inv.empty()) && i<inv.size());
}

void Container :: AddItem(const Item* i) {
  //inv.push_front(i);
  //inv.push_back(i);
  inv.push_back(i);
}

//void Container :: AddItem(Item* i) {
//  //inv.push_front(i);
//  //inv.push_back(i);
//  inv.push_back(i);
//}

void Container :: ReplaceItem(long unsigned int i, const Item* item) {
    if (sizeCheck(i)) {
    *indexToIterator(i) = item;
   }
}

void Container :: RemoveItem(long unsigned int i) {
    if (sizeCheck(i)) {
       inv.erase(indexToIterator(i));
   }

}  

const Item* Container :: GetItem(long unsigned int i) {
  
   if(sizeCheck(i)) { //Boundary check
      return (*indexToIterator(i)); //&* to deal with how iterators work
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

int Container :: GetTotalLootScore() {
  int totalScore = 0;
  for (const Item* itemPtr : inv) {
    int value = 0;
    itemType type = itemPtr -> getType();
    switch (type) {
      case ITEM:
        value = itemPtr -> GetValue();
        break;
      case CONTAINER:
        Container* container = (Container*) itemPtr;
        value = container -> GetTotalLootScore();
        break;
    }
    totalScore += value;
    logging -> logline("Tallying item value for a " + itemPtr -> GetName() + " : " + std::to_string(value));
  }
  return totalScore;
}





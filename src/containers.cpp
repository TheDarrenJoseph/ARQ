#include "containers.h"
#include <iostream>


long int Container :: IndexOf(Item* item) {
  std::vector<Item*>::iterator iter = std::find(inv.begin(), inv.end(), item);
  if (iter != inv.end()) {
	int index = std::distance(inv.begin(), iter);
    logging -> logline("Found item: " + item -> GetName() + ". Index: " + std::to_string(index));
    return index;
  }
  logging -> logline("Failed to find item: " + item -> GetName());
  return -1;
}

/** Creates an iterator for this container's inv, and moves it to the needed position.
 * DO NOT CALL THIS FUNCTION UNLESS THE INV CONTAINS SOMETHING!!
 * 
 */
std::vector<Item*>::iterator Container :: indexToIterator(long  i) {
    
    if (i >=0 && ((long unsigned int) i) < inv.size()) { 
        std::vector<Item*>::iterator it = inv.begin();
        
        for (long int index=0; index<i ; index++){
            it++;
        } //Iterate
        return it;
        
    } else {
    	logging -> logline("Invalid index: " + std::to_string(i) + " Returning end of container.");
        return inv.end();
    }
}

ContainerType Container :: GetContainerType() {
  return this -> containerType;
}


long int Container :: GetSize() {
    return inv.size();
}


long int Container :: GetWeightLimit() {
    return this -> weightLimit;
}

bool Container :: sizeCheck(long int i) {
    return ((!inv.empty()) && i >= 0 && ((long unsigned int) i) < inv.size());
}

void Container :: AddItem(Item* i) {
  inv.push_back(i);
}

void Container :: AddItem(Item* item, long int index) {
  std::vector<Item*>::iterator indexIt = indexToIterator(index);
  if (indexIt != inv.end()) {
	  inv.insert(indexIt, item);
  }
}

void Container :: AddItems(std::vector<Item*> items, long int index) {
  std::vector<Item*>::iterator indexIt = indexToIterator(index);
  if (indexIt != inv.end()) {
	  inv.insert(indexIt, items.begin(), items.end());
  }
}

//void Container :: AddItem(Item* i) {
//  //inv.push_front(i);
//  //inv.push_back(i);
//  inv.push_back(i);
//}

void Container :: ReplaceItem(long int i, Item* item) {
   if (sizeCheck(i)) {
	   *indexToIterator(i) = item;
   }
}

void Container :: RemoveItem(long int i) {
    if (sizeCheck(i)) {
       inv.erase(indexToIterator(i));
   }

}  

void Container :: RemoveItem(Item* item) {
  std::vector<Item*>::iterator iter = std::find(inv.begin(), inv.end(), item);
  if (iter != inv.end()) {
    logging -> logline("Removing item: " + item -> GetName());
    inv.erase(iter);
  }
}

void Container :: RemoveItems(std::vector<Item*> items) {
  std::vector<Item*>::iterator it = items.begin();
  for (long unsigned int i=0; i < items.size(); i++) {
	  if (it != items.end() && it != inv.end()) {
		  RemoveItem(*it);
	  }
  }
}


Item* Container :: GetItem(long int i) {
  
   if(sizeCheck(i)) { //Boundary check
      return (*indexToIterator(i)); //&* to deal with how iterators work
   } else { 
	  return NULL;
      //return (Item*) &ItemLibrary.item_library[0];
   }
    
}

bool Container :: IsOpenable() {
  if (this -> containerType == OBJECT) {
    return true;
  } else {
    return this -> HasItems() && this -> IsLootable();
  }
}

bool Container :: IsImpassable() {
  // We can always open container objects
  return (this -> containerType == OBJECT);
}

//Returns an integer to determine the contents of this area
//Returns 1 for multiple items, and 2 for one
int Container :: HasItems()
{
  long int count = inv.size();
 
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

int Container :: GetTotalWeight() {
  int totalWeight = 0;
  for (Item* itemPtr : inv) {
	if (itemPtr != NULL) {
		int weight = 0;
		itemType type = itemPtr -> getType();
		switch (type) {
		  case ITEM:
			weight = itemPtr -> GetWeight();
			break;
		  case CONTAINER:
			Container* container = (Container*) itemPtr;
			weight = container -> GetTotalWeight();
			break;
		}
		totalWeight += weight;
	}
  }
  return totalWeight;
}

int Container :: GetTotalLootScore() {
  int totalScore = 0;
  if (inv.empty()) return totalScore;
  for (Item* itemPtr : inv) {
	if (itemPtr != NULL) {
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
	}
    //logging -> logline("Tallying item value for a " + itemPtr -> GetName() + " : " + std::to_string(value));
  }
  return totalScore;
}





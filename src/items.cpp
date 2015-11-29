#include <iostream>

#include "map.h"
#include "items.h"

bool IsLootable(Item* i) { return i->lootable; };

bool CanDropItem(Item* thisItem)
{
  if (thisItem->lootable)
    {    
      return true;
    }
            
  else
    {
      return false;
    }
}


#include <iostream>

#include "map.h"
#include "items.h"

bool IsLootable(const Item* i) { return i->lootable; };

bool CanDropItem(const Item* thisItem)
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

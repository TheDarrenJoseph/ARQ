#include <iostream>

#include "grid.h"
#include "items.h"

bool IsLootable(item* i) { return i->lootable; };

bool CanDropItem(item* thisItem)
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


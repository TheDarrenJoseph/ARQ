#include <iostream>

#include "grid.h"
#include "items.h"

void GenerateItems(lootChance thisChance)
{
  for (int y = 0; y < grid_y; y++)
    {
      for (int x = 0; x < grid_x; x++)
	{
	  //add items to room tiles
	  if (GetTile(x,y) == rom)
	    {
	      //for each of the items in the library
	      for (int  i=0; i<item_size; i++)
		{
		  //generate a random chance out of 100 for each item
		  int chance = rand() %100+1;
	      
		  //convert the current index in the library to an item enum 
		  item thisItem = item_library[i];
	      
		  //if the random chance < the number of items (e.g 10, 10%)
		  if (chance<=thisChance)  
		    {
		      //then add it to the item grid
		      SetItem(x,y,new item(thisItem));
	        
		      //break the for loop
		      break; 
		    }
		}
	    
	      /*
	      //spawn each weapon based on a chance out of 100
	      for (int  i=0; i<weapon_size; i++)
	      {
	      weapon thisWeapon = weapon_library[i];
	      
	      int chance = rand() %100+1;
	      
	      //use the number of weapons as the chance of appearing
	      
	      if (chance<=weapon_size && IsLootable(thisWeapon)) 
	      {
	      item_grid[y][x] = new weapon(thisWeapon);
	      cout << thisWeapon.name;
	      break;
	      };
	      }
	      */
	  
	    }
																	
	  else
	    {
	      SetItem(x,y,new item(item_library[no_item]));
	    }
	}
    }
   
  std::cout << "Items generated\n"; 
  return;
}
 
bool IsLootable(item* i)
{
  if (i->lootable == true)
    {
      return true;
    }
  
  else
    {
      return false;
    };
};


#include "containers.h"

void container :: InitialiseInv()
{
  for (int x=0;x<3;x++)
    {
      for (int y=0;y<3;y++)
	{
	  this->inv[y][x] = new invItem(inv_item_library[no_item]);
	}
    }
}

int container :: AddItem(item* i)
{
  invItem* thisItem = ToInvItem(i);

  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  if (inv[y][x]->name == inv_item_library[no_item].name)
	    {
	      inv[y][x] = thisItem;
	      return 0;
	    }
	}
    }
  return 1;
}

void container :: ReplaceItem(int x, int y, item* i)
{
  invItem* thisItem = ToInvItem(i);

  inv[y][x] = thisItem;
}

void container :: RemoveItem(int x, int y)
{
  inv[y][x] = new invItem(inv_item_library[no_item]);
}

item* container :: GetItem(int x, int y)
{
  item* i = ToItem(inv[y][x]);
  return i;
}

void area :: InitialiseInv()
{
  for (int x=0;x<3;x++)
    {
      for (int y=0;y<3;y++)
	{
	  this->inv[y][x] = new item(item_library[no_item]);
	}
    }
}

int area :: AddItem(item* i)
{
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  if (inv[y][x]->name == item_library[no_item].name)
	    {
	      inv[y][x] = i;
	      return 0;
	    }
	}
    }
  return 1;
}

void area :: ReplaceItem(int x, int y, item* i)
{
  inv[y][x] = i;
}

void area :: RemoveItem(int x, int y)
{
  inv[y][x] = new item(item_library[no_item]);
}

item* area :: GetItem(int x, int y)
{
  return inv[y][x];
}

//Returns an integer to determine the contents of this area
//Returns 1 for multiple items, and 2 for one
int area :: HasItems()
{
  int count = 0;

  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  if (inv[y][x]->id != 0)
	    {
	      count = count+1;
	    }
	}
    }

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

item* ToItem(invItem* i)
{
  int id = i->id;
  std::string name = i->name;
  const char* symbol = i->symbol;
  int colour = i->colour;
  int value=i->value;
  bool lootable=i->lootable;
  
  item* thisItem = new item (id,name,symbol,colour,value,lootable);
  
  return thisItem;
} 

invItem* ToInvItem(item* i)
{
  int id = i->id;
  std::string name = i->name;
  const char* symbol = i->symbol;
  int colour = i->colour;
  int value=i->value;
  bool lootable=i->lootable;
  
  invItem* thisItem = new invItem (id,name,symbol,colour,value,lootable);
  
  return thisItem;
} 
 

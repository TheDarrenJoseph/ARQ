#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <string>
#include "items.h"


//enum containerIndex {no_container,body,chest,container_size};

//id 99 to denote container
struct Container : public Item
{
  
  invItem* inv[3][3];
   
  void InitialiseInv();

  invItem* GetInv();

  int AddItem(Item* i);
  void ReplaceItem(int x, int y,Item* i);
  
  void RemoveItem(int x, int y);
 
  Item* GetItem(int x,int y);
 
  Container() : Item (99,"None"," ",0,0,false)
    {
      InitialiseInv();
      id = 99;
    };
  
  Container(int i, std::string n, const char* s, int col, int val,bool lootable) : Item(i,n,s,col,val,lootable)
    {
      InitialiseInv();
    };

};

//id 98 to denote area
struct Area : public Container
{
<<<<<<< 4afa103590b178c44c1f6c1f01aaf35e405f453f
  item* inv[3][3];
=======

  Item* inv[3][3];
>>>>>>> Branch work pre-header fix branch
  
  void InitialiseInv();

  Item* GetInv();

  int AddItem(Item* i);
  void ReplaceItem(int x, int y,Item* i);
  
  void RemoveItem(int x, int y);
 
  Item* GetItem(int x, int y);
  
  int HasItems();
 
  Area(int i, std::string n, const char* s, int col, int val,bool lootable) : Container(i,n,s,col,val,lootable)
    {
      InitialiseInv();
      id = 98;
    };  
};

Item* ToItem(invItem* i);
invItem* ToInvItem(Item* i);

/*
  const container container_library[container_size]=
  {
  // {id}{Name(12 cha)}{Symbol}{Col}{Val}{Lootable}
  {0,  "None"      , " ",    0,   0,   false},
  {1,  "Dead Body" , "X",    1,   0,   true}, 
  {2,  "Chest"     , "C",    2,   0,   true}, 
  };
*/

#endif

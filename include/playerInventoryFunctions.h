#ifndef PLAYER_INV_FUNCS_H
#define PLAYER_INV_FUNCS_H

class InventoryFunctions {
  public:
  virtual int TakeItem(Container* container, int index) = 0;
  virtual int DropPlayerItem(const Item* thisItem) = 0;
  virtual int MoveItem(Item* item, Item* targetItem) = 0;

  virtual ~InventoryFunctions() {}
};

#endif

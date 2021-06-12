#ifndef PLAYER_INV_FUNCS_H
#define PLAYER_INV_FUNCS_H

class InventoryFunctions {
  public:
  virtual int DropPlayerItem(const Item* thisItem) = 0;
  virtual int MoveContainer(Container* container) = 0;

  virtual ~InventoryFunctions() {}
};

#endif

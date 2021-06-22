#ifndef CONTAINER_SELECTION_H
#define CONTAINER_SELECTION_H

#include <curses.h>  
#include "containers.h"

class ContainerSelection {
  private:
    Logging* logging = &logging -> getInstance();
    long unsigned int previousSelectionIndex = 0;
    long unsigned int selectionIndex = 0;
    long unsigned int invStartIndex = 0; //The index of the topmost item on the screen, alows scrolling
    long unsigned int containerIndex = selectionIndex + invStartIndex;
    bool redrawList = false;
    Container* playerInventory;
    Container* container;
    Item* movingItem;
    Item* selectedItem;
    unsigned int itemViewLineCount;

  public:
    void HandleSelection(int choice);

    long unsigned int GetPreviousSelectionIndex() {
      return this -> previousSelectionIndex;
    }

    long unsigned int GetSelectionIndex() {
      return this -> selectionIndex;
    }

    long unsigned int GetInvStartIndex() {
      return this -> invStartIndex;
    }

    long unsigned int GetContainerIndex() {
      return this -> containerIndex;
    }

    Container* GetContainer() {
      return this -> container;
    }

    Item* GetMovingItem() {
      return this -> movingItem;
    }

    void SetMovingItem(Item* movingItem) {
      this -> movingItem = movingItem;
    }

    bool IsRedrawList() {
      return this -> redrawList;
    }

    ContainerSelection(Container* container, const unsigned int itemViewLineCount) {
      this -> container = container;
      this -> itemViewLineCount = itemViewLineCount;
    }
};

#endif

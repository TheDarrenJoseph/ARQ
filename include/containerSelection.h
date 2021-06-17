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
    bool redrawList = false;
    Container* container;
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

    bool IsRedrawList() {
      return this -> invStartIndex;
    }

    ContainerSelection(Container* container, const unsigned int itemViewLineCount) {
      this -> container = container;
      this -> itemViewLineCount = itemViewLineCount;
    }
};

#endif

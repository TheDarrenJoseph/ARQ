#include "itemListSelection.h"

void ItemListSelection :: CheckSelectingItemsAbove() {
  bool selectingItemsAbove = this -> containerIndex < previousContainerIndex && this -> containerIndex < containerSelectionStart;
  if (selectingItemsAbove) {
    for (long int i=this -> containerIndex; i <= containerSelectionStart; i++) {
      Select(i);
    }
  }
}

void ItemListSelection :: CheckReducingSelectionAbove() {
  // Deselect anything we were previously selecting
  bool reducingSelectionAbove = this -> containerIndex > previousContainerIndex && this -> containerIndex < containerSelectionStart;
  bool selectedStartPosition = containerIndex == containerSelectionStart;
  if (reducingSelectionAbove || selectedStartPosition) {
    for (long int i=previousContainerIndex-1; i < containerIndex; i++) {
      Deselect(i);
    }
  }
}

void ItemListSelection :: CheckSelectingItemsBelow() {
  bool selectingItemsBelow = this -> containerIndex > previousContainerIndex && this -> containerIndex > containerSelectionStart;
  if (selectingItemsBelow) {
    for (long int i=this -> containerSelectionStart; i <= containerIndex; i++) {
      Select(i);
    }
  }
}

void ItemListSelection :: CheckReducingSelectionBelow() {
  // Deselect anything we were previously selecting
  bool reducingSelectionBelow = this -> containerIndex < previousContainerIndex && this -> containerIndex > containerSelectionStart;
  bool selectedStartPosition = containerIndex == containerSelectionStart;
  if (reducingSelectionBelow || selectedStartPosition) {
    for (long int i= this -> containerIndex+1; i <= previousContainerIndex; i++) {
      Deselect(i);
    }
  }
}

void ItemListSelection :: UpdateSelectionIndices(long int newSelectionIndex) {
  this -> previousSelectionIndex = this -> selectionIndex;
  this -> previousContainerIndex = this -> containerIndex;
  this -> selectionIndex = newSelectionIndex;
  this -> containerIndex = newSelectionIndex + this -> invStartIndex;

  if (this -> selectingItems && this -> containerSelectionStart == -1) {
    // Set the selection pivot point
    this -> containerSelectionStart = newSelectionIndex + invStartIndex;
  }
}

void ItemListSelection :: UpdateSelection(long int newSelectionIndex) {
    this -> UpdateSelectionIndices(newSelectionIndex);
    bool selectionChanged = this -> containerIndex != previousContainerIndex;
    if (selectionChanged && this -> selectingItems) {
      logging -> logline("Selection starts at: " + std::to_string(containerSelectionStart));
      this -> CheckSelectingItemsAbove();
      this -> CheckReducingSelectionAbove();
      this -> CheckSelectingItemsBelow();
      this -> CheckReducingSelectionBelow();
    }
}

std::vector<Container*> ItemListSelection :: GetSelectedContainers() {
  std::vector<Container*> containers;
  if (!this -> selectedItems.empty()) {
    for(std::vector<Item*>::iterator it = selectedItems.begin(); it != selectedItems.end(); it++) {
       Item* item = *it;
       Container* container = dynamic_cast<Container*> (item);
       if (container != NULL) {
         containers.push_back(container);
       }
    }
  }
  return containers;
}

void ItemListSelection :: ChooseItem() {
  if (!this -> selectingItems) {
    // If we're hitting select on a selected item, but not in select mode, deselect it
    if (this -> IsSelected(containerIndex)) {
      this -> Deselect(containerIndex);
      return;
    }

    this -> selectingItems = true;
    this -> containerSelectionStart = -1;
    logging -> logline("Selecting items");
  } else {
    this -> selectingItems = false;
    logging -> logline("Closing selection");
  }
}

long int  ItemListSelection :: PageUp() {
  long int newSelectionIndex = this -> selectionIndex;
  int maxSelectionIndex = this -> DetermineMaxSelectionIndex();
  // Either page up or jump to top of current page
  if (selectionIndex==0 && invStartIndex >= itemViewLineCount) {
      newSelectionIndex = maxSelectionIndex;
      invStartIndex -= itemViewLineCount;
      if (this -> selectingItems) SelectRange(newSelectionIndex, selectionIndex);
      this -> redrawList = true;
  } else if (selectionIndex == 0) {
      newSelectionIndex = 0;
      invStartIndex = 0;
      this -> redrawList = true;
  } else {
    newSelectionIndex = 0;
  }
  return newSelectionIndex;
}

long int ItemListSelection :: PageDown() {
  long int newSelectionIndex = this -> selectionIndex;
  int maxScrollIndex = this -> DetermineMaxScrollIndex();
  int maxSelectionIndex = this -> DetermineMaxSelectionIndex();
  // Either page down or jump to bottom of current page
  if (selectionIndex == maxSelectionIndex && invStartIndex+itemViewLineCount <= maxScrollIndex) {
      newSelectionIndex = 0;
      invStartIndex += itemViewLineCount;
      this -> redrawList = true;
  } else if (selectionIndex == maxSelectionIndex) {
      invStartIndex = maxScrollIndex;
      this -> redrawList = true;
  } else {
    return maxSelectionIndex;
  }
  return newSelectionIndex;
}

long int ItemListSelection :: MoveUp() {
  if (selectionIndex>0) {
    return selectionIndex - 1;
  } else if (selectionIndex==0 && invStartIndex>0) {
      this -> invStartIndex--;
      this -> redrawList = true;
  }
  return selectionIndex;
}

long int ItemListSelection :: MoveDown() {
  int maxScrollIndex = this -> DetermineMaxScrollIndex();
  int maxSelectionIndex = this -> DetermineMaxSelectionIndex();
  if (selectionIndex < maxSelectionIndex) {
    return selectionIndex + 1;
  } else if (selectionIndex == itemViewLineCount-1 && invStartIndex < maxScrollIndex) {
    this -> invStartIndex++;
    this -> redrawList = true;
  }
  return selectionIndex;
}


int ItemListSelection :: DetermineMaxScrollIndex() {
  int containerSize = this -> items.size();
  int maxScrollIndex = 0;
  if (containerSize > itemViewLineCount) maxScrollIndex = (containerSize - this -> itemViewLineCount);
  logging -> logline("max scroll index: " + std::to_string(maxScrollIndex));
  return maxScrollIndex;
}

int ItemListSelection :: DetermineMaxSelectionIndex() {
  int containerSize = this -> items.size();
  int maxSelectionIndex = this -> itemViewLineCount-1;
  if (containerSize < this -> itemViewLineCount) maxSelectionIndex = this -> itemViewLineCount - (this -> itemViewLineCount - containerSize) - 1;
  logging -> logline("max selection index: " + std::to_string(maxSelectionIndex));
  return maxSelectionIndex;
}

void ItemListSelection :: HandleSelection(int choice) {
  int containerSize = this -> items.size();
  if (containerSize > 0) {
    long int newSelectionIndex = this -> selectionIndex;
    switch (choice) {
      case('\n'): {
        this -> ChooseItem();
        break;
      }
      case KEY_PPAGE:
        newSelectionIndex = this -> PageUp();
        logging -> logline("Page up");
        break;
      case KEY_NPAGE:
        newSelectionIndex = this -> PageDown();
        logging -> logline("Page down");
        break;
      case KEY_UP:
        newSelectionIndex = this -> MoveUp();
        logging -> logline("up");
        break;
      case KEY_DOWN:
        newSelectionIndex = this -> MoveDown();
        logging -> logline("down");
        break;
    }
    logging -> logline("new selection index: " + std::to_string(newSelectionIndex));
    this -> UpdateSelection(newSelectionIndex);
  }
}

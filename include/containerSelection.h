#ifndef CONTAINER_SELECTION_H
#define CONTAINER_SELECTION_H

#include <curses.h>  
#include "containers.h"

class ContainerSelection {
  private:
    Logging* logging = &logging -> getInstance();
    long int previousSelectionIndex = 0;
    long int selectionIndex = 0;
    long int invStartIndex = 0; //The index of the topmost item on the screen, alows scrolling
    long int containerSelectionStart = -1;
    long int containerIndex = 0;
    bool redrawList = false;
    bool playerInventory = false;
    bool selectingItems = false;
    Container* container = NULL;
    std::vector<int> selectedIndices;
    std::vector<Item*> selectedItems;
    int itemViewLineCount;

  public:
    void HandleSelection(int choice);

    long int GetPreviousSelectionIndex() {
      return this -> previousSelectionIndex;
    }

    long int GetSelectionIndex() {
      return this -> selectionIndex;
    }

    long int GetInvStartIndex() {
      return this -> invStartIndex;
    }

    long int GetContainerIndex() {
      return this -> containerIndex;
    }

    Container* GetContainer() {
      return this -> container;
    }

    std::vector<Item*> GetSelectedItems() {
      return this -> selectedItems;
    }

    void Deselect(long int index) {
      Item* selectedItem = this -> container -> GetItem(index);
      std::vector<int>::iterator indexIter = std::find(this -> selectedIndices.begin(), this -> selectedIndices.end(), index);
      std::vector<Item*>::iterator selectionIter = std::find(this -> selectedItems.begin(), this -> selectedItems.end(), selectedItem);
      if (selectedItem != NULL) {
        this -> selectedIndices.erase(indexIter);
        this -> selectedItems.erase(selectionIter);
        logging -> logline("Deselected item idx: " + std::to_string(index));
      } else {
        logging -> logline("Container has no item at: " + std::to_string(index) + ". Ignoring selection.");
      }
    }

    void Select(long int index) {
      Item* selectedItem = this -> container -> GetItem(index);
      if (selectedItem != NULL) {
        this -> selectedIndices.push_back(index);
        this -> selectedItems.push_back(selectedItem);
        logging -> logline("Selected item idx: " + std::to_string(index));
      } else {
        logging -> logline("Container has no item at: " + std::to_string(index) + ". Ignoring selection.");
      }
    } 

    void SelectRange(int startIndex, int endIndex) {
      for (int i=startIndex; i<endIndex; i++) this -> Select(i);
    }

    void ClearSelection() {
      this -> selectedIndices.clear();
      this -> selectedItems.clear();
      this -> containerSelectionStart = -1;
    } 

    void AddSelectedItem(int index, Item* selectedItem) {
      this -> selectedIndices.push_back(index);
      this -> selectedItems.push_back(selectedItem);
    }

    bool IsRedrawList() {
      return this -> redrawList;
    }

    void SetRedrawList(bool redrawList) {
      this -> redrawList = redrawList;
    }

    bool IsPlayerInventory() {
      return this -> playerInventory;
    }

    std::vector<int> GetSelectedIndices() {
      return this -> selectedIndices;
    }

    void ClearSelectedIndices() {
      return this -> selectedIndices.clear();
    }

    ContainerSelection(Container* container, const  int itemViewLineCount, bool playerInventory) {
      this -> container = container;
      this -> itemViewLineCount = itemViewLineCount;
      this -> playerInventory = playerInventory;
    }
};

#endif

#ifndef ITEM_LIST_SELECTION_H
#define ITEM_LIST_SELECTION_H

#include <curses.h>
#include "containers.h"

enum SelectionMode { SELECTING_ITEMS, MOVING_ITEMS, SELECTING_CONTAINER };

class ItemListSelection {
  private:
    Logging* logging = &logging -> getInstance();

    // Internal selection state
    SelectionMode selectionMode = SELECTING_ITEMS;
    long int previousSelectionIndex = 0;
    long int selectionIndex = 0;
    long int invStartIndex = 0; //The index of the topmost item on the screen, allows scrolling
    long int containerSelectionStart = -1;
    long int containerIndex = 0;
    bool redrawList = false;
    bool selectingItems = false;

    // Constructor params
    std::vector<Item*> items;
    int itemViewLineCount;
    bool playerInventory = false;

    // Storage of items in the selection
    std::vector<int> selectedIndices;
    std::vector<Item*> selectedItems;

    void AddSelectedItem(int index, Item* selectedItem) {
      this -> selectedIndices.push_back(index);
      this -> selectedItems.push_back(selectedItem);
    }

    void RemoveSelectedItem(int index, Item* selectedItem) {
      std::vector<int>::iterator indexIter = std::find(this -> selectedIndices.begin(), this -> selectedIndices.end(), index);
      if (indexIter != this -> selectedIndices.end()) {
        this -> selectedIndices.erase(indexIter);
      }

      std::vector<Item*>::iterator selectionIter = std::find(this -> selectedItems.begin(), this -> selectedItems.end(), selectedItem);
      if (selectionIter != this -> selectedItems.end()) {
        this -> selectedItems.erase(selectionIter);
      }
    }

    bool IsIndexSelected(long int index) {
      return std::find(this -> selectedIndices.begin(), this -> selectedIndices.end(), index) != selectedIndices.end();
    }

    void UpdateSelection(long int newSelectionIndex);

  public:
    void HandleSelection(int choice);
    void HandleOtherContainerSelection(int choice);
    std::vector<Container*> GetSelectedContainers();
    std::vector<Container*> GetOtherContainersNotSelected();

    SelectionMode GetSelectionMode() {
      return this -> selectionMode;
    }

    void SetSelectionMode(SelectionMode selectionMode) {
      this -> selectionMode = selectionMode;
    }

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

    std::vector<Item*> GetSelectedItems() {
      return this -> selectedItems;
    }

    bool HasSelectedItems() {
      return !this -> selectedItems.empty();
    }

    Item* GetItem(long int index) {
      if (index < 0) return NULL;
      long unsigned int unsignedIndex = (long unsigned int) index;
      if (unsignedIndex < items.size()) {
         return items.at(index);
      }
      return NULL;
    }

    bool IsSelected(long int index) {
        if (IsIndexSelected(index)) {
          return true;
        } else {
          logging -> logline("index not in selection: " + std::to_string(index));
          return false;
        }
    }

    void Deselect(long int index) {
      if (IsSelected(index)) {
        Item* selectedItem = this -> GetItem(index);
        if (selectedItem != NULL) {
          this -> RemoveSelectedItem(index, selectedItem);
          logging -> logline("Deselected item idx: " + std::to_string(index));
        } else {
          logging -> logline("Container has no item at: " + std::to_string(index) + ". Ignoring deselection.");
        }
      } else {
        logging -> logline("Index: " + std::to_string(index) + " not selected. Ignoring deselection..");
      }
    }

    void Select(long int index) {
      if (!IsSelected(index)) {
        Item* selectedItem = this -> GetItem(index);
        if (selectedItem != NULL) {
        this -> AddSelectedItem(index, selectedItem);
          logging -> logline("Selected item idx: " + std::to_string(index));
        } else {
          logging -> logline("Container has no item at: " + std::to_string(index) + ". Ignoring selection.");
        }
      } else {
        logging -> logline("Index: " + std::to_string(index) + " already selected. Ignoring selection..");
      }
    }

    void DeselectRange(int startIndex, int endIndex) {
      for (int i=startIndex; i<endIndex; i++) this -> Deselect(i);
    }

    void SelectRange(int startIndex, int endIndex) {
      for (int i=startIndex; i<endIndex; i++) this -> Select(i);
    }

    void ClearSelection() {
      this -> selectedIndices.clear();
      this -> selectedItems.clear();
      this -> containerSelectionStart = -1;
      this -> selectingItems = false;
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

    ItemListSelection(std::vector<Item*> items, const  int itemViewLineCount, bool playerInventory) {
      this -> items = items;
      this -> itemViewLineCount = itemViewLineCount;
      this -> playerInventory = playerInventory;
    }

    ItemListSelection(std::vector<Container*> containers, const  int itemViewLineCount, bool playerInventory) {
      std::vector<Item*> containerItems = std::vector<Item*>();
      for (std::vector<Container*>::iterator containerIt = containers.begin(); containerIt != containers.end(); containerIt++) {
        containerItems.push_back(*containerIt);
      }
      this -> items = containerItems;
      this -> itemViewLineCount = itemViewLineCount;
      this -> playerInventory = playerInventory;
    }

};

#endif

#ifndef CONTAINER_SELECTION_H
#define CONTAINER_SELECTION_H

#include <curses.h>  
#include "containers.h"

enum SelectionMode { SELECTING_ITEMS, MOVING_ITEMS, SELECTING_CONTAINER };

class ContainerSelection {
  private:
    Logging* logging = &logging -> getInstance();
    SelectionMode selectionMode = SELECTING_ITEMS;
    long int previousSelectionIndex = 0;
    long int selectionIndex = 0;
    long int invStartIndex = 0; //The index of the topmost item on the screen, allows scrolling
    long int containerSelectionStart = -1;
    long int containerIndex = 0;
    bool redrawList = false;
    bool playerInventory = false;
    bool selectingItems = false;
    Container* container = NULL;
    std::vector<int> selectedIndices;
    std::vector<Item*> selectedItems;
    int itemViewLineCount;
    // For selecting another container
    std::vector<Container*> otherContainers;
    int otherContainerSelectionStartIndex = 0;
    int otherContainerSelectionIndex = 0;
    Container* selectedOtherContainer=NULL;

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

    void UpdateSelection(long int newSelectionIndex);

  public:
    SelectionMode GetSelectionMode() {
      return this -> selectionMode;
    }

    void SetSelectionMode(SelectionMode selectionMode) {
      this -> selectionMode = selectionMode;
    }

    void HandleSelection(int choice);
    void HandleOtherContainerSelection(int choice);

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

    std::vector<Container*> GetSelectedContainers();
    std::vector<Container*> GetOtherContainersNotSelected();

    bool HasSelectedItems() {
      return !this -> selectedItems.empty();
    }


    bool IsSelected(long int index) {
        bool foundIndex = std::find(this -> selectedIndices.begin(), this -> selectedIndices.end(), index) != selectedIndices.end();
        if (foundIndex) {
        	Item* selectedItem = this -> container -> GetItem(index);
        	bool foundItem = std::find(this -> selectedItems.begin(), this -> selectedItems.end(), selectedItem) != selectedItems.end();
        	return foundItem;
        } else {
			logging -> logline("index not in selection: " + std::to_string(index));
        }
        return false;
    }

    void Deselect(long int index) {
      if (IsSelected(index)) {
		  Item* selectedItem = this -> container -> GetItem(index);
		  if (selectedItem != NULL) {
			this -> RemoveSelectedItem(index, selectedItem);
			logging -> logline("Deselected item idx: " + std::to_string(index));
		  } else {
			logging -> logline("Container has no item at: " + std::to_string(index) + ". Ignoring selection.");
		  }
      } else {
		logging -> logline("Index: " + std::to_string(index) + " not selected. Ignoring..");
      }
    }

    void Select(long int index) {
      if (!IsSelected(index)) {
		  Item* selectedItem = this -> container -> GetItem(index);
		  if (selectedItem != NULL) {
			this -> AddSelectedItem(index, selectedItem);
			  logging -> logline("Selected item idx: " + std::to_string(index));
		  } else {
		    logging -> logline("Container has no item at: " + std::to_string(index) + ". Ignoring selection.");
		  }
      } else {
        logging -> logline("Index: " + std::to_string(index) + " already selected. Ignoring..");
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

    void ClearSelectedIndices() {
      return this -> selectedIndices.clear();
    }

    std::vector<Container*> GetOtherContainers() {
      return this -> otherContainers;
    }

    void SetOtherContainers(std::vector<Container*> containers) {
      this -> otherContainers = containers;
    }

    int GetOtherContainerSelectionIndex() const {
      return otherContainerSelectionIndex;
    }

    int GetOtherContainerSelectionStartIndex() const {
      return otherContainerSelectionStartIndex;
    }

    Container* GetSelectedOtherContainer() const {
      return selectedOtherContainer;
    }

    ContainerSelection(Container* container, const  int itemViewLineCount, bool playerInventory) {
      this -> container = container;
      this -> itemViewLineCount = itemViewLineCount;
      this -> playerInventory = playerInventory;
    }


};

#endif

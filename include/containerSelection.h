#ifndef CONTAINER_SELECTION_H
#define CONTAINER_SELECTION_H

#include <curses.h>  
#include "containers.h"
#include "itemListSelection.h"

class ContainerSelection {
  private:
    Logging* logging = &logging -> getInstance();

    Container* container = NULL;
    ItemListSelection* itemListSelection = NULL;

    // For selecting another container
    std::vector<Container*> otherContainers;

  public:
    //void HandleOtherContainerSelection(int choice);
    std::vector<Container*> GetContainersNotSelected();

    // --- Passthrough methods
    void HandleSelection(int choice) {
      return this -> itemListSelection -> HandleSelection(choice);
    }

    long int GetPreviousSelectionIndex() {
      return this -> itemListSelection -> GetPreviousSelectionIndex();
    }

    long int GetSelectionIndex() {
      return this -> itemListSelection -> GetSelectionIndex();
    }

    long int GetInvStartIndex() {
      return this -> itemListSelection -> GetInvStartIndex();
    }

    long int GetContainerIndex() {
      return this -> itemListSelection -> GetContainerIndex();
    }

    void Select(long int index) {
      this -> itemListSelection -> Select(index);
    }

    bool IsSelected(long int index) {
        return this -> itemListSelection -> IsSelected(index);
    }

    bool HasSelectedItems() {
      return this -> itemListSelection -> HasSelectedItems();
    }

    std::vector<Item*> GetSelectedItems() {
      return this -> itemListSelection -> GetSelectedItems();
    }

    std::vector<Container*> GetSelectedContainers() {
      return this -> itemListSelection -> GetSelectedContainers();
    }

    std::vector<int> GetSelectedIndices() {
      return this -> itemListSelection -> GetSelectedIndices();
    }

    SelectionMode GetSelectionMode() {
      return this -> itemListSelection -> GetSelectionMode();
    }

    void SetSelectionMode(SelectionMode selectionMode) {
      return this -> itemListSelection -> SetSelectionMode(selectionMode);
    }

    bool IsRedrawList() {
      return this -> itemListSelection -> IsRedrawList();
    }

    void SetRedrawList(bool redrawList) {
      this -> itemListSelection -> SetRedrawList(redrawList);
    }

    void ClearSelection() {
      this -> itemListSelection -> ClearSelection();
    }

    bool IsPlayerInventory() {
      return this -> itemListSelection -> IsPlayerInventory();
    }
    // ---------

    Container* GetContainer() {
      return this -> container;
    }

    std::vector<Container*> GetOtherContainers() {
      return this -> otherContainers;
    }

    void SetOtherContainers(std::vector<Container*> containers) {
      this -> otherContainers = containers;
    }

    ContainerSelection(Container* container, const  int itemViewLineCount, bool playerInventory) {
      this -> container = container;
      this -> itemListSelection = new ItemListSelection(container -> GetItems(), itemViewLineCount, playerInventory);
    }


};

#endif

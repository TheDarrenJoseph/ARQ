#ifndef INVENTORY_UI_H
#define INVENTORY_UI_H

#include <algorithm>

#include "logging.h"
#include "ui.h"
#include "playerInventoryFunctions.h"
#include "cursesUI.h"
#include "containers.h"
#include "stringUtils.h"
#include "containerSelection.h"

class InventoryUI {
  private:
    Logging* logging = &logging -> getInstance();
    WINDOW* invwin_front=NULL;
    WINDOW* invwin_rear=NULL;
    ContainerSelection* currentContainerSelection;
    std::vector<ContainerSelection*> containerSelections;
    bool selectingItems = false;
    bool movingItem = false;

    // Each column has a +2 margin and the offset of the previous label length (or any other needed padding for contents)
    const int COL_1 = 0;
    const int COL_2 = 20;
    const int COL_3 = 33;

    bool RootContainerIsPlayerInventory();
    void TakeItem(Container* container, int index);
    void DropItem(Item* item);
    int MoveItem(Container* container, Item* item, Item* targetItem);
    int MoveItems(Container* container, std::vector<Item*> items, Item* targetItem);
    void OpenContainer(Container * c, int index);
    std::vector<Container*> FindOtherContainers(ContainerSelection* containerSelection);

  public:
    virtual void DrawRearWindow(ContainerSelection* containerSelection);
    virtual void DrawItem(Item* item, int inventoryLineIndex);
    virtual void DrawAvailableContainers(ContainerSelection* containerSelection);
    virtual void DrawInventory(ContainerSelection* containerSelection, long int invIndex);
    virtual void HighlightInv(int xChars, int xIndex, int yIndex);
    virtual void HighlightInvLine(int yIndex, int colourCode);
    virtual void ColourInvLine(int index, int colourCode);
    virtual void UnhighlightInvLine(int yIndex);

    virtual void ClearInvHighlighting();
    virtual void ClearInvWindow();
    virtual void EraseInvWindow();

    int AttemptMoveItems(ContainerSelection* containerSelection);
    int AttemptDropItems(ContainerSelection* containerSelection);
    int InventoryInput(ContainerSelection* containerSelection, int inputChoice);
    void PrintAccessContainerHints();
    void AccessContainer(Container * c, bool playerInv);
    void InventoryCommandInput(ContainerSelection* containerSelection);

    UI* mainUI = NULL;
    Container* mainContainer = NULL;
    InventoryFunctions* inventoryFunctions = NULL;
    bool playerInventory = true;

    bool IsPlayerInventory();
    void SetPlayerInventory(bool playerInventory);

    InventoryUI(UI* mainUI, Container* mainContainer, InventoryFunctions* inventoryFunctions) {
      this -> mainUI = mainUI;
      this -> mainContainer = mainContainer;
      this -> inventoryFunctions = inventoryFunctions;
    }
};

#endif

#ifndef INVENTORY_UI_H
#define INVENTORY_UI_H

#include "logging.h"
#include "ui.h"
#include "playerInventoryFunctions.h"
#include "cursesUI.h"
#include "containers.h"
#include "stringUtils.h"

class InventoryUI {
  private:
    Logging* logging = &logging -> getInstance();
    void DropItem(const Item* item, bool playerInv);
    void OpenContainer(Container * c, int index);

  public:
    bool InventoryInput(int choice, int index, Container* c, bool playerInv);
    void PrintAccessContainerHints();
    void AccessContainer(Container * c, bool playerInv);
    void AccessListCommand(Container* c, int index, bool playerInv);

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

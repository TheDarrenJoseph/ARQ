#include "inventoryUI.h"

/**
 * 
 * @param choice int code for the player input
 * @param index The current selected item index in this list
 * @param c A pointer to this container
 * @param playerInv Whether or not this is the player's inventory or another container
 * @return 
 */
bool InventoryUI::InventoryInput(ContainerSelection* containerSelection, int inputChoice, bool playerInv) {
  unsigned int invStartIndex = containerSelection -> GetInvStartIndex();
  unsigned int containerIndex = containerSelection -> GetContainerIndex();;

  Container* container = containerSelection -> GetContainer();
  switch(inputChoice) {
    case ('q') : {
      return false;
      break;
    }
    case ('c') : {
      AccessListCommand(container, containerIndex, playerInv);
      break;
    }
    case ('i') : { //Item info
      break;
    }
    case ('o') : {
      OpenContainer(container, containerIndex);
      break;
    }
    case ('t') : {
      TakeItem(container, containerIndex, playerInv);
      mainUI->ListInv(container, invStartIndex);
      break;
    }
    case ('d') : {
      DropItem(container -> GetItem(containerIndex), playerInv);
      mainUI->ListInv(container, invStartIndex);
      break;
    }
    case ('m') : {
      Item* movingItem = containerSelection -> GetMovingItem();
      if (movingItem == NULL) {
        Item* movingItem = container -> GetItem(containerIndex);
        containerSelection -> SetMovingItem(movingItem);
        mainUI->ClearConsoleAndPrint("Moving: " + movingItem -> GetName() + ". Please choose the new location and hit m again. q to cancel");
      } else {
        Item* targetItem = container -> GetItem(containerIndex);
        this -> MoveItem(container, movingItem, targetItem);
      }
      break;
    }
  }
  return true;
}


void InventoryUI::PrintAccessContainerHints() {

    mainUI -> ClearConsole();
    mainUI -> ConsolePrint("Up/Down - Select an item ", 0, 0);
    mainUI -> ConsolePrint("c - Enter Command",0,1);
    mainUI -> ConsolePrint("q - Close window",0,2);
}

void InventoryUI::TakeItem(Container* container, int index, bool playerInv) {
  if (playerInv) {
    logging -> logline("Taking item from " + container -> GetName() + " at index: " + std::to_string(index));
    inventoryFunctions -> TakeItem(container, index);
  } else {
    mainUI->ClearConsoleAndPrint("You cannot take from your own inventory.");
  }
}


void InventoryUI::DropItem(Item* item, bool playerInv) {
  if (playerInv) {
    inventoryFunctions -> DropPlayerItem(item);
    mainUI->ClearConsoleAndPrint("Dropped the " + item -> GetName());
  } else {
    mainUI->ClearConsoleAndPrint("You cannot drop that.");
  }
}

int InventoryUI :: MoveItem(Container* container, Item* item, Item* targetItem)
{   
  //Dynamic cast to type check
  Container* targetContainer = dynamic_cast<Container*> (targetItem);
  //If incorrectly cast (item), pointer will be NULL
  // Move into container
  if (targetContainer != NULL) {
    targetContainer -> AddItem(item);
    container -> RemoveItem(item);
  } else {
    // Insert at this index
  }
    return 1;
}

void InventoryUI :: OpenContainer(Container * parent, int index) {
  Item* itm = parent -> GetItem(index);   

  //Dynamic cast to type check
  Container* c = dynamic_cast<Container*> (itm);

  //If incorrectly cast (item), pointer will be NULL
  if (c != NULL) {
      mainUI-> ClearInvHighlighting();
      AccessContainer((Container*)itm, false); //Cast and pass
  }
}

/**
 * 
 * @param c         
 * @param playerInv whether or not we are current looking at the player's inventory, disables 'take'
 * @return 
 */
void InventoryUI::AccessContainer(Container * c, bool playerInv)
{
    mainUI -> ClearInvWindow();
    bool selection = true;
    long unsigned int selectionIndex = 0;
    long unsigned int invStartIndex = 0; //The index of the topmost item on the screen, alows scrolling

    //Selection loop
    int inputChoice = -1;
    mainUI->ListInv(c,invStartIndex);
    ContainerSelection containerSelection = ContainerSelection(c, INVWIN_FRONT_Y);
    while(InventoryInput(&containerSelection, inputChoice, playerInv)) {
      Item* movingItem = containerSelection.GetMovingItem();
      if (movingItem == NULL) PrintAccessContainerHints();
      mainUI->HighlightInvLine(selectionIndex);      

      inputChoice = mainUI->ConsoleGetInput();
      long unsigned int containerSize = c->GetSize(); 
      containerSelection.HandleSelection(inputChoice);

      selectionIndex = containerSelection.GetSelectionIndex();
      invStartIndex = containerSelection.GetInvStartIndex();
      logging -> logline("new selectionIndex: " + std::to_string(selectionIndex));
      logging -> logline("new invStartIndex: " + std::to_string(invStartIndex));

      mainUI->UnhighlightInvLine(containerSelection.GetPreviousSelectionIndex());      
      if (containerSelection.IsRedrawList()) mainUI->ListInv(c,invStartIndex);

      //logging -> logline("maxScrollIndex: " + std::to_string(maxScrollIndex));
    }
    
}

/** Allows the user to enter a command to perform operations on things in a list/inventory
 *  This allows dropping/moving items, etc
 * @param index The currently selected item index
 * @param mainUI The UI to call to
 * @param playerInv whether or not this is the player's inventory (to disable "take", etc)
 * @return 
 */
void InventoryUI :: AccessListCommand(Container* c, int index, bool playerInv) {
    while (true) {
      echo();
      mainUI->ClearConsole();
      mainUI->ConsolePrint(PROMPT_TEXT, 0, 0);
      std::string lowerCasedAnswer = StringUtils::toLowerCase(mainUI->ConsoleGetString());
      std::string itemName = c-> GetItem(index) -> GetName();
      logging -> logline("Player inputted: '" + lowerCasedAnswer + "'");
      if (lowerCasedAnswer == "drop") {
          
          if (playerInv) {
            inventoryFunctions -> DropPlayerItem(c->GetItem(index));
            mainUI->ClearConsoleAndPrint("Dropped the " + itemName);
          } else {
            mainUI->ClearConsoleAndPrint("Cannot drop a container. Perhaps move it? ");
          }
          return;
      } else if (lowerCasedAnswer == "move") {
          mainUI->ClearConsoleAndPrint("Moving the " + itemName);
          // TODO
          //1. Player Inv
          //2.. Location Inv
          return;
      } else if (lowerCasedAnswer == "open") {
          OpenContainer(c, index);
          return;
      }
  }
}

bool InventoryUI :: IsPlayerInventory() {
  return this -> playerInventory;
}

void InventoryUI :: SetPlayerInventory(bool playerInventory) {
  this -> playerInventory = playerInventory;
}

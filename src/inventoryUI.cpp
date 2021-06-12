#include "inventoryUI.h"

/**
 * 
 * @param choice int code for the player input
 * @param index The current selected item index in this list
 * @param c A pointer to this container
 * @param playerInv Whether or not this is the player's inventory or another container
 * @return 
 */
bool InventoryUI::InventoryInput(int choice, int index, Container* c, bool playerInv) {
     switch(choice) {
            case ('q') : {
                return false;
                break;
            }
            case ('c') : {
                AccessListCommand(c, index, playerInv);
                break;
            }
           case ('i') : { //Item info
                break;
            }
           case ('o') : {
                OpenContainer(c, index);
                break;
            }
           case ('d') : {
                DropItem(c -> GetItem(index), playerInv);
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

void InventoryUI::DropItem(const Item* item, bool playerInv) {
  if (playerInv) {
    inventoryFunctions -> DropPlayerItem(item);
    mainUI->ClearConsoleAndPrint("Dropped the " + item -> GetName());
  } else {
    mainUI->ClearConsoleAndPrint("Cannot drop a container. Perhaps move it? ");
  }
}

void InventoryUI::OpenContainer(Container * parent, int index) {
  const Item* itm = parent -> GetItem(index);   

  //Dynamic cast to type check
  const Container* c = dynamic_cast<const Container*> (itm);

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
    bool selection = true;
    long unsigned int selectionIndex = 0;
    long unsigned int invStartIndex = 0; //The index of the topmost item on the screen, alows scrolling
    const unsigned long int INV_WINDOW_LINES = INVWIN_FRONT_Y-2;

    //Selection loop
    int choice = -1;
    while(InventoryInput(choice, selectionIndex, c, playerInv)) {
        PrintAccessContainerHints();
        mainUI->ListInv(c,invStartIndex);
        mainUI->HighlightInvLine(selectionIndex);      

        choice = mainUI->ConsoleGetInput();
        long unsigned int containerSize = c->GetSize(); 
        long unsigned int lowestInvIndex = containerSize - INV_WINDOW_LINES;
        logging -> logline("Container size: " + std::to_string(containerSize));
        //logging -> logline("Lowest inv index: " + std::to_string(lowestInvIndex));
        //logging -> logline("inv index: " + std::to_string(invStartIndex));
        switch (choice) {
          case KEY_UP:
            if (selectionIndex>0) {
                selectionIndex--; 
            } else if (selectionIndex==0 && invStartIndex>0) {
                invStartIndex--;
            }
            break;
          case KEY_DOWN:
            //Checking against the window boundary. containerSize allows
            if (selectionIndex < INV_WINDOW_LINES && selectionIndex < containerSize) {
                if (selectionIndex == INV_WINDOW_LINES-1 && invStartIndex < containerSize && invStartIndex < lowestInvIndex) {
                  invStartIndex++;
                } else {
                  selectionIndex++; 
                }
            }
            break;
        }
        mainUI-> ClearInvHighlighting();
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

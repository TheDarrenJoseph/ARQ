#include "inventoryUI.h"


bool InventoryUI::RootContainerIsPlayerInventory() {
  return this -> containerSelections.front() -> IsPlayerInventory(); 
}

/**
 * 
 * @param c The container to list
 * @param invIndex the top index of the list, so that the list can scroll down
 */
void InventoryUI::DrawInventory(ContainerSelection* containerSelection, long unsigned int invIndex)
{   
    Container* c = containerSelection -> GetContainer();
    box(invwin_rear, 0, 0);
    mainUI -> wprint_at(invwin_rear,c -> GetName().c_str(),0,1);
    
    // Each column has a +2 margin and the offset of the previous label length (or any other needed padding for contents)
    const int COL_1 = 0;
    const int COL_2 = 20;
    const int COL_3 = 33;
      //For each line of the window
    mainUI -> wprint_at(invwin_rear,"NAME", 1, COL_1+1); // +1 for border
    mainUI -> wprint_at(invwin_rear,"WEIGHT (Kg)", 1, COL_2);
    mainUI -> wprint_at(invwin_rear,"VALUE", 1, COL_3);
    //box(invwin_rear, 0, 0);
    //box(invwin_front, 0, 0);
    //wrefresh(invwin_rear);
    //wrefresh(invwin_front);

    long unsigned int invSize = c->GetSize();    
    long unsigned int lowestDisplayIndex = (long unsigned int)INVWIN_FRONT_Y;
    for (long unsigned int i=0;  i < lowestDisplayIndex && (invIndex+i) < invSize; i++) {
                Item* thisItem = c->GetItem(invIndex+i);
                char buffer[120];            
                sprintf(buffer,"%-20s",thisItem->GetName().c_str());
                mainUI -> wprint_at(invwin_front, buffer, i, COL_1);
               
                //Weight
                sprintf(buffer,"%-4d",thisItem->GetWeight());
                mainUI -> wprint_at(invwin_front, buffer, i, COL_2);
               
                //Value
                sprintf(buffer,"%-4d",thisItem->GetValue());
                mainUI -> wprint_at(invwin_front, buffer, i, COL_3);
    }        

    int maxX, maxY;
    getmaxyx(invwin_rear, maxY, maxX);
    // Footer instructions
    char buffer[30];            
    sprintf(buffer,"%03d/%03dkgs", c -> GetTotalWeight(), c -> GetWeightLimit());
    mainUI -> wprint_at(invwin_rear, buffer, maxY-1, maxX-11); // Add commands to the bottom of the window

    if (RootContainerIsPlayerInventory()) {
      mainUI -> wprint_at(invwin_rear,"(o)pen, (d)rop, (m)ove", maxY-1, 1); // Add commands to the bottom of the window
    } else {
      mainUI -> wprint_at(invwin_rear,"(o)pen, (t)ake, (m)ove", maxY-1, 1); // Add commands to the bottom of the window
    }

    //mainUI -> UpdateUI();
    //wrefresh(invwin_rear);
    //wrefresh(invwin_front);
    return;
}

/**
 * 
 * @param choice int code for the player input
 * @param index The current selected item index in this list
 * @param c A pointer to this container
 * @param playerInv Whether or not this is the player's inventory or another container
 * @return 
 */
bool InventoryUI::InventoryInput(ContainerSelection* containerSelection, int inputChoice) {
  unsigned int containerIndex = containerSelection -> GetContainerIndex();;
  logging -> logline("Container index: " + std::to_string(containerIndex));

  Container* container = containerSelection -> GetContainer();
  switch(inputChoice) {
    case ('q') : {
      bool movingAnItem = NULL != containerSelection -> GetMovingItem();
      if(movingAnItem) {
        containerSelection -> SetMovingItem(NULL);
        containerSelection -> SetRedrawList(true);
        return true;
      }
      return false;
      break;
    }
    case ('c') : {
      AccessListCommand(container, containerIndex);
      break;
    }
    case ('i') : { //Item info
      break;
    }
    case ('o') : {
      OpenContainer(container, containerIndex);
      containerSelection -> SetRedrawList(true);
      break;
    }
    case ('t') : {
      TakeItem(container, containerIndex);
      containerSelection -> SetRedrawList(true);
      break;
    }
    case ('d') : {
      DropItem(container -> GetItem(containerIndex));
      containerSelection -> SetRedrawList(true);
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
        if (targetItem == movingItem) {
          logging -> logline("Cannot move item to itself!");
        } if (targetItem != NULL && movingItem != NULL) {
          this -> MoveItem(container, movingItem, targetItem);
          containerSelection -> SetRedrawList(true);
          containerSelection -> SetMovingItem(NULL);
          mainUI->ClearConsoleAndPrint("Moved: " + movingItem -> GetName());
          //logging -> logline("Moving: " + movingItem -> GetName());
        } else {
          logging -> logline("Target or moving item was NULL!");
        }
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

void InventoryUI::TakeItem(Container* container, int index) {
  bool playerInv = RootContainerIsPlayerInventory();
  if (!playerInv) {
    logging -> logline("Taking item from " + container -> GetName() + " at index: " + std::to_string(index));
    inventoryFunctions -> TakeItem(container, index);
  }
}


void InventoryUI::DropItem(Item* item) {
  if (RootContainerIsPlayerInventory()) {
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
    logging -> logline("Moved item");
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
      ClearInvHighlighting();
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
    this -> invwin_rear = newwin(INVWIN_REAR_Y, INVWIN_REAR_X, 2, 2);
    this -> invwin_front = newwin(INVWIN_FRONT_Y, INVWIN_FRONT_X, 4, 4);

    ClearInvWindow();
    long unsigned int selectionIndex = 0;
    long unsigned int invStartIndex = 0; //The index of the topmost item on the screen, alows scrolling

    //Selection loop
    int inputChoice = -1;
    ContainerSelection* containerSelection = new ContainerSelection(c, INVWIN_FRONT_Y, playerInv);
    this -> containerSelections.push_back(containerSelection);
    currentContainerSelection = this -> containerSelections.back();
    DrawInventory(containerSelection,invStartIndex);
    while(InventoryInput(containerSelection, inputChoice)) {
      if (containerSelection -> IsRedrawList()) {
        logging -> logline("Redrawing container list..");
        DrawInventory(containerSelection,invStartIndex);
        containerSelection -> SetRedrawList(false);
      }

      Item* movingItem = containerSelection -> GetMovingItem();
      if (movingItem == NULL) PrintAccessContainerHints();
      HighlightInvLine(selectionIndex);      

      inputChoice = mainUI->ConsoleGetInput();
      containerSelection -> HandleSelection(inputChoice);

      selectionIndex = containerSelection ->  GetSelectionIndex();
      invStartIndex = containerSelection -> GetInvStartIndex();
      UnhighlightInvLine(containerSelection -> GetPreviousSelectionIndex());      


      //wrefresh(invwin_rear);
      wrefresh(invwin_front);
      //logging -> logline("maxScrollIndex: " + std::to_string(maxScrollIndex));
    }
    // delete the old selection
    this -> containerSelections.pop_back();
}

/** Allows the user to enter a command to perform operations on things in a list/inventory
 *  This allows dropping/moving items, etc
 * @param index The currently selected item index
 * @param mainUI The UI to call to
 * @param playerInv whether or not this is the player's inventory (to disable "take", etc)
 * @return 
 */
void InventoryUI :: AccessListCommand(Container* c, int index) {
    bool playerInv = RootContainerIsPlayerInventory();
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

void InventoryUI :: ClearInvHighlighting() {
    //Clear other highlighting
    for (int y=0; y<INVWIN_FRONT_Y-1; y++) {
        mvwchgat(invwin_front, y, 0, INVWIN_FRONT_X, A_NORMAL, 0, NULL);
    }
    wrefresh(invwin_front);
}

void InventoryUI :: ClearInvWindow() {
  wclear(invwin_front);
  wclear(invwin_rear);
}

void InventoryUI :: EraseInvWindow() {
  werase(invwin_front);
  werase(invwin_rear);
}


/** Highlights xChars characters at the specified x,yIndex
 * 
 * @param xChars
 * @param yIndex
 */
void InventoryUI :: HighlightInv(int xChars, int xIndex, int yIndex) {    
    //Index/Selection highlight
    mvwchgat(invwin_front, yIndex, xIndex, xChars, A_BLINK, 1, NULL); 
    wrefresh(invwin_front);
}

void InventoryUI :: HighlightInvLine(int yIndex) {
    mvwchgat(invwin_front, yIndex, 0, INVWIN_FRONT_X-1 , A_BLINK, 1, NULL); //add red blink to the current line
    wrefresh(invwin_front);
}

void InventoryUI :: UnhighlightInvLine(int yIndex) {
    mvwchgat(invwin_front, yIndex, 0, INVWIN_FRONT_X-1, A_NORMAL, 0, NULL);
    wrefresh(invwin_front);
}


bool InventoryUI :: IsPlayerInventory() {
  return this -> playerInventory;
}

void InventoryUI :: SetPlayerInventory(bool playerInventory) {
  this -> playerInventory = playerInventory;
}

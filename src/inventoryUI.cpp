#include "inventoryUI.h"


bool InventoryUI::RootContainerIsPlayerInventory() {
  return this -> containerSelections.front() -> IsPlayerInventory(); 
}


// Redraws the rear / outside inventory window (title, decoration, etc)
void InventoryUI::DrawRearWindow(ContainerSelection* containerSelection) {
	wclear(invwin_rear);
    Container* c = containerSelection -> GetContainer();
    box(invwin_rear, 0, 0);
    mainUI -> wprint_at(invwin_rear,c -> GetName().c_str(),0,1);
    
    //For each line of the window
    mainUI -> wprint_at(invwin_rear,"NAME", 1, COL_1+1); // +1 for border
    mainUI -> wprint_at(invwin_rear,"WEIGHT (Kg)", 1, COL_2);
    mainUI -> wprint_at(invwin_rear,"VALUE", 1, COL_3);

    int maxX, maxY;
    getmaxyx(invwin_rear, maxY, maxX);
    // Footer instructions
    char buffer[30];
    sprintf(buffer,"%03d/%03ldkgs", c -> GetTotalWeight(), c -> GetWeightLimit());
    mainUI -> wprint_at(invwin_rear, buffer, maxY-1, maxX-11); // Add commands to the bottom of the window

    if (RootContainerIsPlayerInventory()) {
      mainUI -> wprint_at(invwin_rear,"(o)pen, (d)rop, (m)ove", maxY-1, 1); // Add commands to the bottom of the window
    } else {
      mainUI -> wprint_at(invwin_rear,"(o)pen, (t)ake, (m)ove", maxY-1, 1); // Add commands to the bottom of the window
    }
    wrefresh(invwin_rear);
}

void InventoryUI::DrawItem(Item* item, int inventoryLineIndex) {
	char buffer[120];
	sprintf(buffer,"%-20s",item->GetName().c_str());
	mainUI -> wprint_at(invwin_front, buffer, inventoryLineIndex, COL_1);
	sprintf(buffer,"%-4d",item->GetWeight());
	mainUI -> wprint_at(invwin_front, buffer, inventoryLineIndex, COL_2);
	sprintf(buffer,"%-4d",item->GetValue());
	mainUI -> wprint_at(invwin_front, buffer, inventoryLineIndex, COL_3);
}

/**
 * Draws the ContainerSelection to the front (inner) inventory window
 * @param c The container to list
 * @param invIndex the top index of the list, so that the list can scroll down
 */
void InventoryUI::DrawInventory(ContainerSelection* containerSelection, long int invIndex)
{
	wclear(invwin_front);
    Container* c = containerSelection -> GetContainer();
    long int invSize = c->GetSize();    
    long int lowestDisplayIndex = (long int)INVWIN_FRONT_Y;
    for (long int i=0;  i < lowestDisplayIndex && (invIndex+i) < invSize; i++) {
		Item* item = c->GetItem(invIndex+i);
		if (item != NULL) this -> DrawItem(item, i);
    }        
    wrefresh(invwin_front);
}

int InventoryUI::AttemptMoveItems(ContainerSelection* containerSelection) {
	std::vector<Item*> movingItems = containerSelection -> GetSelectedItems();
	std::vector<int> selectedIndices = containerSelection -> GetSelectedIndices();
	int containerIndex = containerSelection -> GetContainerIndex();;
	Container* container = containerSelection -> GetContainer();
	if (container->GetSize() == 0) return 1;
	if (movingItems.empty()) {
		Item* movingItem = container -> GetItem(containerIndex);
		containerSelection -> Select(containerIndex);
		mainUI->ClearConsoleAndPrint("Moving: " + movingItem -> GetName() + ". Please choose the new location and hit m again. q to cancel");
	} else {
		Item* targetItem = container -> GetItem(containerIndex);
		std::vector<Item*>::iterator targetFindIter = std::find(movingItems.begin(), movingItems.end(), targetItem);
		bool targetIsInSelection = targetFindIter != movingItems.end();
		if (targetIsInSelection) {
		  logging -> logline("Cannot move item to itself!");
		} else {
			for (std::vector<Item*>::iterator movingIter = movingItems.begin(); movingIter != movingItems.end(); movingIter++) {
				Item* item = *movingIter;
				if (item != NULL) {
				  this -> MoveItem(container, item, targetItem);
				  mainUI -> ClearConsoleAndPrint("Moved: " + item -> GetName());
				} else {
				  logging -> logline("moving item was NULL!");
				}
			}
			containerSelection -> SetRedrawList(true);
			containerSelection -> ClearSelection();
			return 0;
		}
	}
	return 1;
}

int InventoryUI::AttemptDropItems(ContainerSelection* containerSelection) {
	std::vector<Item*> droppingItems = containerSelection -> GetSelectedItems();
	std::vector<int> selectedIndices = containerSelection -> GetSelectedIndices();
	if (!droppingItems.empty()) {
		for (std::vector<Item*>::iterator dropItemsIter = droppingItems.begin(); dropItemsIter != droppingItems.end(); dropItemsIter++) {
			Item* item = *dropItemsIter;
			if (item != NULL) {
			  this -> DropItem(item);
			  mainUI -> ClearConsoleAndPrint("Dropped: " + item -> GetName());
			} else {
			  logging -> logline("Dropping item was NULL!");
			}
		}
		containerSelection -> SetRedrawList(true);
		containerSelection -> ClearSelection();
		return 0;
	}
	return 1;
}

/**
 * 
 * @param choice int code for the player input
 * @param index The current selected item index in this list
 * @param c A pointer to this container
 * @param playerInv Whether or not this is the player's inventory or another container
 * @return whether to 
 */
int InventoryUI::InventoryInput(ContainerSelection* containerSelection, int inputChoice) {
   int containerIndex = containerSelection -> GetContainerIndex();;
  logging -> logline("Container index: " + std::to_string(containerIndex));

  Container* container = containerSelection -> GetContainer();
  switch(inputChoice) {
    case ('q') : {
      // Selection clearing
      if (!containerSelection -> GetSelectedIndices().empty()) {
        containerSelection -> ClearSelection();  
        containerSelection -> SetRedrawList(true);
        return 0;
      }

      return 1;
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
      if(!RootContainerIsPlayerInventory()) {
		  TakeItem(container, containerIndex);
		  containerSelection -> SetRedrawList(true);
      }
	  break;
    }
    case ('d') : {
      if(RootContainerIsPlayerInventory()) {
		  std::vector<Item*> droppingItems = containerSelection -> GetSelectedItems();
		  // If we've not selected anything, select the currently chosen item
		  if (droppingItems.empty()) {
			 containerSelection -> Select(containerIndex);
		  }
		  this -> AttemptDropItems(containerSelection);
		  containerSelection -> SetRedrawList(true);
      }
      break;
    }
    case ('m') : {
      this -> AttemptMoveItems(containerSelection);
      break;
    }
  }
  return 0;
}


void InventoryUI::PrintAccessContainerHints() {

    mainUI -> ClearConsole();
    mainUI -> ConsolePrint("Up/Down - Select an item. Enter - select multiple. q clear selection", 0, 0);
    mainUI -> ConsolePrint("c - Enter a Command",0,1);
    mainUI -> ConsolePrint("h - Help/Command list",0,1);
    mainUI -> ConsolePrint("q - Close this window",0,3);
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
	if (item == targetItem) {
		logging -> logline("item matches targetItem! Cannot move item to it's own location!");
		return 1;
	}

	//Dynamic cast to type check
	Container* targetContainer = dynamic_cast<Container*> (targetItem);
	//If incorrectly cast (item), pointer will be NULL
	// Move into container
	if (targetContainer != NULL) {
		targetContainer -> AddItem(item);
		container -> RemoveItem(item);
		logging -> logline("Moved item: " + item -> GetName() + " into " + targetContainer -> GetName());
	} else {
		// Insert at this index
		long int fromIndex = container -> IndexOf(item);
		long int targetIndex = container -> IndexOf(targetItem);
		container -> RemoveItem(item);
		container -> AddItem(item, targetIndex);
		logging -> logline("Moved item: " + item -> GetName() + " from " + std::to_string(fromIndex) + " to " + std::to_string(targetIndex));

	}
	return 1;
}

int InventoryUI :: MoveItems(Container* container, std::vector<Item*> items, Item* targetItem)
{

  int targetIndex = container -> IndexOf(targetItem);
  //Dynamic cast to type check
  Container* targetContainer = dynamic_cast<Container*> (targetItem);
  //If incorrectly cast (item), pointer will be NULL
  // Move into container
  if (targetContainer != NULL) {
    targetContainer -> AddItems(items, targetIndex);
    container -> RemoveItems(items);
    logging -> logline("Moved " + std::to_string(items.size()) + " items into: " + targetContainer -> GetName());
    return 0;
  } else {
    // Insert at this index
    container -> RemoveItems(items);
	container -> AddItems(items, targetIndex);
    logging -> logline("Moved " + std::to_string(items.size()) + " items to index: " + std::to_string(targetIndex));
	return 0;
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
	// Hide the cursor
    curs_set(0);
    this -> invwin_rear = newwin(INVWIN_REAR_Y, INVWIN_REAR_X, 2, 2);
    this -> invwin_front = newwin(INVWIN_FRONT_Y, INVWIN_FRONT_X, 4, 4);

    ClearInvWindow();
    long int selectionIndex = 0;
    long int invStartIndex = 0; //The index of the topmost item on the screen, alows scrolling

    //Selection loop
    int inputChoice = -1;
    ContainerSelection* containerSelection = new ContainerSelection(c, INVWIN_FRONT_Y, playerInv);
    this -> containerSelections.push_back(containerSelection);
    currentContainerSelection = this -> containerSelections.back();
    DrawRearWindow(containerSelection);
    DrawInventory(containerSelection,invStartIndex);
    while(InventoryInput(containerSelection, inputChoice) == 0) {
      int containerSize = c -> GetSize();
      if (containerSize > 0) {
          if (containerSelection -> IsRedrawList()) {
            logging -> logline("Redrawing container list..");
            DrawInventory(containerSelection,invStartIndex);
            containerSelection -> SetRedrawList(false);
          }

		  std::vector<Item*> selectedItems = containerSelection -> GetSelectedItems();
		  if (selectedItems.empty()) PrintAccessContainerHints();
		  std::vector<int> selectionIndices = containerSelection -> GetSelectedIndices();
		  for (std::vector<int>::iterator i = selectionIndices.begin(); i != selectionIndices.end(); i++) {
			ColourInvLine(*i, 2);
		  }
		  if (containerSelection -> IsSelected(selectionIndex)) {
			  HighlightInvLine(selectionIndex,2);
		  } else {
			  HighlightInvLine(selectionIndex,0);
		  }
      } else {
    	wclear(invwin_front);
    	wrefresh(invwin_front);
      }

	  inputChoice = mainUI->ConsoleGetInput();
	  containerSelection -> HandleSelection(inputChoice);
	  selectionIndex = containerSelection ->  GetSelectionIndex();
	  invStartIndex = containerSelection -> GetInvStartIndex();
      int previousSelectionIndex = containerSelection -> GetPreviousSelectionIndex();
      if (selectionIndex != previousSelectionIndex) {
    	  UnhighlightInvLine(containerSelection -> GetPreviousSelectionIndex());
      }

      wrefresh(invwin_front);
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

void InventoryUI :: HighlightInvLine(int yIndex,  int colourCode) {
    mvwchgat(invwin_front, yIndex, 0, INVWIN_FRONT_X-1 , A_REVERSE, colourCode, NULL);
    wrefresh(invwin_front);
}

void InventoryUI :: ColourInvLine(int yIndex, int colourCode) {
    mvwchgat(invwin_front, yIndex, 0, INVWIN_FRONT_X-1 , A_NORMAL, colourCode, NULL);
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

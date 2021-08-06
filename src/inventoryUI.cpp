#include "inventoryUI.h"

Command InventoryUI :: MapInputToCommand(int characterCode) {
  try {
    return commandMappings.at(characterCode);
   }
   catch (const std::out_of_range& ex) {
     logging -> logline("Unkown command code " + std::to_string(characterCode));
     return NONE;
   }
}


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

void InventoryUI::DrawOtherContainers(std::vector<Container*> otherContainers) {
  if (!otherContainers.empty()) {
    wclear(invwin_front);
    int i=0;
    std::vector<Container*>::iterator it;
    for(it = otherContainers.begin(); it != otherContainers.end(); it++) {
      Container* container = *it;
      char buffer[120];
      sprintf(buffer,"%-20s", container -> GetName().c_str());
      mainUI -> wprint_at(invwin_front, buffer, i, COL_1);
      sprintf(buffer,"%-4d", container -> GetWeight());
      mainUI -> wprint_at(invwin_front, buffer, i, COL_2);
      i++;
    }
    wrefresh(invwin_front);
  }
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

  SelectionMode selectionMode = containerSelection -> GetSelectionMode();
	std::vector<Item*> movingItems = containerSelection -> GetSelectedItems();
	int containerIndex = containerSelection -> GetContainerIndex();

	Container* container = containerSelection -> GetContainer();

	if (container->GetSize() == 0) return 1;
	if (movingItems.empty()) {
		Item* movingItem = container -> GetItem(containerIndex);
		containerSelection -> Select(containerIndex);
		mainUI->ClearConsoleAndPrint("Moving: " + movingItem -> GetName() + ". Please choose the new location and hit m again. c to choose another container. q to cancel");
	} else {
	  Item* targetItem;
	  if (SELECTING_CONTAINER == selectionMode) {
	    targetItem = containerSelection -> GetSelectedOtherContainer();
	  } else {
	    targetItem = container -> GetItem(containerIndex);
	  }
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

// TODO Refactor into singular command / function mappings
int InventoryUI::InventoryInput(ContainerSelection* containerSelection, int inputChoice) {
   int containerIndex = containerSelection -> GetContainerIndex();;
  logging -> logline("Container index: " + std::to_string(containerIndex));

  SelectionMode selectionMode = containerSelection -> GetSelectionMode();
  Container* container = containerSelection -> GetContainer();

  Command command = this -> MapInputToCommand(inputChoice);

  switch(command) {
    case (QUIT) : {
      if (SELECTING_CONTAINER != selectionMode) {
        // Selection clearing
        if (!containerSelection -> GetSelectedIndices().empty()) {
          containerSelection -> ClearSelection();
          containerSelection -> SetRedrawList(true);
          return 0;
        }
      } else {
        // Escape the container selection mode
        containerSelection -> SetSelectionMode(MOVING_ITEMS);
        containerSelection -> SetRedrawList(true);
        return 0;
      }
      return 1;
      break;
    }
    case (CONTAINER_OR_CONSOLE) : {
      if (SELECTING_CONTAINER != selectionMode) {
        if (containerSelection -> HasSelectedItems()) {
          containerSelection -> SetSelectionMode(SELECTING_CONTAINER);
          std::vector<Container*> otherContainers = containerSelection -> GetContainersNotSelected();
          if (!otherContainers.empty()) {
            this -> DrawOtherContainers(otherContainers);
            std::vector<Container*> unselectedContainers = containerSelection -> GetContainersNotSelected();
            this -> otherContainerSelection = new ItemListSelection(unselectedContainers, INVWIN_FRONT_Y, false);
          } else {
            containerSelection -> SetSelectionMode(SELECTING_ITEMS);
            mainUI -> ClearConsole();
            mainUI -> ConsolePrint("No other containers available to move these items to.", 0, 0);
          }
        } else {
          InventoryCommandInput(containerSelection);
        }
      }
      break;
    }
    case (INFO) : { //Item info
      break;
    }
    case (OPEN) : {
      if (SELECTING_CONTAINER != selectionMode) {
        OpenContainer(container, containerIndex);
        containerSelection -> SetRedrawList(true);
      }
      break;
    }
    case (TAKE) : {
      if (SELECTING_ITEMS == selectionMode) {
        if(!RootContainerIsPlayerInventory()) {
          TakeItem(container, containerIndex);
          containerSelection -> SetRedrawList(true);
        }
      }
      break;
    }
    case (DROP) : {
      if (SELECTING_ITEMS == selectionMode) {
        if(RootContainerIsPlayerInventory()) {
        // If we've not selected anything, select the currently chosen item
        if (!containerSelection -> HasSelectedItems()) {
         containerSelection -> Select(containerIndex);
        }
        this -> AttemptDropItems(containerSelection);
        containerSelection -> SetRedrawList(true);
        }
      }
      break;
    }
    case (MOVE) : {
      this -> AttemptMoveItems(containerSelection);
      break;
    }
    default:
      return 0;
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

std::vector<Container*> InventoryUI::FindOtherContainers(ContainerSelection* containerSelection) {
  std::vector<Container*> otherContainers;
  Container* openingContainer = containerSelection -> GetContainer();
  // Parent container / first opened
  if (currentContainerSelection == NULL) {
    otherContainers = openingContainer -> GetAllContainers();
  } else {
    // Pass down previous other containers
    otherContainers = currentContainerSelection -> GetOtherContainers();
    // Add the previous container
    otherContainers.push_back(currentContainerSelection -> GetContainer());

    // Remove any containers that we've selected previously
    std::vector<Container*> selectedContainers = currentContainerSelection -> GetSelectedContainers();
    for (std::vector<Container*>::iterator selectedIt=selectedContainers.begin(); selectedIt != selectedContainers.end(); selectedIt++) {
      std::vector<Container*>::iterator otherContainerFindIt = std::find(otherContainers.begin(), otherContainers.end(), *selectedIt);
      if (otherContainerFindIt != otherContainers.end()) {
        logging -> logline("Removing selected container from others list: " + (*otherContainerFindIt) -> GetName());
        otherContainers.erase(otherContainerFindIt);
      }
    }
  }

  // Remove the container we're opening
  std::vector<Container*>::iterator openingContainerFindIt = std::find(otherContainers.begin(), otherContainers.end(), openingContainer);
  if (openingContainerFindIt != otherContainers.end()) {
    logging -> logline("Removing opening container from others: " + openingContainer -> GetName());
    otherContainers.erase(openingContainerFindIt);
  }
  return otherContainers;
}

void InventoryUI::HandleSelection(ContainerSelection* containerSelection) {
  long int selectionIndex = containerSelection ->  GetSelectionIndex();
  long int invStartIndex = containerSelection -> GetInvStartIndex();
  Container* c = containerSelection -> GetContainer();
  SelectionMode selectionMode = containerSelection -> GetSelectionMode();
  if (SELECTING_ITEMS == selectionMode || MOVING_ITEMS == selectionMode) {
    int containerSize = c -> GetSize();
    if (containerSize > 0) {
        if (containerSelection -> IsRedrawList()) {
          logging -> logline("Redrawing container list..");
          DrawInventory(containerSelection, invStartIndex);
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
    long int previousSelectionIndex = containerSelection -> GetPreviousSelectionIndex();
    if (selectionIndex != previousSelectionIndex) {
      UnhighlightInvLine(previousSelectionIndex);
    }

    wrefresh(invwin_front);
  }

  if (SELECTING_CONTAINER == selectionMode) {
   long int otherContainerSelectionIndex = containerSelection -> GetOtherContainerSelectionIndex();
   HighlightInvLine(otherContainerSelectionIndex,0);
   inputChoice = mainUI->ConsoleGetInput();
   if (this -> otherContainerSelection != NULL) {
     this -> otherContainerSelection -> HandleSelection(inputChoice);
   }
   wrefresh(invwin_front);
  }
  // delete the old selection
  this -> containerSelections.pop_back();
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
    inputChoice = -1;

    // Find all known containersFindOtherContainers
    ContainerSelection* containerSelection = new ContainerSelection(c, INVWIN_FRONT_Y, playerInv);
    logging -> logline("Starting container selection for: " + c -> GetName());
    std::vector<Container*> otherContainers = FindOtherContainers(containerSelection);
    logging -> logline("--- Found other containers for: " + c -> GetName());
    for (std::vector<Container*>::iterator otherContainerIt = otherContainers.begin(); otherContainerIt != otherContainers.end(); otherContainerIt++) {
      logging -> logline((*otherContainerIt) -> GetName());
    }
    containerSelection -> SetOtherContainers(otherContainers);

    this -> containerSelections.push_back(containerSelection);
    currentContainerSelection = this -> containerSelections.back();

    DrawRearWindow(containerSelection);
    DrawInventory(containerSelection, 0);
    while(InventoryInput(containerSelection, inputChoice) == 0) {
      this -> HandleSelection(containerSelection);
    }
}

// TODO Refactor into singular command / function mappings
void InventoryUI::InventoryCommandInput(
    ContainerSelection *containerSelection) {
  std::vector<Item*> movingItems = containerSelection->GetSelectedItems();
  std::vector<int> selectedIndices = containerSelection->GetSelectedIndices();
  int containerIndex = containerSelection->GetContainerIndex();
  while (true) {
    echo();
    mainUI->ClearConsole();
    mainUI->ConsolePrint(PROMPT_TEXT, 0, 0);
    std::string lowerCasedAnswer = StringUtils::toLowerCase(
    mainUI->ConsoleGetString());
    std::vector<Item*> selectedItems = containerSelection->GetSelectedItems();
    if (lowerCasedAnswer == "drop") {
      if (RootContainerIsPlayerInventory()) {
        // If we've not selected anything, select the currently chosen item
        if (!containerSelection->HasSelectedItems()) {
          containerSelection->Select(containerIndex);
        }
        this->AttemptDropItems(containerSelection);
        containerSelection->SetRedrawList(true);
      }
    } else if (lowerCasedAnswer == "move") {
      this->AttemptMoveItems(containerSelection);
    } else if (lowerCasedAnswer == "open") {
      Container *container = containerSelection->GetContainer();
      this->OpenContainer(container, containerIndex);
      containerSelection->SetRedrawList(true);
    } else if (lowerCasedAnswer == "open" || lowerCasedAnswer == "q"
        || lowerCasedAnswer == "quit" || lowerCasedAnswer == "exit") {
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

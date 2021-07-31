#include "containerSelection.h"


std::vector<Container*> ContainerSelection :: GetSelectedContainers() {
  std::vector<Container*> containers;
  if (!this -> selectedItems.empty()) {
    for(std::vector<Item*>::iterator it = selectedItems.begin(); it != selectedItems.end(); it++) {
       Item* item = *it;
       Container* container = dynamic_cast<Container*> (item);
       if (container != NULL) {
         containers.push_back(container);
       }
    }
  }
  return containers;
}

std::vector<Container*> ContainerSelection :: GetOtherContainersNotSelected() {
  std::vector<Container*> results = std::vector<Container*>();
  for (std::vector<Container*>::iterator otherContainerIt=otherContainers.begin(); otherContainerIt != otherContainers.end(); otherContainerIt++) {
    results.push_back(*otherContainerIt);
  }

  std::vector<Container*> selectedContainers = this -> GetSelectedContainers();
  std::vector<Container*> toRemove = std::vector<Container*>();
  for (std::vector<Container*>::iterator resultIt=results.begin(); resultIt != results.end(); resultIt++) {
    logging -> logline("Checking other container" + (*resultIt) -> GetName());
    // Remove any containers that we've selected
    std::vector<Container*>::iterator selectionFindIt = std::find(selectedContainers.begin(), selectedContainers.end(), *resultIt);
    if (selectionFindIt != selectedContainers.end()) {
      logging -> logline("Removing selected other container from results: " + (*resultIt) -> GetName());
      toRemove.push_back(*resultIt);

      // Also remove any child containers that are selected
      Container* foundContainer = *selectionFindIt;
      std::vector<Container*> childContaienrs = foundContainer -> GetChildContainers();
      for (std::vector<Container*>::iterator childContainerIt=childContaienrs.begin(); childContainerIt != childContaienrs.end(); childContainerIt++) {
        std::vector<Container*>::iterator resultChildFindIt = std::find(results.begin(), results.end(), *childContainerIt);
        if (resultChildFindIt != results.end()) {
         logging -> logline("Removing selected other child container from results: " + (*resultChildFindIt) -> GetName());
         toRemove.push_back(*resultChildFindIt);
        }
      }
    }
  }

  for (std::vector<Container*>::iterator toRemoveIt=toRemove.begin(); toRemoveIt != toRemove.end(); toRemoveIt++) {
    std::vector<Container*>::iterator resultRemovalIt = std::find(results.begin(), results.end(), *toRemoveIt);
    Container* toRemoveContainer = *resultRemovalIt;
    logging -> logline("Removing other container from results: " + toRemoveContainer -> GetName());
    results.erase(resultRemovalIt);
  }
  return results;
}

void ContainerSelection :: UpdateSelection(long int newSelectionIndex) {
    this -> previousSelectionIndex = this -> selectionIndex;
    long int previousContainerIndex = this -> containerIndex;
    this -> selectionIndex = newSelectionIndex;
    this -> containerIndex = newSelectionIndex + this -> invStartIndex;

    if (this -> selectingItems && this -> containerSelectionStart == -1) {
    	// Set the selection pivot point
      this -> containerSelectionStart = newSelectionIndex + invStartIndex;
    }

    bool selectionChanged = this -> containerIndex != previousContainerIndex;
    if (selectionChanged && this -> selectingItems) {
        logging -> logline("Selection starts at: " + std::to_string(containerSelectionStart));

        bool selectingItemsAbove = this -> containerIndex < previousContainerIndex && this -> containerIndex < containerSelectionStart;
    	if (selectingItemsAbove) {
    		for (long int i=this -> containerIndex; i <= containerSelectionStart; i++) {
    			Select(i);
    		}
    	}

    	// Deselect anything we were previously selecting
    	bool reducingSelectionAbove = this -> containerIndex > previousContainerIndex && this -> containerIndex < containerSelectionStart;
    	if (reducingSelectionAbove) {
    		for (long int i=previousContainerIndex-1; i < containerIndex; i++) {
    			Deselect(i);
    		}
    	}

        bool selectingItemsBelow = this -> containerIndex > previousContainerIndex && this -> containerIndex > containerSelectionStart;
    	if (selectingItemsBelow) {
    		for (long int i=this -> containerSelectionStart; i <= containerIndex; i++) {
    			Select(i);
    		}
    	}

    	// Deselect anything we were previously selecting
    	bool reducingSelectionBelow = this -> containerIndex < previousContainerIndex && this -> containerIndex > containerSelectionStart;
    	if (reducingSelectionBelow) {
    		for (long int i= this -> containerIndex+1; i <= previousContainerIndex; i++) {
    			Deselect(i);
    		}
    	}

	}
}

void ContainerSelection :: HandleSelection(int choice) {
  int containerSize = this -> container -> GetSize();
  if (containerSize > 0) {
    //logging -> logline("WINDOW SIZE: " + std::to_string(ITEM_LINE_COUNT));
    //logging -> logline("Container SIZE: " + std::to_string(containerSize));

    int maxScrollIndex = 0;
    if (containerSize > itemViewLineCount) maxScrollIndex = (containerSize - this -> itemViewLineCount);

    int maxSelectionIndex = this -> itemViewLineCount-1;
    if (containerSize < this -> itemViewLineCount) maxSelectionIndex = this -> itemViewLineCount - (this -> itemViewLineCount - containerSize) - 1;
    //logging -> logline("maxSelectionIndex: " + std::to_string(maxSelectionIndex));

    long int newSelectionIndex = this -> selectionIndex;
    switch (choice) {
      case('\n'): {

    	if (!this -> selectingItems) {
    		// If we're hitting select on a selected item, but not in select mode, deselect it
    		if (this -> IsSelected(containerIndex)) {
    			this -> Deselect(containerIndex);
    			return;
    		}

    		this -> selectingItems = true;
    		this -> containerSelectionStart = -1;
        logging -> logline("Selecting items");
    	} else {
    		this -> selectingItems = false;
        logging -> logline("Closing selection");
    	}
        break;
      }
      case KEY_PPAGE:
        // Either page up or jump to top of current page
        if (selectionIndex==0 && invStartIndex >= itemViewLineCount) {
            newSelectionIndex = maxSelectionIndex;
            invStartIndex -= itemViewLineCount; 
            if (this -> selectingItems) SelectRange(newSelectionIndex, selectionIndex);
            this -> redrawList = true;
        } else if (selectionIndex == 0) {
            newSelectionIndex = 0;
            invStartIndex = 0; 
            this -> redrawList = true;
        } else {
          newSelectionIndex = 0;
        }
        break;
      case KEY_NPAGE:
        // Either page down or jump to bottom of current page
        if (selectionIndex == maxSelectionIndex && invStartIndex+itemViewLineCount <= maxScrollIndex) {
            newSelectionIndex = 0;
            invStartIndex += itemViewLineCount; 
            this -> redrawList = true;
        } else if (selectionIndex == maxSelectionIndex) {
            invStartIndex = maxScrollIndex; 
            this -> redrawList = true;
        } else {
          newSelectionIndex = maxSelectionIndex;
        }
        break;
      case KEY_UP:
        if (selectionIndex>0) {
            newSelectionIndex--; 
        } else if (selectionIndex==0 && invStartIndex>0) {
            this -> invStartIndex--;
            this -> redrawList = true;
        }
        break;
      case KEY_DOWN:
        if (selectionIndex < maxSelectionIndex) {
          newSelectionIndex++; 
        } else if (selectionIndex == itemViewLineCount-1 && invStartIndex < maxScrollIndex) {
          this -> invStartIndex++;
          this -> redrawList = true;
        }
        break;
    }

    this -> UpdateSelection(newSelectionIndex);
  }
}

void ContainerSelection :: HandleOtherContainerSelection(int choice) {
  int size = this -> otherContainers.size();
  if (size > 0) {
    int maxScrollIndex = 0;
    if (size > itemViewLineCount) maxScrollIndex = (size - this -> itemViewLineCount);

    int maxSelectionIndex = this -> itemViewLineCount-1;
    if (size < this -> itemViewLineCount) maxSelectionIndex = this -> itemViewLineCount - (this -> itemViewLineCount - size) - 1;
    //logging -> logline("maxSelectionIndex: " + std::to_string(maxSelectionIndex));

    long int newSelectionIndex = this -> otherContainerSelectionIndex;
    switch (choice) {
      case ('\n'): {
        Container* chosenContainer = this -> otherContainers.at(newSelectionIndex);
        if (chosenContainer != NULL) {
          this -> selectedOtherContainer = chosenContainer;
        }
        break;
      } case KEY_PPAGE:
        // Either page up or jump to top of current page
        if (otherContainerSelectionIndex==0 && otherContainerSelectionStartIndex >= itemViewLineCount) {
            newSelectionIndex = maxSelectionIndex;
            otherContainerSelectionStartIndex -= itemViewLineCount;
            this -> redrawList = true;
        } else if (selectionIndex == 0) {
            newSelectionIndex = 0;
            otherContainerSelectionStartIndex = 0;
            this -> redrawList = true;
        } else {
          newSelectionIndex = 0;
        }
        break;
      case KEY_NPAGE:
        // Either page down or jump to bottom of current page
        if (otherContainerSelectionIndex == maxSelectionIndex && otherContainerSelectionStartIndex+itemViewLineCount <= maxScrollIndex) {
            newSelectionIndex = 0;
            otherContainerSelectionStartIndex += itemViewLineCount;
            this -> redrawList = true;
        } else if (otherContainerSelectionIndex == maxSelectionIndex) {
            otherContainerSelectionStartIndex = maxScrollIndex;
            this -> redrawList = true;
        } else {
          newSelectionIndex = maxSelectionIndex;
        }
        break;
      case KEY_UP:
        if (otherContainerSelectionIndex>0) {
            newSelectionIndex--;
        } else if (otherContainerSelectionIndex==0 && otherContainerSelectionStartIndex>0) {
            this -> otherContainerSelectionStartIndex--;
            this -> redrawList = true;
        }
        break;
      case KEY_DOWN:
        if (otherContainerSelectionIndex < maxSelectionIndex) {
          newSelectionIndex++;
        } else if (otherContainerSelectionIndex == itemViewLineCount-1 && otherContainerSelectionStartIndex < maxScrollIndex) {
          this -> otherContainerSelectionStartIndex++;
          this -> redrawList = true;
        }
        break;
    }
  }
}

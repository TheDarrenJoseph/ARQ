#include "containerSelection.h"


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

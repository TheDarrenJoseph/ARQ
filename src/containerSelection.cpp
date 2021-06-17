#include "containerSelection.h"

void ContainerSelection :: HandleSelection(int choice) {
  unsigned int containerSize = this -> container -> GetSize();
  if (containerSize > 0) {
    //logging -> logline("WINDOW SIZE: " + std::to_string(ITEM_LINE_COUNT));
    //logging -> logline("Container SIZE: " + std::to_string(containerSize));

    int maxScrollIndex = 0;
    if (containerSize > itemViewLineCount) maxScrollIndex = (containerSize - this -> itemViewLineCount);

    int maxSelectionIndex = this -> itemViewLineCount-1;
    if (containerSize < this -> itemViewLineCount) maxSelectionIndex = this -> itemViewLineCount - (this -> itemViewLineCount - containerSize) - 1;
    //logging -> logline("maxSelectionIndex: " + std::to_string(maxSelectionIndex));

    long unsigned int newSelectionIndex = this -> selectionIndex;
    this -> redrawList = false;
    switch (choice) {
      case KEY_UP:
        if (selectionIndex>0) {
            newSelectionIndex--; 
        } else if (selectionIndex==0 && invStartIndex>0) {
            this -> invStartIndex--;
            this -> redrawList = true;
        }
        break;
      case KEY_DOWN:
        //Checking against the window boundary. containerSize allows
        if (selectionIndex < maxSelectionIndex) {
          newSelectionIndex++; 
        } else if (selectionIndex == itemViewLineCount-1 && invStartIndex < maxScrollIndex) {
          this -> invStartIndex++;
          this -> redrawList = true;
        }
        break;
    }

    this -> previousSelectionIndex = selectionIndex;
    this -> selectionIndex = newSelectionIndex;
    logging -> logline("maxSelectionIndex: " + std::to_string(maxSelectionIndex));
    logging -> logline("selectionIndex: " + std::to_string(selectionIndex));
    logging -> logline("invStartIndex: " + std::to_string(invStartIndex));
    logging -> logline("maxScrollIndex: " + std::to_string(maxScrollIndex));
  }
}

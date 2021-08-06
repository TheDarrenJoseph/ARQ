#include "containerSelection.h"

/**
 * Used to find containers not in the current selection
 * This allows us to find "other containers" for moving items to
 */
std::vector<Container*> ContainerSelection :: GetContainersNotSelected() {
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

#include "pathfinding.h"

int Pathfinding :: ManhattanPathCostEstimate(Position startPos, Position endPos) {
    //straight-line cost is the minimum absolute distance between the two
    int absoluteCost = (abs(endPos.x-startPos.x)+abs(endPos.y-startPos.y));
    //logging -> logline("Manhattan cost of path is: " + std::to_string(absoluteCost));
    return absoluteCost;
}

/** Part of A* Implementation.
 * Evaluates all unvisited nodes based on the heuristicCostMap (the lowest heuristic cost nodes are picked and evaluated)
 * 
 * @param currentNode the startPosition 
 * @param previousNode
 * @param endPos
 * @param visitedNodes
 * @param unvisitedNodes
 * @param navigatedNodes
 * @param nonHeuristicCostMap
 * @param heuristicCostMap
 * 
 * @return true if the destination was reached (should mean a complete path), false otherwise
 */
bool Pathfinding :: EvaluateNodes(Position currentNode, Position endPos) {
    Position previousNode = Position(-1,-1); //-1 to fail boundary check, and not be checked on the first pass-through
    
    //While we haven't run out of nodes to visit, grab the lowest cost, and try to build our path
    while(!unvisitedNodes.empty()) {
        std::map<Position,int> possibleNewNodes = heuristicCostMap; //create a new map as a copy of all heuristic scores for nodes, so we can modify it
        
        bool foundNewNode = false;
        while (!foundNewNode) {
			      //Check for the lowest node using our CompareMapLessThanCost comparison function
            std::pair<Position, int> lowestCostNode = *std::min_element(possibleNewNodes.begin(), possibleNewNodes.end(), &Pathfinding::CompareMapLessThanCost);
            currentNode = lowestCostNode.first; 
            
            //Only care if this node is unvisited
            if (unvisitedNodes.count(currentNode) == 1) {
                foundNewNode = true;
            } else if (possibleNewNodes.empty()){
                logging -> logline("Ran out of new nodes..");
                return false;
            } else {
                possibleNewNodes.erase(currentNode);
            }
            
        }
        
        //Check that we've looped once (by the pos now being valid), if so make sure we haven't repeated the node search
        if (map -> IsInBoundaries(previousNode.x,previousNode.y)) {
            if (currentNode == previousNode) {
                logging -> logline("Node repeated! Aborting pathing here..");
                return false;
            } 
        } 
        
        //Print data about the current node
        //std::cout << "Lowest cost node=" << currentNode.x << " " << currentNode.y << "\n";
        unvisitedNodes.erase(currentNode);
        visitedNodes.insert(currentNode);    
        
        //Check for reaching our goal
        if (currentNode == endPos) { 
           logging -> logline("Reached target position: " + std::to_string(endPos.x) + "," + std::to_string(endPos.y));
            return true;
        } else { 
            //Evaluate all neighbors of the current node
            //logging -> logline("Checking neighbors of: " + std::to_string(currentNode.x) + "," + std::to_string(currentNode.y));
            for (Position p : map -> GetNeighbors(currentNode.x,currentNode.y)) {
                EvaluatePathNeighborNode(p,endPos,currentNode);
            }
        }
        previousNode = currentNode;
        
    }
    
  return false;  
}

void Pathfinding :: initialiseMaps() {
  for (unsigned int x=0; x<GRID_X; x++) {
    for (unsigned int y=0; y<GRID_Y; y++) {
      heuristicCostMap[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
      nonHeuristicCostMap[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
    }
  }
}

/** Builds a path between two points, avoiding walls, currently only works with startPos<endPos (top-left to bottom-right)
 * 
 * @param startPos The position on the map to path from
 * @param endPos   The position on the map to path to
 * @param endPath a pointer to the finished path from the result of our search
 */
Path Pathfinding :: BuildPathUsingAStarSearch(Position startPos, Position endPos) {
        
        bool pathPointsInBoundaries = map -> IsInBoundaries(startPos.x,startPos.y) && map -> IsInBoundaries(endPos.x, endPos.y);
        bool pathPointsNotWalls = !map -> IsWall(startPos.x,startPos.y) && !map -> IsWall(endPos.x,endPos.y);
        Path path;
        if (pathPointsInBoundaries && pathPointsNotWalls) {
            initialiseMaps();
            unvisitedNodes.insert(Position(startPos.x,startPos.y));
            //Manhattan cost the startPos default cost (the distance between the two points absolutely)
            heuristicCostMap[startPos] = ManhattanPathCostEstimate(startPos, endPos); 
            //assign the starPos to have a cost of 0
            nonHeuristicCostMap[startPos] = 0; 

            //Start evaluating our nodes
            if (EvaluateNodes(startPos,endPos)) { 
              logging -> logline("Successfully calculated path.");
              path = ConstructPath(navigatedNodes, endPos); //Build a path to the current endPos, set as endPath 
            } else {
              logging -> logline("Failed to calculate path.");
            }
        }
    return path;
}

Path Pathfinding :: BuildPathBetweenRooms(Room* firstRoom, Room* nextRoom) {
    int firstRoomDoorCount = 0;
    int nextRoomDoorCount = 0;
    Door* firstRoomDoors = firstRoom -> getDoors(&firstRoomDoorCount);
    Door* nextRoomDoors = nextRoom -> getDoors(&nextRoomDoorCount);
  
    // TODO potentially randomise door choice
    Door firstRoomDoor = firstRoomDoors[0];
    Door nextRoomDoor = nextRoomDoors[0];
    Position startPos = Position(firstRoomDoor.posX, firstRoomDoor.posY);
    Position targetPos = Position(nextRoomDoor.posX, nextRoomDoor.posY);
    return BuildPathUsingAStarSearch(startPos, targetPos);    
}

std::list<Path> Pathfinding :: BuildPathsBetweenRooms() {
  logging -> logline("Pathfinding between rooms...");
  Room* rooms = map -> GetRooms();
  int roomCount = 3; //map -> GetRoomCount();
  std::list<Path> paths;
  for (unsigned short int i=0; i < roomCount - 1; i++) {
    logging -> logline("Pathfinding for room [" + std::to_string(i) + " / " + std::to_string(roomCount) + "]");
    Path roomToRoomPath = BuildPathBetweenRooms(&rooms[i], &rooms[i+1]);
    if (roomToRoomPath.empty()) {
      logging -> logline("Pathfinding failed for room.");
      //delete(&roomToRoomPath);
    } else {
      logging -> logline("Pathfinding successful for room.");
      paths.push_back(roomToRoomPath);
    }
  }

  logging -> logline("Pathfinding between rooms complete.");
  return paths;
}

/** Part of A* Implementation. Evaluates a single neighbor node, defined as a position in our game grid.
 * 
 * @param neighbor The position of the node to evaluate
 * @param endPos   The target position for the path
 * @param currentNode the current node, which the neighbor is adjacent to (costs go between these two)
 * @param visitedNodes a set of all visited nodes, this function will add the neighbor to this set if not already in there (The node will only be evaluated if it's not in the set)
 * @param unvisitedNodes a set of all nodes available to be evaluated, this function will add the neighbor to it so that it can become the currentNode for later evaluations (assuming currentNode is passed as the lowest cost position in the unvisited set/the start position)
 * @param navigatedNodes a map of all nodes that have been evaluated and their distances saved, acting as a linked list from any point in the map back to the original currentNode
 * @param nonHeuristicCostMap a map of integer costs from our start position (based purely on distance) (requires all nodes with int max/infinity values by default, and a value of 0 for our start position)
 * @param heuristicCostMap a map of costs using our heuristic (requires all nodes with int max/infinity values by default, and a heuristic cost applied to our start position)
 */
void Pathfinding :: EvaluatePathNeighborNode(Position neighbor, Position endPos, Position currentNode ) {
   
    //If the node has not been "visited"/evaluated, evaluate it's cost
    if (visitedNodes.count(neighbor) == 0) {
        //logging -> logline("Evaluating path neightbor node at: " + std::to_string(neighbor.x) + "," + std::to_string(neighbor.y));
        visitedNodes.insert(neighbor); //add this node straight to the visited list
        // If we cannot traverse the node, return immediately
        bool neighborTraversible = map -> IsTraversable(neighbor.x,neighbor.y);
        bool currentNodeTraversible = map -> IsTraversable(currentNode.x,currentNode.y);
        tile neighbor_tile_type = map -> GetTile(neighbor.x,neighbor.y);
        tile current_tile_type = map -> GetTile(currentNode.x,currentNode.y);
        if ((!currentNodeTraversible && ntl != current_tile_type && dor != current_tile_type) || 
            (!neighborTraversible && ntl != neighbor_tile_type && dor != neighbor_tile_type) ) return;
        
        bool bothNodesAreCorridors = neighbor_tile_type == cor && current_tile_type == cor;

        int nonHeuristicCost = nonHeuristicCostMap.at(currentNode);
        int absXDiff = abs(currentNode.x-neighbor.x);
        int absYDiff = abs(currentNode.y-neighbor.y);
        int nonHeuristicScore = (nonHeuristicCost + absXDiff + absYDiff); //add a constant factor of 1 tile distance to the current path score
  
        //If neighbor isn't in the unvisited list, add it, allows this function to evaluate the node later
        if (unvisitedNodes.count(neighbor) == 0) {
            unvisitedNodes.insert(neighbor);
        } else if (nonHeuristicScore >= nonHeuristicCost) return; //ignore this node if the fixed path score is greater than it's cost
        
        //Otherwise, save the path data
        navigatedNodes[neighbor] = currentNode; //sets the neighbors path to backtrace to the start pos
        
        nonHeuristicCostMap[neighbor] = nonHeuristicScore;
        unsigned int heuristicCostEstimate = ManhattanPathCostEstimate(neighbor, endPos);
        unsigned int heuristicCost = nonHeuristicCostMap[neighbor] + heuristicCostEstimate; //Add the manhattan costing to the non heuristic values
        heuristicCostMap[neighbor] = heuristicCost; //add the cost value to this spot on the heuristic cost map
        
    }

}
 

/** Navigates along the map of navigated nodes from a given position until the path ends, 
 *  it builds a Path of these nodes and returns it.
 * 
 * @param navigated
 * @param pathPosition
 * @return The path followed from the position passed
 */
Path Pathfinding :: ConstructPath(std::map<Position,Position> navigated, Position pathPosition) {
    Path path;
    while (navigated.count(pathPosition) > 0) {
        //std::cout << "Path node " << pathPosition.x << " " << pathPosition.y << "\n"; 
        pathPosition = navigated.at(pathPosition); //next node navigated to
        path.push_back(pathPosition);
    }
    Position startPosition = path.front();
    logging -> logline("Assembled path from: " + std::to_string(startPosition.x) + "," + std::to_string(startPosition.y) + " to " + std::to_string(pathPosition.x) + "," + std::to_string(pathPosition.y));
    return path;
}

/** Subfunction for PathRooms that checks that a suitable path exists for the player to traverse the level
 * 
 */
bool Pathfinding :: LevelPathValid() {
    Path levelTraversalPath = BuildPathUsingAStarSearch(map -> GetEntryPosition(), map -> GetExitPosition());
    bool pathFound = !levelTraversalPath.empty();
    //qdelete &levelTraversalPath;
    return pathFound;
}

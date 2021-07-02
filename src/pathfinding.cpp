#include "pathfinding.h"

int Pathfinding :: ManhattanPathCostEstimate(Position startPos, Position endPos) {
    //straight-line cost is the minimum absolute distance between the two
    int absoluteCost = (abs(endPos.x-startPos.x)+abs(endPos.y-startPos.y));
    //logging -> logline("Manhattan cost of path is: " + std::to_string(absoluteCost));
    return absoluteCost;
}

Position Pathfinding :: findLowestCostPosition() {
  std::map<Position,int> possiblePositions = fScores;
  Position newPosition = Position(-1,-1);
  if (possiblePositions.empty()){
      logging -> logline("Ran out of new nodes..");
      return newPosition;
  } 

  bool foundNewNode = false;
  while (!foundNewNode) {
      //Check for the lowest node using our CompareMapLessThanCost comparison function
      std::pair<Position, int> lowestCostNode = *std::min_element(possiblePositions.begin(), possiblePositions.end(), &Pathfinding::CompareMapLessThanCost);
      newPosition  = lowestCostNode.first; 
      if (map -> IsPaveable(newPosition) && unvisitedNodes.count(newPosition) == 1) {
          return newPosition;
      } else {
          possiblePositions.erase(newPosition);
      }
  }
  return newPosition;
}

/** Part of A* Implementation.
 * @return true if the destination was reached (should mean a complete path), false otherwise
 */
bool Pathfinding :: EvaluateNodes(Position startPos, Position endPos) {
    logging -> logline("Evaluating nodes for path: " + std::to_string(startPos.x) + "," + std::to_string(startPos.y) + " - " + std::to_string(endPos.x) + "," + std::to_string(endPos.y));
    //While we haven't run out of nodes to visit, grab the lowest cost, and try to build our path
    while(!unvisitedNodes.empty()) {
        Position currentPos = findLowestCostPosition();
        if (map -> IsPaveable(currentPos)) {
          //Check for reaching our goal
          if (currentPos == endPos) { 
             logging -> logline("Reached target position: " + std::to_string(endPos.x) + "," + std::to_string(endPos.y));
             return true;
          } else { 
              //Evaluate all neighbors of the current node
              logging -> logline("Checking neighbors of: " + std::to_string(currentPos.x) + "," + std::to_string(currentPos.y));
              for (Position p : map -> GetNeighbors(currentPos.x,currentPos.y)) {
                  EvaluatePathNeighborNode(p,endPos,currentPos);
              }
          }
        } 
        unvisitedNodes.erase(currentPos);
    }
    logging -> logline("Ran out of nodes to calculate path: " + std::to_string(startPos.x) + "," + std::to_string(startPos.y) + " - " + std::to_string(endPos.x) + "," + std::to_string(endPos.y));
    return false;  
}

void Pathfinding :: InitialiseMaps(Position startPos, Position endPos) {
  for ( int x=0; x < map -> GetGridX(); x++) {
    for ( int y=0; y < map -> GetGridY(); y++) {
      //logging -> logline("Initialising scores for pos " + std::to_string(x) + "," + std::to_string(y));
      if (startPos.x == x && startPos.y == y) {
        gScores[startPos] = 0;
        fScores[startPos] = ManhattanPathCostEstimate(startPos, endPos); 
      } else {
        fScores[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
        gScores[Position(x,y)] = std::numeric_limits<int>::max(); //~infinity default values to ensure valid comparisons
      }
    }
  }
  unvisitedNodes.clear();
  navigatedNodes.clear();
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
  if (pathPointsInBoundaries && pathPointsNotWalls) {
      InitialiseMaps(startPos, endPos);
      unvisitedNodes.insert(Position(startPos.x,startPos.y));
      //assign the starPos to have a cost of 0
      //assign the heuristic estimate cost of taking this path from startPos to the endNode

      //Start evaluating our nodes
      if (EvaluateNodes(startPos,endPos)) { 
        logging -> logline("Successfully calculated path.");
        return ConstructPath(endPos); //Build a path to the current endPos, set as endPath 
      } else {
        logging -> logline("Failed to calculate path.");
      }
  }
  return Path();    
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
  int roomCount = map -> GetRoomCount();
  std::list<Path> paths;
  for ( short int a=0; a < roomCount - 1; a++) {
      Room* roomA = &rooms[a];
      Room* roomB = &rooms[a+1];
      logging -> logline("Pathfinding for room [" + std::to_string(a) + " / " + std::to_string(roomCount) + "]");
      Path roomToRoomPath = BuildPathBetweenRooms(roomA, roomB);
      if (roomToRoomPath.empty()) {
        logging -> logline("Failed to path room " + std::to_string(a) + " to room " + std::to_string(a+1));
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
void Pathfinding :: EvaluatePathNeighborNode(Position neighbor, Position endPos, Position currentPos) {
  
    //If the node has not been "visited"/evaluated, evaluate it's cost
    if (unvisitedNodes.count(neighbor) == 0) {
        if (gScores.count(currentPos) == 0) {
          logging -> logline("No gScore cost for current pos: " + std::to_string(currentPos.x) + "," + std::to_string(currentPos.y));
          return;
        }
        if (gScores.count(neighbor) == 0) {
          logging -> logline("No gScore cost for neighbor pos: " + std::to_string(neighbor.x) + "," + std::to_string(neighbor.y));
          return;
        }

        //logging -> logline("Evaluating path neightbor node at: " + std::to_string(neighbor.x) + "," + std::to_string(neighbor.y));
        int tentativeGScore = gScores.at(currentPos);
        int currentNeighborGScore = gScores.at(neighbor);
        //logging -> logline("Tentative gScore: " + std::to_string(tentativeGScore));
        //logging -> logline("Curent neighbor gScore: " + std::to_string(currentNeighborGScore));
        if (tentativeGScore < currentNeighborGScore) {
          //logging -> logline("SAVING Tentative gScore for neighbor: " + std::to_string(tentativeGScore));
          navigatedNodes[neighbor] = currentPos; //sets the neighbors path to backtrace to the start pos
          gScores[neighbor] = tentativeGScore;
           int heuristicNeighborCostEstimate = ManhattanPathCostEstimate(neighbor, endPos);
           int heuristicCost = gScores[neighbor] + heuristicNeighborCostEstimate; 
          fScores[neighbor] = heuristicCost; //add the cost value to this spot on the heuristic cost map
          //logging -> logline("SAVING Heuristic fScore for neighbor: " + std::to_string(heuristicCost));
          //If neighbor isn't in the unvisited list, add it, allows this function to evaluate the node later
          unvisitedNodes.insert(neighbor);          
        } else if (tentativeGScore != std::numeric_limits<int>::max() && currentNeighborGScore != std::numeric_limits<int>::max()) {
          logging -> logline("Tentative gScore: " + std::to_string(tentativeGScore) +" >= current neighbor gScore: " + std::to_string(currentNeighborGScore));
        }
    }

}
 
Path Pathfinding :: ConstructPath(Position endPos) {
    //std::set<Position> pathNodes;
    Path* path = new Path();
    Position* pathPositionPtr = new Position(endPos);
    Position pathPosition = *pathPositionPtr;
    while (navigatedNodes.count(pathPosition) > 0) {
        logging -> logline("Building path " + std::to_string(pathPosition.x) + "," + std::to_string(pathPosition.y));
        path -> push_back(pathPosition);
        //pathNodes.insert(*pos);
        // Avoid looping paths
        //if (pathNodes.count(*pos) == 0) return *path;
        pathPosition = navigatedNodes.at(pathPosition); //next node navigated to
        //navigatedNodes.erase(pathPosition);
    }
    Position startPosition = path -> front();
    logging -> logline("Assembled path from: " + std::to_string(startPosition.x) + "," + std::to_string(startPosition.y) + " to " + std::to_string(pathPosition.x) + "," + std::to_string(pathPosition.y) + " using " + std::to_string(path -> size()) + " path nodes");
    return *path;
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

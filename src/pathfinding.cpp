#include "pathfinding.h"

int Pathfinding :: ManhattanPathCostEstimate(Position startPos, Position endPos) {
    //straight-line cost is the minimum absolute distance between the two
    int absoluteCost = (abs(endPos.x-startPos.x)+abs(endPos.y-startPos.y));
    //std::cout << "abs cost of \n" << absoluteCost;
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
bool Pathfinding :: EvaluateNodes(Position currentNode, Position endPos ) {
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
                std::cout << "Ran out of new nodes.. \n";
                return false;
            } else {
                possibleNewNodes.erase(currentNode);
            }
            
        }
        
        //Check that we've looped once (by the pos now being valid), if so make sure we haven't repeated the node search
        if (map -> IsInBoundaries(previousNode.x,previousNode.y)) {
            if (currentNode == previousNode) {
                std::cout << "Node repeated! Aborting pathing here..\n";
                return false;
            } 
        } 
        
        //Print data about the current node
        //std::cout << "Lowest cost node=" << currentNode.x << " " << currentNode.y << "\n";
        unvisitedNodes.erase(currentNode);
        visitedNodes.insert(currentNode);    
        
        //Check for reaching our goal
        if (currentNode == endPos) { 
            return true;
        } else { //Evaluate all neighbors of the current node
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
bool Pathfinding :: AStarSearch(Position startPos, Position endPos, Path* endPath) {
        
        bool pathPointsInBoundaries = map -> IsInBoundaries(startPos.x,startPos.y) && map -> IsInBoundaries(endPos.x, endPos.y);
        bool pathPointsNotWalls = !map -> IsWall(startPos.x,startPos.y) && !map -> IsWall(endPos.x,endPos.y);
        if (pathPointsInBoundaries && pathPointsNotWalls) {
            initialiseMaps();
            unvisitedNodes.insert(Position(startPos.x,startPos.y));
            //Manhattan cost the startPos default cost (the distance between the two points absolutely)
            heuristicCostMap[startPos] = ManhattanPathCostEstimate(startPos, endPos); 
            //assign the starPos to have a cost of 0
            nonHeuristicCostMap[startPos] = 0; 

            //Start evaluating our nodes
            if (EvaluateNodes(startPos,endPos)) { 
              ConstructPath(navigatedNodes, endPos, endPath); //Build a path to the current endPos, set as endPath 
              return true;
            } 
        }
    
    return false;
}

bool Pathfinding :: PathRooms() { 
  Room* rooms = map -> GetRooms();
  Room initialRoom = map -> GetRooms()[0];
  int doorCount=0;
  Door* startDoors = initialRoom.getDoors(&doorCount);

     for (unsigned short int roomNo=0; roomNo< map -> GetRoomCount(); roomNo++) {

         unsigned short int startX = startDoors[0].posX;
         unsigned short int startY = startDoors[0].posY;
         unsigned short int targetX = startX;
         unsigned short int targetY = startY;
         
         Position startPos = Position(startX,startY);
         Position targetPos = startPos;
         
         int targetDoorCount;
         Door* targetDoors = rooms[roomNo].getDoors(&targetDoorCount);
         
         for (unsigned short int doorNo=0; doorNo<targetDoorCount; doorNo++) {
             targetX = targetDoors[doorNo].posX;
             targetY = targetDoors[doorNo].posY;
             
             targetPos = Position(targetX,targetY);
             
             Path* doorPath = new Path();
             
             if (AStarSearch(startPos,targetPos,doorPath) == true) {
                 for (Position pos : (*doorPath) ) {
                    // TODO Check tile? tile tileType = map -> GetTile(pos.x, pos.y);
                    map -> SetTile(pos.x, pos.y, cor);
                 }
             }
             
             delete (doorPath);
             
         }
                  
     }
  
  //Return based on whether or not you can reach the exit
  //return LevelPathValid();
  return true;
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
        visitedNodes.insert(neighbor); //add this node straight to the visited list

        //std::cout << "evaluating " << neighbor.x << ","<< neighbor.y << "\n";
        
        int nonHeuristicCost = nonHeuristicCostMap.at(currentNode);
        int absXDiff = abs(currentNode.x-neighbor.x);
        int absYDiff = abs(currentNode.y-neighbor.y);
        int nonHeuristicScore = (nonHeuristicCost + absXDiff + absYDiff); //add a constant factor of 1 tile distance to the current path score
        if (map -> IsWall(neighbor.x,neighbor.y) || map -> IsWall(currentNode.x,currentNode.y)) return; //nonHeuristicScore = std::numeric_limits<int>::max(); //Increase cost for walls
        
        //If neighbor isn't in the unvisited list, add it, allows this function to evaluate the node later
        if (unvisitedNodes.count(neighbor) == 0) {
            unvisitedNodes.insert(neighbor);
        } else if (nonHeuristicScore >= nonHeuristicCost) return; //ignore this node if the fixed path score is greater than it's cost
        
        //Otherwise, save the path data
        // std::cout << "saving " << neighbor.x << neighbor.y << "\n";
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
void Pathfinding :: ConstructPath(std::map<Position,Position> navigated, Position pathPosition, Path* endPath) {
    while (navigated.count(pathPosition) > 0) {
        //std::cout << "Path node " << pathPosition.x << " " << pathPosition.y << "\n"; 
        pathPosition= navigated.at(pathPosition); //next node navigated to
        (*endPath).push_back(pathPosition);
    }
}

/** Subfunction for PathRooms that checks that a suitable path exists for the player to traverse the level
 * 
 */
bool Pathfinding :: LevelPathValid() {
    Path* levelPath = new Path();
    bool isValid = AStarSearch(map -> GetEntryPosition(), map -> GetExitPosition(), levelPath);
    delete levelPath;
    return isValid;
}

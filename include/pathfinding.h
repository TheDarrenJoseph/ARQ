#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <cstdlib>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <limits>
#include <stdbool.h>
#include <algorithm>

#include "position.h"
#include "logging.h"
#include "map.h"

class Pathfinding {
  public:
    Map* map;
    std::set<Position> visitedNodes;
    std::set<Position> unvisitedNodes;
    std::map<Position,Position> navigatedNodes;
    std::map<Position,int> fScores;
    // Costs with our 
    std::map<Position,int> gScores;

    Path BuildPathBetweenRooms(Room* firstRoom, Room* nextRoom);
    std::list<Path> BuildPathsBetweenRooms();
    bool LevelPathValid();

    Pathfinding(Map* map) : map(map) {}

  private:
    Logging* logging = &logging -> getInstance();
    bool EvaluateNodes(Position currentNode, Position endPos);
    void EvaluatePathNeighborNode(Position neighbor, Position endPos, Position currentNode);
    void InitialiseMaps(Position startPos, Position endPos);
    int ManhattanPathCostEstimate(Position startPos, Position endPos);
    Position findLowestCostPosition();
    Path BuildPathUsingAStarSearch(Position startPos, Position endPos);
    Path ConstructPath(std::map<Position,Position> navigated, Position pathPosition);

    //Compares the pairs within a map based on their rvalues, returns true if lval is < than the rval
    static bool CompareMapLessThanCost(std::pair<Position,int> lval, std::pair<Position,int> rval) {
        return (lval.second < rval.second); //compares the int costs of the two PositionCosts, allowing sorting of positions by their movement cost
    }

};

#endif

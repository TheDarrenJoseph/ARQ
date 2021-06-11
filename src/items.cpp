#include "items.h"

unsigned long long int Item::latestId = 0;

void Item::SetName(std::string name) { 
  this -> name = name; 
} 

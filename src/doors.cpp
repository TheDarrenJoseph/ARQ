#include "doors.h"

Door WOODEN_DOOR = *new Door("Wooden Door", 0, 100);
Door IRON_DOOR   = *new Door("Iron Door", 1, 150);
Door STEEL_DOOR  = *new Door("Steel Door", 2, 200);

void Door::Open() {
  if (!this -> isOpen) this -> isOpen = true;
}

void Door::Close() {
  if (this -> isOpen)  this -> isOpen = false;

}

void Door::Lock() {
  if (!this -> isLocked) this -> isLocked = true;
}

void Door::Unlock() {
  if (this -> isLocked)  this -> isLocked = false;
}

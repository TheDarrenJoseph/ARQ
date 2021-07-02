#ifndef PLAYERUI_H
#define	PLAYERUI_H

#include <string>
#include <sstream>

#include "cursesUI.h"
#include "inventoryUI.h"
#include "playerInventoryFunctions.h"
#include "map.h"
#include "characters.h"
#include "position.h"
#include "stringUtils.h"

class PlayerUI : public InventoryFunctions {
private:
    Logging* logging = &logging -> getInstance();
    void ProcessMovementInput(int choice, Position* p);

public:
    UI* mainUI = NULL;
    InventoryUI* inventoryUI = NULL;

    Map* map = NULL;
    
    NPC* npcs = NULL;
    Player* player = NULL;

    int MAX_NPCS=0;
    
    void Battle (int npc_id);
    int BattleTurn ();
    
    int processYesOrNoChoice(std::string choice);
    int DoorProc (int y, int x);
    void LockProc(int door_x, int door_y);
    
    void PlayerContainerProc(Player* p, Container* container);
    int PlayerItemProc(Player* p, Item* itm, Position itemPosition);
    void PlayerMoveTurn (int y, int x, bool* levelEnded, bool* newLevel);
    void Interact();
    bool TextInput();
    bool InventoryInput(int choice, int index,  Container* c, bool playerInv);
    void ShowControls();
    bool Input (bool* levelEnded, bool* newLevel);
    void DrawNPCS();
    void DrawPlayer();
    void AccessPlayerInv();
    void DrawPlayerEquipment();
    
    void TileProc(tile t);

    // PlayerInventoryFunctions
    int TakeItem(Container* container, int index);
    int DropPlayerItem(Item* thisItem);
    int MoveItem(Item* item,  Item* targetItem);

    PlayerUI(int maxNPCS, UI* mainUI, Map* map, Player* player, NPC* npcs){
        MAX_NPCS = maxNPCS;
        
        this->mainUI = mainUI;
        this->inventoryUI = new InventoryUI(mainUI, player -> GetInventory(), this);
        this->map = map;
        this->player = player;
        this->npcs = npcs;
    }

    ~PlayerUI() {
      free(this -> inventoryUI);
    }
};




#endif	/* PLAYERUI_H */


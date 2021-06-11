#ifndef PLAYERUI_H
#define	PLAYERUI_H

#include <string>
#include <sstream>

#include "cursesUI.h"
#include "map.h"
#include "characters.h"
#include "position.h"

#define PROMPT_TEXT "ARQ: "

class PlayerUI {
private:
    Logging* logging = &logging -> getInstance();
public:
    UI* mainUI = NULL;
    
    Map* map = NULL;
    
    NPC* npcs = NULL;
    Player* player = NULL;

    int MAX_NPCS=0;
    
    void Battle (int npc_id);
    int BattleTurn (int npc_id);
    
    int processYesOrNoChoice(std::string choice);
    int DoorProc (int y, int x);
    void LockProc(int door_x, int door_y);
    
    void PlayerContainerProc(Player* p, Container* container);
    int PlayerItemProc(Player* p, const Item* itm, Position itemPosition);
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

//    int AccessArea (Inventory* a);
    void AccessListCommand(Container* c,int index, bool playerInv);
    void PrintAccessContainerHints();
    void AccessContainer (Container* c,bool playerInv);

    
    void TileProc(tile t);
    
    PlayerUI(int maxNPCS, UI* mainUI, Map* map, Player* player, NPC* npcs){
        MAX_NPCS = maxNPCS;
        
        this->mainUI = mainUI;
        this->map = map;
        this->player = player;
        this->npcs = npcs;
    }
};




#endif	/* PLAYERUI_H */


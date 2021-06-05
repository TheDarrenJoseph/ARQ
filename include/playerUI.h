#ifndef PLAYERUI_H
#define	PLAYERUI_H

#include <string>
#include <sstream>

#include "cursesUI.h"
#include "map.h"
#include "characters.h"

#define PROMPT_TEXT "ARQ: "

class PlayerUI {
    
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
    void LockProc(int door_y, int door_x);
    
    int PlayerItemProc(Player* p, Item* itm, int x, int y);
    void PlayerMoveTurn (int y, int x, bool* levelEnded, bool* newLevel);
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
    void AccessContainer (Container* c,bool playerInv);

    
    void TileProc(int y, int x,tile t);
    
    PlayerUI(int maxNPCS, UI* mainUI, Map* map, Player* player, NPC* npcs){
        MAX_NPCS = maxNPCS;
        
        this->mainUI = mainUI;
        this->map = map;
        this->player = player;
        this->npcs = npcs;
    }
};




#endif	/* PLAYERUI_H */


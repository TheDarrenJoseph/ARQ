#ifndef PLAYERUI_H
#define	PLAYERUI_H

#include "cursesUI.h"
#include <string>
#include <sstream>

#define PROMPT_TEXT "ARQ: "

class PlayerUI {
    
public:
    UI* mainUI;
    
    Map* map;
    
    NPC* npcs;
    Player* player;
    
    int MAX_NPCS;
    
    void Battle (int npc_id);
    int BattleTurn (int npc_id);
    
    int DoorProc (int y, int x, tile doortype);
    void LockProc(int door_y, int door_x, tile doortype, int doortile, std::string doorname);
    
    void PlayerMoveTurn (int y, int x);
    bool TextInput();
    void ShowControls();
    bool Input ();
    void DrawNPCS();
    void DrawPlayer();
    void AccessPlayerInv();
    void DrawPlayerEquipment();

//    int AccessArea (Inventory* a);
    void accessListCommand(Container* c,int index);
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


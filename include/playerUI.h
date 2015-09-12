#ifndef PLAYERUI_H
#define	PLAYERUI_H

#include "grid.h"
#include "characters.h"
#include "ui.h"

class PlayerUI : public CursesUI {
    
public:
    int BattleTurn (int npc_id);
    int DoorProc (int y, int x, tile doortype);
    
    void PlayerMoveTurn (int y, int x);
    bool TextInput();
    void ShowControls();
    bool Input ();
    void DrawNPCS();
    void DrawPlayer();
    
    void ShowNotification(const char* text);
    
    int Menu(const char** text, int buttons);
   
    int AccessArea (area* a);
    int AccessContainer (container* c);
    int AccessInventory ();
    
    void TileProc(int y, int x,tile t);
    
    PlayerUI(int maxNPCS, NPC* npcs, Player* p, Map* m) : CursesUI(maxNPCS, npcs, p, m){
        
    }
};




#endif	/* PLAYERUI_H */


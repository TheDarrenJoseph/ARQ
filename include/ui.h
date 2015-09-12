#ifndef UI_H
#define UI_H

#include <string>
#include "grid.h"

class CursesUI
{
    private:

    WINDOW* titlewin; //Creates the stats window for content

    WINDOW* mainwin_rear; //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
    WINDOW* mainwin_front; //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.

    WINDOW* consolewin_rear; //Creates the console window for deco
    WINDOW* consolewin_front; //Creates the console window for content

    WINDOW* statwin_rear; //Creates the stat window for deco
    WINDOW* statwin_front; //Creates the stats window for content

    WINDOW* invwins_front[3][3];
    WINDOW* invwins_rear; 

    WINDOW* equipwin_rear; 
    WINDOW* equipwin_outfit; 
    WINDOW* equipwin_front[3];
  
    
    int MAX_NPCS;
    
    int UI_X = GRID_X*2+7;
    int UI_Y = GRID_Y+6;
    
    NPC* npcs;
    Player* player;
    Map* map;
  
    public :
    void InitWindows();
    void DestroyWindows();
    void DecorateWindows();


    void DrawItems(Map* m);
    void DrawAreas(Map* m);

    void DrawPlayerInv();
    void DrawPlayerEquip();
    void DrawPlayerStats ();

    void DrawInv(container* c);
    void DrawInv(area* a);

    int DrawMap (Map* m, bool fogOfWar);
    void DrawCharacter (int x, int y, int colour, char symbol);


    void Info();

    void UpdateUI();

    int wprintw_col (WINDOW* winchoice, const char *text, int color_choice);
    int wprint_at (WINDOW* winchoice, const char *text, int pos_y, int pos_x);
    int wprintNoRefresh (WINDOW* win, std::string text);

    int init_screen ();
    
    //Player based functions
    void Battle (int npc_id);

    item* GetFromInventory ();
    
    int ItemProc (item* itm, int x, int y);
    int LockProc (int door_y, int door_x, tile doortype, int doortile, std::string doorname );
    void TileProc(int y, int x,tile t);

    CursesUI(int maxNPCS, NPC* npcs, Player* p, Map* m) {
        MAX_NPCS = maxNPCS;
        this->npcs = npcs;
        player = p;
        map = m;

        raw();  //stops needing [Enter] to input text
    }
};

#endif

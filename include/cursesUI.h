#ifndef CURSES_UI_H
#define CURSES_UI_H

#include <string>

#include "ui.h"

#define MAINWIN_REAR_X     GRID_X+2
#define MAINWIN_REAR_Y     GRID_Y+2
#define MAINWIN_FRONT_X    GRID_X
#define MAINWIN_FRONT_Y    GRID_Y

#define CONSOLEWIN_REAR_X  GRID_X*2+7 
#define CONSOLEWIN_REAR_Y  6
#define CONSOLEWIN_FRONT_X GRID_X*2+5
#define CONSOLEWIN_FRONT_Y 4

#define STATWIN_REAR_X  GRID_X+4
#define STATWIN_REAR_Y  5
#define STATWIN_FRONT_X GRID_X-2
#define STATWIN_FRONT_Y 3

#define INVWINS_REAR_X GRID_X+4
#define INVWINS_REAR_Y 7

#define EQUIPWIN_REAR_X GRID_X+4
#define EQUIPWIN_REAR_Y 5

#define INV_ITEM_Y 10 //The number of chars wide an inventory slot should be (How much text it can hold)

class CursesUI : public UI {
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

    int UI_X = GRID_X * 2 + 7;
    int UI_Y = GRID_Y + 6;

public:
    int wprintw_col(WINDOW* winchoice, const char* text, int color_choice);
    int wprint_at(WINDOW* winchoice, const char* text, int pos_y, int pos_x);
    int wprintNoRefresh(WINDOW* win, std::string text);

    Item* AccessPlayerInventory(Player* p);
    int PlayerItemProc(Player* p, Item* itm, int x, int y);

    void TileProc(int y, int x, tile t);


    virtual int InitScreen();
    virtual void InitWindows();
    virtual void DestroyWindows();
    virtual void DecorateWindows();

    virtual void DrawItems(Map* m);
    virtual void DrawAreas(Map* m);

    virtual void DrawPlayerEquipmentSlot(int slot, std::string name);
    virtual void DrawPlayerStats(std::string name, int health, int loot);

    virtual void DrawInv(Container* c);
    virtual void DrawInv(Area* a);
    virtual void ClearInvHighlighting();
    virtual void HighlightInv(int loc_x, int loc_y);
    virtual void DrawInvWindow(int loc_x, int loc_y, const char* tileChar, int colour);

    virtual int DrawMap(Map* m, bool fogOfWar, int playerX, int playerY, int viewDistance);
    virtual void DrawCharacter(int x, int y, int colour, char symbol);

    virtual void ShowInfo();
    virtual void ShowNotification(const char* text);
    virtual int Menu(const char** text, int buttons);

    virtual void UpdateUI();

    virtual void ClearConsole();
    virtual void ConsolePrint(std::string text, int posX, int posY);
    virtual void ConsolePrintWithWait(std::string text, int posX, int posY);
    virtual int ConsoleGetInput();
    virtual std::string ConsoleGetString();
    
    virtual void ClearConsoleHighlighting();
    virtual void HighlightConsole(int scr_x, int scr_y);

    CursesUI() {
        raw(); //stops needing [Enter] to input text
    }
    

};

#endif

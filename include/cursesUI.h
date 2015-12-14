#ifndef CURSES_UI_H
#define CURSES_UI_H

#include <string>
#include "ui.h"
 
#define MAINWIN_REAR_X     getMaxX()
#define MAINWIN_REAR_Y     getMaxY()
#define MAINWIN_FRONT_X    MAINWIN_REAR_X-2
#define MAINWIN_FRONT_Y    MAINWIN_REAR_Y-2

#define CONSOLEWIN_REAR_X  MAINWIN_REAR_X-2
#define CONSOLEWIN_REAR_Y  6
#define CONSOLEWIN_FRONT_X CONSOLEWIN_REAR_X-2
#define CONSOLEWIN_FRONT_Y CONSOLEWIN_REAR_Y-2

#define INVWIN_REAR_X MAINWIN_REAR_X
#define INVWIN_REAR_Y MAINWIN_REAR_Y-CONSOLEWIN_REAR_Y

#define INVWIN_FRONT_X INVWIN_REAR_X-2
#define INVWIN_FRONT_Y INVWIN_REAR_Y-2

#define INV_ITEM_Y 10 //The number of chars wide an inventory slot should be (How much text it can hold)

class CursesUI : public UI {
private:

    WINDOW* titlewin; //Creates the stats window for content

    WINDOW* mainwin_rear; //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
    WINDOW* mainwin_front; //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.

    WINDOW* consolewin_rear; //Creates the console window for deco
    WINDOW* consolewin_front; //Creates the console window for content

    WINDOW* invwin_front;
    WINDOW* invwin_rear;
    
    
    
    
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
    virtual void DrawContainers(Map* m);

    virtual void DrawPlayerStats(std::string name, int health, int loot);

    void HighlightInvList(int i, int max_x, int max_y);
    virtual void ListInv(Container* c,long unsigned int invIndex);
//    virtual void ListInv(Inventory* a);
    virtual void ClearInvHighlighting();
    virtual void HighlightInv(int index);

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

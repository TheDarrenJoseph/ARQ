#ifndef CURSES_UI_H
#define CURSES_UI_H

#include <string>
#include <iostream>
#include <curses.h>  
#include "ui.h"
#include "tile.h"
#include "position.h"
 
#define MAINWIN_REAR_X     getmaxx(stdscr)-2
#define MAINWIN_REAR_Y     getmaxy(stdscr)-1
#define MAINWIN_FRONT_X    MAINWIN_REAR_X-2
#define MAINWIN_FRONT_Y    MAINWIN_REAR_Y-2

#define CONSOLEWIN_REAR_X  MAINWIN_REAR_X-2
#define CONSOLEWIN_REAR_Y  6
#define CONSOLEWIN_FRONT_X CONSOLEWIN_REAR_X-2
#define CONSOLEWIN_FRONT_Y CONSOLEWIN_REAR_Y-2

#define INVWIN_REAR_X MAINWIN_REAR_X-2
#define INVWIN_REAR_Y MAINWIN_REAR_Y-CONSOLEWIN_REAR_Y-1

#define INVWIN_FRONT_X INVWIN_REAR_X-3
#define INVWIN_FRONT_Y INVWIN_REAR_Y-3

#define INV_ITEM_Y 10 //The number of chars wide an inventory slot should be (How much text it can hold)

class CursesUI : public UI {
private:

    WINDOW* titlewin=NULL; //Creates the stats window for content

    WINDOW* mainwin_rear =NULL; //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
    WINDOW* mainwin_front=NULL; //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.

    WINDOW* consolewin_rear=NULL; //Creates the console window for deco
    WINDOW* consolewin_front=NULL; //Creates the console window for content

    WINDOW* invwin_front=NULL;
    WINDOW* invwin_rear=NULL;
        
public:
    
    int wprintw_col(WINDOW* winchoice, const char* text, int color_choice);
    int wprint_at(WINDOW* winchoice, const char* text, int pos_y, int pos_x);
    int wprintNoRefresh(WINDOW* win, std::string text);

    Item* AccessPlayerInventory(Player* p);

    void TileProc(int y, int x, tile t);

    virtual int InitScreen();
    virtual void InitWindows();
    virtual void DestroyWindows();
    virtual void DecorateWindows();

    virtual void DrawItems(Map* m);
    virtual void DrawContainers(Map* m);

    virtual void DrawPlayerStats(std::string name, int health, unsigned long int loot, unsigned long int levelIndex);

    virtual void HighlightInv(int xChars, int xIndex, int yIndex);
    virtual void HighlightInvLine(int index);
    virtual void UnhighlightInvLine(int yIndex);

    virtual void ListInv(Container* c,long unsigned int invIndex);
//    virtual void ListInv(Inventory* a);
    virtual void ClearInvHighlighting();
   
    virtual void CalculateViewBoundaries(Position playPos, int viewDistance, Position* viewStart, Position* viewEnd);
    virtual int DrawMap(Map* m, bool fogOfWar, Position playerPos);
    virtual void DrawCharacter(int x, int y, int colour, char symbol);

    virtual void ShowInfo();
    virtual void ShowNotification(const char* text);
    virtual int Menu(std::vector<std::pair<std::string,std::string>> items);

    virtual void UpdateUI();

    virtual void ClearConsole();
    virtual void ClearConsoleAndPrint (std::string text);
    virtual void ConsolePrint(std::string text, int posX, int posY);
    virtual int ConsolePrintWithWait(std::string text, int posX, int posY);
    virtual int ConsoleGetInput();
    virtual std::string ConsoleGetString();
    
    virtual void ClearConsoleHighlighting();
    virtual void HighlightConsole(int scr_x, int scr_y);
    virtual void UpdateVisibleTiles(Map* map, Position playerPos);
    virtual void EnableFogOfWar(Map* map, Position playerPosition);
    virtual void DisableFogOfWar(Map* map);

    void copyUI(const CursesUI& ui) {
        this->consolewin_front = ui.consolewin_front;
        this->consolewin_rear = ui.consolewin_rear;
        this->invwin_front = ui.invwin_front;
        this->invwin_rear = ui.invwin_rear;
        this->mainwin_front = ui.mainwin_front;
        this->mainwin_rear = ui.mainwin_rear;
        this->titlewin = ui.titlewin;
    }
    
    //Overriding assignment operator
    CursesUI& operator=(const CursesUI& ui) {  
        copyUI(ui);
        return *this;
    }
    
    //Copy constructor  
    CursesUI(const CursesUI& ui) {  
        copyUI(ui);
    }
    
    CursesUI() {
        raw(); //stops needing [Enter] to input text
    }
    

};

#endif

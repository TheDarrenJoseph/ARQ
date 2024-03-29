#ifndef UI_H
#define	UI_H

#include <curses.h>  
#include <vector>
#include "position.h"
#include "map.h"
#include "containerSelection.h"

#define PROMPT_TEXT "ARQ: "


class UI {
public:
    virtual int wprintw_col(WINDOW* winchoice, const char* text, int color_choice) = 0;
    virtual int wprintw_col_char(WINDOW* winchoice, char symbol, int color_choice) = 0;
    virtual int wprint_at(WINDOW* winchoice, const char* text, int pos_y, int pos_x) = 0;
    virtual int wprintNoRefresh(WINDOW* win, std::string text) = 0;

    virtual void InitWindows() = 0;
    virtual void DestroyWindows() = 0;
    virtual void DecorateWindows() = 0;

    virtual void DrawItems(Map* m) = 0; //Draw an assortment of items that will only ever have items (e.g inside of a chest)
    virtual void DrawContainers(Map* m) = 0; //Draw an assortment of items that may contain containers (e.g a room)

    virtual void DrawPlayerStats (std::string name, int health,  long int loot,  long int levelIndex) = 0;

    virtual int DrawMap(Map* m, bool fogOfWar, Position playerPos)=0;
    virtual void DrawCharacter (int x, int y, int colour, char symbol) = 0;

    virtual void ShowInfo() = 0;
    virtual void ShowNotification(const char* text) = 0;
    virtual int Menu(std::vector<std::pair<std::string,std::string>>) = 0;

    virtual void UpdateUI() = 0;
    
    virtual void ClearConsole() = 0;
    virtual void ClearConsoleAndPrint (std::string text) = 0;
    virtual void ConsolePrint (std::string text, int posX, int posY) = 0;
    virtual int ConsolePrintWithWait (std::string text, int posX, int posY) = 0;
    virtual int ConsoleGetInput() = 0;
    virtual std::string ConsoleGetString() = 0;
    virtual void ClearConsoleHighlighting() = 0;
    virtual void HighlightConsole(int scr_x, int scr_y) = 0;
        
    virtual int InitScreen () = 0;

    virtual void UpdateVisibleTiles(Map* map, Position playerPos)=0;
    virtual void EnableFogOfWar(Map* map, Position playerPosition) = 0;
    virtual void DisableFogOfWar(Map* map) = 0;

    virtual ~UI() {}
};

#endif	/* UI_H */


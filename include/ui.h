#ifndef UI_H
#define	UI_H

#include <curses.h>  
#include "map.h"

class UI {
public:
    virtual void InitWindows() = 0;
    virtual void DestroyWindows() = 0;
    virtual void DecorateWindows() = 0;

    virtual void DrawItems(Map* m) = 0;
    virtual void DrawAreas(Map* m) = 0;

    virtual void DrawPlayerStats (std::string name, int health, int loot) = 0;
    virtual void DrawPlayerEquipmentSlot(int slot, std::string name) = 0;

    virtual void DrawInv(Container* c) = 0;
    virtual void DrawInv(Area* a) = 0;
    virtual void ClearInvHighlighting() = 0;
    virtual void HighlightInv(int loc_x, int loc_y) = 0;
    virtual void DrawInvWindow(int loc_x, int loc_y, const char* tileChar, int colour) = 0;

    virtual int DrawMap (Map* m, bool fogOfWar, int playerX, int playerY, int viewDistance) = 0;
    virtual void DrawCharacter (int x, int y, int colour, char symbol) = 0;


    virtual void ShowInfo();
    
    virtual void ShowNotification(const char* text);
    
    virtual int Menu(const char** text, int buttons);

    virtual void UpdateUI();
    
    virtual void ClearConsole();
    virtual void ConsolePrint (std::string text, int posX, int posY);
    virtual void ConsolePrintWithWait (std::string text, int posX, int posY);
    virtual int ConsoleGetInput();
    virtual std::string ConsoleGetString();
    virtual void ClearConsoleHighlighting();
    virtual void HighlightConsole(int scr_x, int scr_y);
        
    virtual int InitScreen ();

    virtual ~UI() {}
};

#endif	/* UI_H */


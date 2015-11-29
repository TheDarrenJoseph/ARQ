#include <iostream>
#include <curses.h>  

#include "cursesUI.h"


using std::string;

void CursesUI::InitWindows()
{
    titlewin = newwin(1, 37, 0, 0); //Creates the stats window for content

    mainwin_rear = newwin(MAINWIN_REAR_Y, MAINWIN_REAR_X, 1, 1); //Creates a new window called new win at 1,1 that has the dimensions of GRID_X, and GRID_Y.
    mainwin_front = newwin(GRID_Y, GRID_X, 2, 2); //Creates a new window called new win at 2,2 that has the dimensions of GRID_X, and GRID_Y.

    consolewin_rear = newwin(CONSOLEWIN_REAR_Y  , CONSOLEWIN_FRONT_X, GRID_Y+3, 1); //Creates the console window for deco
    consolewin_front = newwin(CONSOLEWIN_FRONT_Y, CONSOLEWIN_FRONT_X, GRID_Y+4, 2); //Creates the console window for content

    statwin_rear = newwin(STATWIN_REAR_Y,  STATWIN_REAR_X , 1, GRID_X+4); //Creates the stat window for deco
    statwin_front = newwin(STATWIN_FRONT_Y,STATWIN_FRONT_X , 2, GRID_X+5); //Creates the stats window for content

    invwins_rear = newwin(INVWINS_REAR_Y, INVWINS_REAR_X, 11, GRID_X+4);

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            //Creates a new window INV_ITEM_Y chars wide, 1 tall, at spacings of 12 chars across
            invwins_front[x][y] = newwin(1, INV_ITEM_Y, 12+(y * 2), GRID_X+5+(x * 11)); //first half as content window
        }
    }

    equipwin_rear = newwin(EQUIPWIN_REAR_Y, EQUIPWIN_REAR_X, 6, GRID_X+4);
    equipwin_outfit = newwin(1, INV_ITEM_Y*3, 9, GRID_X+5);

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 2; y++) {
            equipwin_front[x] = newwin(1, INV_ITEM_Y, 7, GRID_X+5 + (x * 11)); //first half as content window
        }
    }

    std::cout << "Display Initialised\n";
}

void CursesUI::DestroyWindows()
{
    werase(titlewin);

    werase(mainwin_rear);
    werase(mainwin_front);

    werase(consolewin_rear);
    werase(consolewin_front);

    werase(statwin_rear);
    werase(statwin_front);

    werase(invwins_rear);

    for (int x = 0; x < INV_X; x++) {
        for (int y = 0; y < INV_Y; y++) {
            werase(invwins_front[y][x]);
        }
    }

    werase(equipwin_rear);

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            WINDOW * thisWin = &equipwin_front[y][x];
            werase(thisWin);
        }
    }

    werase(equipwin_outfit);
}

void CursesUI::DecorateWindows()
{
    wprint_at(titlewin, (const char *) "||ARQ -- ASCII Roguelike Quester||", 0, 3);

    //draw borders around the main windows
    box(mainwin_rear, 0, 0);
    box(consolewin_rear, 0, 0); //Puts borders around the finalised screen image
    box(statwin_rear, 0, 0);
    box(invwins_rear, 0, 0);
    box(equipwin_rear, 0, 0);

    //add titles to those that need them
    wprint_at(statwin_rear, "Player Stats", 0, 10);
    wprint_at(equipwin_rear, "Equipment", 0, 11);
    wprint_at(invwins_rear, "Inventory", 0, 11);

    wrefresh(mainwin_rear);
    wrefresh(consolewin_rear);
    wrefresh(statwin_rear);
    wrefresh(invwins_rear);
    wrefresh(equipwin_rear);
}

void CursesUI::ShowInfo()
{
    wprint_at(consolewin_front, (const char *) "Created by Rave Kutsuu", 0, 0); //Please leave this untouched as proof of origin
    wprint_at(consolewin_front, (const char *) "ARQ Learner project/Tech demo", 2, 0);
    wprint_at(consolewin_front, (const char *) "Made using C++ and ncurses", 3, 0);
    wgetch(consolewin_front);
}

void CursesUI::ShowNotification(const char* text)
{
    WINDOW* nWinRear = newwin(UI_Y + 2, UI_X + 2, 1, 1);
    WINDOW* nWin = newwin(UI_Y, UI_X, 2, 2);

    box(nWinRear, 0, 0);
    wrefresh(nWinRear);

    wprint_at(nWin, text, 5, 5);
    wprint_at(nWin, "[Enter]", UI_Y - 1, UI_X - 12);


    wrefresh(nWin);

    wgetch(nWin);

}

/** Displays a simple vertical menu of choices and returns the number of the chosen element */
int CursesUI::Menu(const char** text, int buttons)
{

    WINDOW* nWinRear = newwin(UI_Y + 2, UI_X + 2, 1, 1);
    WINDOW* nWin = newwin(UI_Y, UI_X, 2, 2);

    keypad(nWin, TRUE);
    noecho(); //stops input printing on screen

    box(nWinRear, 0, 0);
    wrefresh(nWinRear);

    werase(nWin);

    int scr_x = UI_X / 2 - 8; //ui size/2 - 8 (8 being maximum letters displayed)

    int index = 0;

    for (int i = 0; i < buttons; i++) {
        wprint_at(nWin, text[i], i, scr_x);
    }

    while (true) {
        wchgat(nWin, 12, A_NORMAL, 0, NULL);

        mvwchgat(nWin, index, scr_x, 9, A_NORMAL, 1, NULL); //add red blink to the current item
        wrefresh(nWin);

        int choice = wgetch(nWin);

        if ((choice == KEY_UP) && (index > 0)) {
            index--;
        } else if ((choice == KEY_DOWN) && (index < buttons - 1)) {
            index++;
        } else if (choice == KEY_ENTER) {
            return index;
        }

    }

    return 0;

}

int CursesUI::wprintw_col(WINDOW* winchoice, const char *text, int color_choice) //allows ncurses window and colour selection when outputting characters
{
    wattron(winchoice, COLOR_PAIR(color_choice)); //turns on the chosen colour pair
    wprintw(winchoice, text);
    wattroff(winchoice, COLOR_PAIR(color_choice)); //turns off the chosen colour pair
    return (0);
};

int CursesUI::wprint_at(WINDOW* winchoice, const char *text, int pos_y, int pos_x)
{
    wmove(winchoice, pos_y, pos_x);
    wprintw(winchoice, text);
    wrefresh(winchoice);
    return (0);
}

int CursesUI::wprintNoRefresh(WINDOW* win, std::string text)
{
    wmove(win, 0, 0);
    werase(win);

    wprintw(win, text.c_str());
    return 0;
}

void CursesUI::DrawItems(Map* m)
{
    //iterate through the rows of the grid/map
    for (int y = 0; y < GRID_Y; y++) {
        for (int x = 0; x < GRID_X; x++) {
            //create variables to hold item info
            int colour;
            const char *symbol;

            //grab the info for each item from the library
            Item* i = m->GetItem(x, y);

            if ((i != NULL) && (IsLootable(i))) //Check for an item type, && it not being an empty item 
            {
                colour = i->colour;
                symbol = i->symbol;

                //draw the tile to the screen
                wmove(mainwin_front, y, x);

                wprintw_col(mainwin_front, symbol, colour);
                //	      wrefresh(mainwin_front);
            }
        };
    };

    return;
}

void CursesUI::DrawAreas(Map* m)
{
    //iterate through the rows of the grid/map
    for (int y = 0; y < GRID_Y; y++) {
        for (int x = 0; x < GRID_X; x++) {
            //create variables to hold item info
            int colour;
            const char *symbol;

            //grab the info for each item from the library
            Area* a = m->GetArea(x, y);

            if (a->id == 98) //Check for an item type, && it not being an empty item 
            {
                colour = a->colour;
                symbol = a->symbol;

                //draw the tile to the screen
                wmove(mainwin_front, y, x);

                wprintw_col(mainwin_front, symbol, colour);
            }
        };
    };
   
    return;
}

int CursesUI::DrawMap(Map* m, bool fogOfWar, int playerX, int playerY, int viewDistance)
{
    wmove(mainwin_front, 0, 0);

    int startX = 0;
    int startY = 0;
    int endX = GRID_X;
    int endY = GRID_Y;

    //If fogOfWar is enabled, modify draw distances to fit
    if (fogOfWar) {

        startX = playerX;
        startY = playerY;

        if (startX < GRID_X - viewDistance && startY < GRID_Y - viewDistance) {
            endX = startX + viewDistance;
            endY = startY + viewDistance;
        }

        if (startX > viewDistance && startY > viewDistance) {
            startX -= viewDistance;
            startY -= viewDistance;
        } else {
            startX = 0;
            startY = 0;

        }
    }

    for (int y = startY; y < endY; y++) {
        wmove(mainwin_front, y, 0);

        for (int x = startX; x < endX; x++) {
            int maptile;
            int maptile_colour;
            const char *maptile_char;

            maptile = m->GetTile(x, y);

            maptile_colour = tile_library[maptile].color;
            maptile_char = tile_library[maptile].symbol;

            wmove(mainwin_front, y, x);
            wprintw_col(mainwin_front, maptile_char, maptile_colour);
        };

    };


    return (0);
};

void CursesUI::DrawCharacter(int x, int y, int colour, char symbol)
{
    if ((x < 0) || (x > GRID_X)) {
        return;
    } else if ((y < 0) || (y > GRID_Y)) {
        return;
    } else {
        wmove(mainwin_front, y, x);

        wattron(mainwin_front, COLOR_PAIR(colour)); //turns on the chosen colour pair
        wprintw(mainwin_front, "%c", symbol);
        wattroff(mainwin_front, COLOR_PAIR(colour));
    };

    return;
};

void CursesUI::DrawPlayerStats(std::string name, int health, int loot)
{
    WINDOW* winchoice = statwin_front;

    werase(winchoice);
    wmove(winchoice, 0, 1);
    wattron(winchoice, A_UNDERLINE); //turns on the chosen colour pair
    wprintw(winchoice, "%s", name);
    wattroff(winchoice, A_UNDERLINE); //turns off the chosen colour pair

    wmove(winchoice, 1, 0);
    wprintw_col(winchoice, "Health: ", 4);
    wprintw(winchoice, "%d", health);

    wmove(winchoice, 2, 0);
    wprintw_col(winchoice, "Loot:   ", 4);
    wprintw(winchoice, "%d", loot);

    return;
}

//make sure any overloads of this func are up-to-date with the header
void CursesUI::DrawInv(Container* c)
{
    for (int y = 0; y < 3; y++) {
        //draw each item on this row to the screen
        for (int x = 0; x < 3; x++) {
            WINDOW* thisWindow = invwins_front[y][x];

            werase(thisWindow);
            wmove(thisWindow, 0, 0);

            Item* itm = c->GetItem(x, y);

            //int colour = itm->colour;
            std::string str = itm->name;

            const char* symbol = str.c_str();

            wprintw_col(thisWindow, symbol, 0);
            //wrefresh(thisWindow);
        };
    };
    return;
}

void CursesUI::DrawPlayerEquipmentSlot(int slot, std::string name)
{
    WINDOW* thisWindow;
    
    //Only allow 0-2 indexing
    if (slot>=0 && slot<3) {
       thisWindow = equipwin_front[slot];
       wprintw_col(thisWindow, name.c_str(),0);
    }
    
}

//make sure any overloads of this func are up-to-date with the header
void CursesUI::DrawInv(Area* a)
{
    for (int y = 0; y < 3; y++) {
        //draw each item on this row to the screen
        for (int x = 0; x < 3; x++) {
            WINDOW* thisWindow = invwins_front[y][x];

            werase(thisWindow);
            wmove(thisWindow, 0, 0);

            Item* itm = a->GetItem(x, y); //works

            int colour = itm->colour;

            const char* name = itm->name.c_str();

            wprintw_col(thisWindow, name, colour);
            wrefresh(thisWindow);
        };
    };
    return;
}

void CursesUI :: ClearInvHighlighting() {
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            WINDOW* thisWin = invwins_front[y][x];
            
            mvwchgat(thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
            wrefresh(thisWin);
        }
    }
}

void CursesUI :: ClearConsoleHighlighting() {
    
//    for (int x = 0; x < 3; x++) {
//        for (int y = 0; y < 3; y++) {
//  
//        }
//    }
}

void CursesUI :: HighlightInv(int loc_x, int loc_y) {
    WINDOW* selWin = invwins_front[loc_y][loc_x]; //create a holder for the currently selected window
    mvwchgat(selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red (All 9 characters in length)
    wrefresh(selWin);
}

void CursesUI :: DrawInvWindow(int loc_x, int loc_y, const char* tileChar, int colour) {
    WINDOW* thisWin = invwins_front[loc_x][loc_y];
    
    wmove(thisWin, 0, 0);
    wprintw_col(thisWin, tileChar, colour);
    wrefresh(thisWin);
}

void CursesUI :: HighlightConsole(int scr_x, int scr_y) {
    mvwchgat(consolewin_front, scr_y, scr_y,    27, A_NORMAL, 0, NULL); //remove fancy effects
    mvwchgat(consolewin_front, scr_y, scr_x, 9, A_BLINK, 1, NULL); //add red blink to the current item
    wrefresh(consolewin_front);
}

int CursesUI::InitScreen()
{
    std::cout << "Starting ncurses";
    initscr();

    if (has_colors() == FALSE) {
        endwin();

        printf("Your terminal does not support color\n");
        getch();

        return (1);
    }

    start_color();

    //---------colour pairs---------//
    init_pair(0, COLOR_BLACK, COLOR_BLACK);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);


    keypad(consolewin_front, TRUE);
    return (0);
};

void CursesUI::UpdateUI()
{
    DecorateWindows();

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            wrefresh(invwins_front[x][y]);
        }

        wrefresh(equipwin_front[x]);
    }

    wrefresh(titlewin);

    wrefresh(mainwin_rear);
    wrefresh(consolewin_rear);
    wrefresh(statwin_rear);

    wrefresh(mainwin_front);
    wrefresh(consolewin_front);
    wrefresh(statwin_front);

    wrefresh(equipwin_outfit);
}

Item* CursesUI::AccessPlayerInventory(Player* p)
{
    Area* a = p->GetInventory();

    std::string slc_string;
    char slc_char[20];

    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    while (selection == true) {
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                WINDOW* thisWin = invwins_front[y][x];

                mvwchgat(thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
                wrefresh(thisWin);
            }
        }

        DrawInv(a);
        UpdateUI();

        WINDOW* selWin = invwins_front[loc_y][loc_x]; //create a holder for the currently selected window

        mvwchgat(selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
        wrefresh(selWin);

        werase(consolewin_front);
        wprint_at(consolewin_front, "Select an item with WASD.. 'put' to transfer to the container.", 0, 0);

        wprint_at(consolewin_front, "ARQ:~$ ", 2, 0);

        wgetstr(consolewin_front, slc_char);
        slc_string = slc_char;

        if ((slc_string == "W") || (slc_string == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((slc_string == "A") || (slc_string == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((slc_string == "S") || (slc_string == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((slc_string == "D") || (slc_string == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if ((slc_string == "Exit") || (slc_string == "exit") || (slc_string == "EXIT")) {
            selection = false;
            return (NULL);
        } else if (slc_string == "put") {
            Item* thisItem = a->GetItem(loc_x, loc_y);
            a->RemoveItem(loc_x, loc_y);

            return thisItem;
        } else {
            werase(consolewin_front);
            wprint_at(consolewin_front, "Not a correct selection, try again.", 0, 0);
            wgetch(consolewin_front);
        };

        std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (NULL);
};

int CursesUI::PlayerItemProc(Player* p, Item* itm, int x, int y)
{
   std::string answer;
    char answerchar[20];
   std::string itm_name = itm->name;

    werase(consolewin_front);
    wmove(consolewin_front, 0, 0);
    wprintw(consolewin_front, "There's a %s on the floor..", itm_name.c_str());
    wrefresh(consolewin_front);

    wgetch(consolewin_front);

    wmove(consolewin_front, 0, 0);
    wprintw(consolewin_front, "Would you like to pick the %s? ", itm_name.c_str());
    wgetstr(consolewin_front, answerchar);

    answer = (answerchar);

    if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y")) {
        if (p->AddToInventory(itm) == (1)) //If addToInventory unsuccessful
        {
            werase(consolewin_front);
            wprint_at(consolewin_front, (const char *) "Inventory full..", 0, 0);
            wgetch(consolewin_front);
            return (1);
        } else {
            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);
            wprintw(consolewin_front, "You pick up the %s..", itm_name.c_str());
            wgetch(consolewin_front);

            p->SetInventoryTile(x, y, new Item(item_library[no_item]));

            p->SetPos(x, y);

            return (0);
        };
    } else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N")) {
        wmove(consolewin_front, 0, 0);
        werase(consolewin_front);
        wprintw(consolewin_front, "You leave the %s untouched..", itm_name.c_str());
        wgetch(consolewin_front);

        p->SetPos(x, y);
        return (0);
    } else {
        wprint_at(consolewin_front, "Incorrect choice, please answer yes or no.. ", 0, 0);
        wgetch(consolewin_front);

        PlayerItemProc(p, itm, x, y);
    };

    return (0);
};

void CursesUI::ConsolePrint(std::string text, int posX, int posY)
{
    wmove(consolewin_front, posY, posX);
    wprintw(consolewin_front, text.c_str());

    wgetch(consolewin_front);
}

void CursesUI::ClearConsole()
{
    werase(consolewin_front);
}

void CursesUI::ConsolePrintWithWait(std::string text, int posX, int posY)
{
    wmove(consolewin_front, posY, posX);
    wprintw(consolewin_front, text.c_str());

    wgetch(consolewin_front);
}

int CursesUI::ConsoleGetInput() {
    keypad(consolewin_front, TRUE);
    noecho(); //stops input printing on screen
    int choice = wgetch(consolewin_front);
    echo();
    return choice;
}

std::string CursesUI::ConsoleGetString()
{
    char answerchar[20];

    wgetstr(consolewin_front, answerchar);

   std::string output;
    output += answerchar;
    return output;
}







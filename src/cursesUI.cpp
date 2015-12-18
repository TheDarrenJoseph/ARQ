#include <iostream>
#include <curses.h>  

#include "cursesUI.h"


using std::string;

void CursesUI::InitWindows()
{
    //newwin(size y, size x, pos y, pos x) 
    int maxX, maxY;
    getmaxyx(stdscr, maxY, maxX);
    
    titlewin = newwin(1, maxX, 0, maxX/3); //Creates the stats window for content

    mainwin_rear = newwin(MAINWIN_REAR_Y, MAINWIN_REAR_X, 1, 1); 
    mainwin_front = newwin(MAINWIN_FRONT_Y, MAINWIN_FRONT_X, 2, 2); //Creates a new window called new win at 2,2 that has the dimensions of GRID_X, and GRID_Y.

    consolewin_rear = newwin(CONSOLEWIN_REAR_Y  , CONSOLEWIN_REAR_X, maxY-6, 2); //Creates the console window for deco
    consolewin_front = newwin(CONSOLEWIN_FRONT_Y, CONSOLEWIN_FRONT_X, maxY-5, 3); //Creates the console window for content

    invwin_rear = newwin(INVWIN_REAR_Y, INVWIN_REAR_X, 2, 2);
    invwin_front = newwin(INVWIN_FRONT_Y, INVWIN_FRONT_X,4,4);

    std::cout << "Display Initialised\n";
}

void CursesUI::DestroyWindows()
{
    werase(titlewin);

    werase(mainwin_rear);
    werase(mainwin_front);

    werase(consolewin_rear);
    werase(consolewin_front);

}

void CursesUI::DecorateWindows()
{
    wprint_at(titlewin, (const char *) "||ARQ -- ASCII Roguelike Quester||", 0, 3);

    //draw borders around the main windows
    box(mainwin_rear, 0, 0);
    box(consolewin_rear, 0, 0); //Puts borders around the finalised screen image

    //add titles to those that need them

    wrefresh(mainwin_rear);
    wrefresh(consolewin_rear);
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
    WINDOW* nWinRear = newwin(MAINWIN_REAR_Y, MAINWIN_REAR_X, 1, 1);
    WINDOW* nWin = newwin(MAINWIN_FRONT_Y, MAINWIN_FRONT_X, 2, 2);

    box(nWinRear, 0, 0);
    wrefresh(nWinRear);

    wprint_at(nWin, text, 5, 5);
    //wprint_at(nWin, "[Enter]", UI_Y-1, UI_X-12);


    wrefresh(nWin);

    wgetch(nWin);
}

/** Displays a simple vertical menu of choices and returns the number of the chosen element */
int CursesUI::Menu(const char** text, int buttons)
{
    WINDOW* nWinRear = newwin(MAINWIN_REAR_Y, MAINWIN_REAR_X, 1, 1);
    WINDOW* nWin = newwin(MAINWIN_FRONT_Y, MAINWIN_FRONT_X, 2, 2);

    keypad(nWin, TRUE);
    noecho(); //stops input printing on screen

    box(nWinRear, 0, 0);
    wrefresh(nWinRear);

    werase(nWin);

    int scr_x = 12; //ui size/2 - 8 (8 being maximum letters displayed)

    int index = 0;

    for (int i = 0; i < buttons; i++) {
        wprint_at(nWin,text[i],i,scr_x);
    }

    while (true) {
        wchgat(nWin, 12, A_NORMAL, 0, NULL);

        mvwchgat(nWin, index, scr_x, 12, A_NORMAL, 1, NULL); //add red blink to the current item
        wrefresh(nWin);

        int choice = wgetch(nWin);

        if ((choice == KEY_UP) && (index > 0)) {
            index--;
        } else if ((choice == KEY_DOWN) && (index < buttons - 1)) {
            index++;
        } else if (choice == KEY_RIGHT) {
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
}

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
            char symbol;

            //grab the info for each item from the library
            const Item* i = m->GetItem(x, y);

            if ((i != NULL) && (IsLootable(i))) //Check for an item type, && it not being an empty item 
            {
                colour = i->colour;
                symbol = i->symbol;

                //draw the tile to the screen
                wmove(mainwin_front, y, x);

                wprintw_col(mainwin_front, &symbol, colour);
                //	      wrefresh(mainwin_front);
            }
        }
    }

    return;
}

void CursesUI::DrawContainers(Map* m)
{
     //create variables to hold item info
     int colour;
     char symbol;
     Container c;
     
    //iterate through the rows of the grid/map
    for (int y = 0; y < GRID_Y; y++) {
        for (int x = 0; x < GRID_X; x++) {
           
            //grab the info for each item from the library
            c = m->GetContainer(x, y);

            if (c.id == 98) //Check for an item type, && it not being an empty item 
            {
                colour = c.colour;
                symbol = c.symbol;

                //draw the tile to the screen
                wmove(mainwin_front, y, x);
                wprintw_col(mainwin_front, &symbol, colour);
            }
        }
    }

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
}

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
}

void CursesUI::DrawPlayerStats(std::string name, int health, int loot)
{
    WINDOW* winchoice = mainwin_rear;

    //werase(winchoice);
    //box(winchoice,0,12);
    wmove(winchoice, 0, 1);
    wprintw(winchoice, "%-12s", name.c_str());

    wmove(winchoice, 0, 14);
    wprintw_col(winchoice, "Health: ", 2);
    wprintw(winchoice, "%-4d", health);

    wmove(winchoice, 0, 28);
    wprintw_col(winchoice, "Loot: ", 4);
    wprintw(winchoice, "%06d", loot);
    
    wrefresh(winchoice);
    
    return;
}

//make sure any overloads of this func are up-to-date with the header
//void CursesUI::ListInv(Container* c)
//{
//    for (int y = 0; y < 3; y++) {
//        //draw each item on this row to the screen
//        for (int x = 0; x < 3; x++) {
//            WINDOW* thisWindow = invwins_front[y][x];
//
//            werase(thisWindow);
//            wmove(thisWindow, 0, 0);
//
//            Item* itm = c->GetItem(x, y);
//
//            //int colour = itm->colour;
//            std::string str = itm->name;
//
//            const char* symbol = str.c_str();
//
//            wprintw_col(thisWindow, symbol, 0);
//            //wrefresh(thisWindow);
//        };
//    };
 //   return;
//}


void CursesUI :: ClearInvHighlighting() {
    //Clear other highlighting
    for (int y=0; y<INVWIN_FRONT_Y-1; y++) {
        mvwchgat(invwin_front, y, 0, INVWIN_FRONT_X, A_NORMAL, 0, NULL);
    }
}

/** Highlights xChars characters at the specified x,yIndex
 * 
 * @param xChars
 * @param yIndex
 */
void CursesUI :: HighlightInv(int xChars, int xIndex, int yIndex) {
   
    
    //Index/Selection highlight
    mvwchgat(invwin_front, yIndex, xIndex, xChars, A_BLINK, 1, NULL); 
    wrefresh(invwin_front);
}

void CursesUI :: HighlightInvLine(int index) {
    mvwchgat(invwin_front, index, 0,INVWIN_FRONT_X, A_BLINK, 1, NULL); //add red blink to the current line
    wrefresh(invwin_front);
 }

/**
 * 
 * @param c The container to list
 * @param invIndex the top index of the list, so that the list can scroll down
 */
void CursesUI::ListInv(Container* c, long unsigned int invIndex)
{   
    box(invwin_rear, 0, 0);
    wprint_at(invwin_rear,c->name.c_str(),0,1);
    
      //For each line of the window
    wprint_at(invwin_rear,"NAME",1,1);
    wprint_at(invwin_rear,"WEIGHT",1,12);
    wprint_at(invwin_rear,"VALUE",1,24);
    
    wrefresh(invwin_rear);
    
    long unsigned int invSize = c->GetSize();    
    
    for (long unsigned int i=0;  i<(long unsigned int)INVWIN_FRONT_Y-1 && (invIndex+i)<invSize; i++) {
                //thisIndex += i;
                char buffer[20];
            
                const Item* thisItem = c->GetItem(invIndex+i);
                
                sprintf(buffer,"%-12s",thisItem->name.c_str());
                wprint_at(invwin_front,buffer,i,0);
               
                //Weight
                sprintf(buffer,"%-4d",c->weight);
                wprint_at(invwin_front,buffer,i,14);
               
                //Value
                sprintf(buffer,"%-4d",c->value);
                wprint_at(invwin_front,buffer,i,20);
    }
        
    //wrefresh(invwin_front);
    return;
}



void CursesUI :: ClearConsoleHighlighting() {
    //    for (int x = 0; x < 3; x++) {
    //        for (int y = 0; y < 3; y++) {
    //  
    //        }
    //    }
}

void CursesUI :: HighlightConsole(int scr_x, int scr_y) {
    mvwchgat(consolewin_front, scr_y, scr_y,    27, A_NORMAL, 0, NULL); //remove fancy effects
    mvwchgat(consolewin_front, scr_y, scr_x, 9, A_BLINK, 1, NULL); //add red blink to the current item
    wrefresh(consolewin_front);
}

int CursesUI::InitScreen()
{
    std::cout << "Starting ncurses\n";
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
}

void CursesUI::UpdateUI()
{
    DecorateWindows();

    wrefresh(titlewin);

    wrefresh(mainwin_rear);
    wrefresh(consolewin_rear);

    wrefresh(mainwin_front);
    wrefresh(consolewin_front);
}

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
        p->AddToInventory(itm);

            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);
            wprintw(consolewin_front, "You pick up the %s..", itm_name.c_str());
            wgetch(consolewin_front);

           // p->SetInventoryTile(x, y, new Item(item_library[no_item]));

            p->SetPos(x, y);

            return (0);
 
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
    }

    return (0);
}

void CursesUI::ConsolePrint(std::string text, int posX, int posY)
{
    wmove(consolewin_front, posY, posX);
    wprintw(consolewin_front, text.c_str());
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

/** Allows grabbing of a single keyboard key press from the input console, 
 * used for the main UI (movement, shortcuts)
 * 
 * @return the curses keyboard input code
 */
int CursesUI::ConsoleGetInput() {
    keypad(consolewin_front, TRUE);
    return wgetch(consolewin_front); 
}

std::string CursesUI::ConsoleGetString()
{
    echo();
    char answerchar[20];
    wgetstr(consolewin_front, answerchar);
    noecho();
    
    std::string output;
    output += answerchar;
    
    return output;
}







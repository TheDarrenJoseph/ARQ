#include "cursesUI.h"

using std::string;

void CursesUI::ShowInfo() {
    wprint_at(consolewin_front, (const char *) "Created by Darren Joseph", 0, 0);
    wprint_at(consolewin_front, (const char *) "ARQ Learner project/Tech demo", 2, 0);
    wprint_at(consolewin_front, (const char *) "Made using C++ and ncurses", 3, 0);
    wgetch(consolewin_front);
}

void CursesUI::InitWindows() {
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

void CursesUI::DestroyWindows() {
    werase(titlewin);

    werase(mainwin_rear);
    werase(mainwin_front);

    werase(consolewin_rear);
    werase(consolewin_front);

}

void CursesUI::DecorateWindows() {
    wprint_at(titlewin, (const char *) "||ARQ -- ASCII Roguelike Quester||", 0, 3);

    //draw borders around the main windows
    box(mainwin_rear, 0, 0);
    box(consolewin_rear, 0, 0); //Puts borders around the finalised screen image

    //add titles to those that need them

    wrefresh(mainwin_rear);
    wrefresh(consolewin_rear);
}

void CursesUI::ShowNotification(const char* text) {
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
int CursesUI::Menu(std::vector<std::pair<std::string,std::string>> items) {
    WINDOW* nWinRear = newwin(MAINWIN_REAR_Y, MAINWIN_REAR_X, 1, 1);
    WINDOW* nWin = newwin(MAINWIN_FRONT_Y, MAINWIN_FRONT_X, 2, 2);

    keypad(nWin, TRUE);
    noecho(); //stops input printing on screen

    //Border and update the slightly larger rear window
    box(nWinRear, 0, 0); 
    wrefresh(nWinRear);

    werase(nWin);

    int scr_x = 12; //ui size/2 - 8 (8 being maximum letters displayed)
    int y     = 0;
    for (std::pair<std::string,std::string> pair : items) {
        wprint_at(nWin,pair.first.c_str(),y,scr_x);
        wprint_at(nWin,pair.second.c_str(),y++,scr_x+12);
    }

    int index = 0;
    while (true) {
        wchgat(nWin, nWin->_maxx, A_NORMAL, 0, NULL);

        mvwchgat(nWin, index, scr_x, 12, A_NORMAL, 1, NULL); //add red blink to the current item
        wrefresh(nWin);

        int choice = wgetch(nWin);

        if ((choice == KEY_UP) && (index > 0)) {
            index--;
        } else if ((choice == KEY_DOWN) && (index < y-1)) {
            index++;
        } else if (choice == KEY_RIGHT) {
            return index;
        }
        
    }

    return 0;

}

/** Allows ncurses window and colour selection when outputting characters (DOES NOT REFRESH THE WINDOW)
 * 
 * @param winchoice the ncurses window to print to
 * @param text The text to print
 * @param color_choice the colour to print ()
 * @return 
 */
int CursesUI::wprintw_col(WINDOW* winchoice, const char *text, int color_choice) { 

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
    for (int y = 0; y < m -> GetGridY(); y++) {
        for (int x = 0; x < m -> GetGridX(); x++) {
            const Item* item = m->GetItem(x, y);
            if ((item != NULL) && (item -> IsLootable())) {
                int colour = item -> GetColour();
                char symbol = item -> GetSymbol();
                wmove(mainwin_front, y, x);
                wprintw_col(mainwin_front, &symbol, colour);
            }
        }
    }
}

void CursesUI::DrawContainers(Map* m)
{
  for (int y = 0; y < m -> GetGridY(); y++) {
    for (int x = 0; x < m -> GetGridX(); x++) {
      Container container = m -> GetContainer(x, y);
      if (container.HasItems())
      {
        int colour = container.GetColour();
        char symbol = container.GetSymbol();
        wmove(mainwin_front, y, x);
        wprintw_col(mainwin_front, &symbol, colour);
      }
    }
  }
}

void CursesUI::CalculateViewBoundaries(Position playerPos, int viewDistance, Position* viewStart, Position* viewEnd) {
  viewStart -> x = (playerPos.x - viewDistance);
  viewStart -> y = (playerPos.y - viewDistance);
  viewEnd -> x   = (playerPos.x + viewDistance);
  viewEnd -> y   = (playerPos.y + viewDistance);

  if (playerPos.x < viewDistance) viewStart -> x = viewDistance - playerPos.x;
  if (playerPos.y < viewDistance) viewStart -> y = viewDistance - playerPos.y;
  if (playerPos.x > (GRID_X - viewDistance)) viewEnd -> x = GRID_X - playerPos.x;
  if (playerPos.y > (GRID_Y - viewDistance)) viewEnd -> x = GRID_Y - playerPos.y;
}

int CursesUI::DrawMap(Map* m, bool fogOfWar, Position playerPos)
{
    wmove(mainwin_front, 0, 0);
    
    Position mapViewStart = Position(0, 0);
    Position mapViewEnd = Position(GRID_X, GRID_Y);
    
    tile maptile;
    int maptile_colour;
    const char *maptile_char;
    const tile_details* tileDetails;

    // Set any tiles in range to visible
    if (fogOfWar) {
      UpdateVisibleTiles(m, playerPos);
    } 

    //Drawing everything visible
    for (unsigned int y = mapViewStart.y; y < mapViewEnd.y; y++) {
      for (unsigned int x = mapViewStart.x; x < mapViewEnd.x; x++) {
        if (!fogOfWar || m -> TileIsVisible(x,y)) {
          maptile = m->GetTile(x, y);
          tileDetails = &tile_library[maptile];        
          maptile_colour = tileDetails -> color;

          if (maptile == dor) {
            Door* door = m -> GetDoor(x,y);
            const char* open_door_symbol = "-";
            maptile_char = door -> isOpen ? open_door_symbol : tileDetails -> symbol;
          } else {
            maptile_char = tileDetails -> symbol;
          }
                
          wmove(mainwin_front, y, x);
          wprintw_col(mainwin_front, maptile_char, maptile_colour);
        }
      }
	}

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

void CursesUI::DrawPlayerStats(std::string name, int health, long unsigned int loot, long unsigned int currentLevel)
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
    
    wmove(winchoice, 0, 42);
    wprintw_col(winchoice, "Level: ", 4);
    wprintw(winchoice, "%06d", currentLevel);
    
    wrefresh(winchoice);
    
    return;
}

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

void CursesUI :: HighlightInvLine(int yIndex) {
    mvwchgat(invwin_front, yIndex, 0, INVWIN_FRONT_X -1 , A_BLINK, 1, NULL); //add red blink to the current line
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
    wprint_at(invwin_rear,c -> GetName().c_str(),0,1);
    
    // Each column has a +2 margin and the offset of the previous label length (or any other needed padding for contents)
    const int COL_1 = 0;
    const int COL_2 = 20;
    const int COL_3 = 33;
      //For each line of the window
    wprint_at(invwin_rear,"NAME", 1, COL_1+1); // +1 for border
    wprint_at(invwin_rear,"WEIGHT (Kg)", 1, COL_2);
    wprint_at(invwin_rear,"VALUE", 1, COL_3);
    
    wrefresh(invwin_rear);
    long unsigned int invSize = c->GetSize();    
    long unsigned int lowestDisplayIndex = (long unsigned int)INVWIN_FRONT_Y-1;
    for (long unsigned int i=0;  i < lowestDisplayIndex && (invIndex+i) < invSize; i++) {
                const Item* thisItem = c->GetItem(invIndex+i);
                char buffer[20];            
                sprintf(buffer,"%-20s",thisItem->GetName().c_str());
                wprint_at(invwin_front, buffer, i, COL_1);
               
                //Weight
                sprintf(buffer,"%-04d",thisItem->GetWeight());
                wprint_at(invwin_front, buffer, i, COL_2);
               
                //Value
                sprintf(buffer,"%-04d",thisItem->GetValue());
                wprint_at(invwin_front, buffer, i, COL_3);
    }        
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

void CursesUI::ConsolePrint(std::string text, int posX, int posY)
{
    wmove(consolewin_front, posY, posX);
    wprintw(consolewin_front, text.c_str());
    wrefresh(consolewin_front);
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


void CursesUI::UpdateVisibleTiles(Map* map, Position playerPos) {
  Position viewStart = Position(0, 0);
  Position viewEnd = Position(GRID_X, GRID_Y);

  Position localViewStart = Position(0, 0);
  Position localViewEnd = Position(GRID_X, GRID_Y);
  CalculateViewBoundaries(playerPos, 6, &localViewStart, &localViewEnd);

  for (unsigned int y = viewStart.y; y < viewEnd.y; y++) {
    for (unsigned int x = viewStart.x; x < viewEnd.x; x++) {
      if ( y >= localViewStart.y && y <= localViewEnd.y && x >= localViewStart.x && x <= localViewEnd.x) {
        map -> SetTileVisible(x,y, true);
      }
    }
  }
}

void CursesUI::EnableFogOfWar(Map* map, Position playerPos) {
  Position viewStart = Position(0, 0);
  Position viewEnd = Position(GRID_X, GRID_Y);

  Position localViewStart = Position(0, 0);
  Position localViewEnd = Position(GRID_X, GRID_Y);
  CalculateViewBoundaries(playerPos, 6, &localViewStart, &localViewEnd);

  for (unsigned int y = viewStart.y; y < viewEnd.y; y++) {
    for (unsigned int x = viewStart.x; x < viewEnd.x; x++) {
      if ( y >= localViewStart.y && y <= localViewEnd.y && x >= localViewStart.x && x <= localViewEnd.x) {
        map -> SetTileVisible(x,y, true);
      } else {
        map -> SetTileVisible(x,y, false);
      }
    }
  }

}

void CursesUI::DisableFogOfWar(Map* map) {
  Position viewStart = Position(0, 0);
  Position viewEnd = Position(GRID_X, GRID_Y);

  for (unsigned int y = viewStart.y; y < viewEnd.y; y++) {
    for (unsigned int x = viewStart.x; x < viewEnd.x; x++) {
      map -> SetTileVisible(x,y, true);
    }
  }
}





#include <iostream>
#include <curses.h>

#include "game.h"
#include "grid.h"
#include "ui.h"

WINDOW*titlewin; //Creates the stats window for content
 
WINDOW*mainwin_rear; //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
WINDOW*mainwin_front; //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.
 
WINDOW*consolewin_rear; //Creates the console window for deco
WINDOW*consolewin_front; //Creates the console window for content
 
WINDOW*statwin_rear; //Creates the stat window for deco
WINDOW*statwin_front; //Creates the stats window for content

WINDOW* invwins_front[3][3];
WINDOW* invwins_rear; 
  
WINDOW* equipwin_rear; 
WINDOW* equipwin_outfit; 
WINDOW* equipwin_front[3];

void InitWindows()
{
  titlewin=newwin(1,37,0,0); //Creates the stats window for content
 
  mainwin_rear=newwin(grid_y+2,grid_x+2,1,1); //Creates a new window called new win at 0,1 that has the dimensions of grid_x, and grid_y.
  mainwin_front=newwin(grid_y,grid_x,2,2); //Creates a new window called new win at 1,2 that has the dimensions of grid_x, and grid_y.
 
  consolewin_rear=newwin(6,grid_x*2+7,grid_y+3,1); //Creates the console window for deco
  consolewin_front=newwin(4,grid_x*2+5,grid_y+4,2); //Creates the console window for content
 
  statwin_rear=newwin(5,grid_x+4,1,grid_x+4); //Creates the stat window for deco
  statwin_front=newwin(3,grid_x-2,2,grid_x+5); //Creates the stats window for content

  invwins_rear = newwin(7,grid_x+4,11,grid_x+4); 
 
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  invwins_front[x][y] = newwin(1,10, 12+(y*2), grid_x+5+(x*11)); //first half as content window
	}
    }
  
  equipwin_rear = newwin(5,grid_x+4,6,grid_x+4); 
  equipwin_outfit = newwin(1,30,9,grid_x+5); 
  
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<2; y++)
	{
	  equipwin_front[x] = newwin(1,10, 7, grid_x+5+(x*11)); //first half as content window
	}
    }
  
  std::cout << "Display Initialised\n";
}
 
void DestroyWindows()
{
  werase (titlewin);
  
  werase (mainwin_rear);
  werase (mainwin_front);
  
  werase (consolewin_rear);
  werase (consolewin_front);
  
  werase (statwin_rear);
  werase (statwin_front);
  
  werase (invwins_rear);
  
  for (int x=0; x<inv_x; x++)
    {
      for (int y=0; y<inv_y; y++)
	{
	  werase (invwins_front[y][x]);
	}
    }
  
  werase (equipwin_rear);
  
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  WINDOW * thisWin = &equipwin_front[y][x];
	  werase (thisWin);
	}
    }
   
  werase (equipwin_outfit);
}
 
void DecorateWindows()
{
  wprint_at (titlewin,(const char *)"||ARQ -- ASCII Roguelike Quester||",0,3);
   
  //draw borders around the main windows
  box (mainwin_rear,0,0);
  box (consolewin_rear,0,0); //Puts borders around the finalised screen image
  box (statwin_rear,0,0); 
  box (invwins_rear,0,0);
  box (equipwin_rear,0,0);
   
  //add titles to those that need them
  wprint_at(statwin_rear,"Player Stats",0,10);  
  wprint_at(equipwin_rear,"Equipment",0,11);  
  wprint_at(invwins_rear,"Inventory",0,11);  
  
  wrefresh (mainwin_rear);
  wrefresh (consolewin_rear);
  wrefresh (statwin_rear);
  wrefresh (invwins_rear);
  wrefresh (equipwin_rear);
}
 
void Info()
{
  wprint_at (consolewin_front,(const char *)"Created by Rave Kutsuu",0,0); //Please leave this untouched as proof of origin
  wprint_at (consolewin_front,(const char *)GetVersion().c_str(),1,0); 
  wprint_at (consolewin_front,(const char *)"ARQ Learner project/Tech demo",2,0);
  wprint_at (consolewin_front,(const char *)"Made using C++ and ncurses",3,0);
  wgetch (consolewin_front);
}

int wprintw_col (WINDOW* winchoice, const char *text, int color_choice) //allows ncurses window and colour selection when outputting characters
{
  wattron(winchoice,COLOR_PAIR(color_choice)); //turns on the chosen colour pair
  wprintw (winchoice, text); 
  wattroff(winchoice,COLOR_PAIR(color_choice)); //turns off the chosen colour pair
  return (0);
};             

int wprint_at (WINDOW* winchoice, const char *text, int pos_y, int pos_x)
{
  wmove (winchoice,pos_y,pos_x);
  wprintw (winchoice, text); 
  wrefresh (winchoice);
  return (0);
}

int wprintNoRefresh (WINDOW* win, std::string text)
{
  wmove (win,0,0);
  werase(win);
  
  wprintw (win, text.c_str());
  return 0;
}
 
void DrawItems()
{
  //iterate through the rows of the grid/map
  for (int y = 0; y < grid_y; y++)
    {	  
      for(int x = 0; x < grid_x; x++)
	{
	  //create variables to hold item info
	  int colour;
	  const char *symbol;
        
	  //grab the info for each item from the library
	  item* i = GetItem(x,y);
      
	  if ((i != NULL) && (IsLootable(i)) ) //Check for an item type, && it not being an empty item 
	    {
	      colour = i->colour;
	      symbol = i->symbol;
        
	      //draw the tile to the screen
	      wmove (mainwin_front,y,x);
      
	      wprintw_col (mainwin_front, symbol, colour);
	      //	      wrefresh(mainwin_front);
	    }
	};  
    }; 
   
  return;
}

void DrawAreas()
{
  //iterate through the rows of the grid/map
  for (int y = 0; y < grid_y; y++)
    {	  
      for(int x = 0; x < grid_x; x++)
	{
	  //create variables to hold item info
	  int colour;
	  const char *symbol;
        
	  //grab the info for each item from the library
	  area* a = GetArea(x,y);
      
	  if (a->id == 98) //Check for an item type, && it not being an empty item 
	    {
	      colour = a->colour;
	      symbol = a->symbol;
        
	      //draw the tile to the screen
	      wmove (mainwin_front,y,x);
      
	      wprintw_col (mainwin_front, symbol, colour);
	    }
	};  
    }; 
   
  return;
}
 
int DrawMap ()
{
  wmove (mainwin_front,0,0);
 	
  for (int y = 0; y < grid_y; y++)
    {
      wmove (mainwin_front,y,0);
  
      for(int x = 0; x < grid_x; x++)
	{
	  int maptile;
	  int maptile_colour;
	  const char *maptile_char;
      
	  maptile = GetTile(x,y);
     
	  maptile_colour = tile_library[maptile].color;
	  maptile_char = tile_library[maptile].symbol;
     
	  wmove (mainwin_front,y,x);
	  wprintw_col (mainwin_front, maptile_char, maptile_colour);
	};
	 
    };
   
  return (0);
};

void DrawCharacter(int x, int y, int colour, char symbol)
{ 
  if ((x < 0) || (x > grid_x))
    {
      return;
    }
  
  else if ((y < 0) || (y > grid_y))
    {
      return;
    }
   
  else
    {
      wmove (mainwin_front,y,x);
  
      wattron(mainwin_front,COLOR_PAIR(colour)); //turns on the chosen colour pair
      wprintw (mainwin_front, "%c", symbol);
      wattroff(mainwin_front,COLOR_PAIR(colour));
    };
   
  return;
};

void DrawPlayerInv(Player* p)
{
  for (int y = 0; y < 3; y++)
    {
      //draw each item on this row to the screen
      for(int x = 0; x < 3; x++)
	{
	  WINDOW* thisWindow = invwins_front[y][x];
         
	  werase(thisWindow);
	  wmove(thisWindow,0,0); 
		 
	  item* itm = p->GetFromInventory(x,y); 
		   
	  int colour = itm->colour;
	  std::string str = itm->name;
         
	  const char* symbol = str.c_str();

	  wprintw_col (thisWindow, symbol, colour);         
	}; 
    };  
  return;
};

void DrawPlayerEquip (Player* p)
{
  //draw weapons
  for(int x = 0; x < inv_x; x++)
    {
      WINDOW* thisWindow = equipwin_front[x];
    
      werase(thisWindow);
      wmove(thisWindow,0,0); 
	
      weapon* weps = p->GetWeps();
		   
      int colour = weps[x].colour;
      std::string str = weps[x].name;
         
      const char* symbol = str.c_str();
      wprintw_col (thisWindow, symbol, colour);       
    }; 
   
    
  werase(equipwin_outfit);
  wmove(equipwin_outfit,0,0); 
  
  outfit currentOutfit = p->GetOutfit();
		   
  int colour = currentOutfit.colour;
  std::string str = currentOutfit.name;
         
  const char* nameChars = str.c_str();

  wprintw_col (equipwin_outfit, nameChars, colour);         
  
  return;
};
 
void DrawPlayerStats (Player* p)
{ 
  WINDOW* winchoice = statwin_front;
	
  werase (winchoice);
  wmove (winchoice,0,1);
  wattron(winchoice,A_UNDERLINE); //turns on the chosen colour pair
  wprintw (winchoice,"%s",p->GetName().c_str());
  wattroff(winchoice,A_UNDERLINE); //turns off the chosen colour pair
 
  wmove (winchoice,1,0);
  wprintw_col (winchoice,"Health: ",4); 
  wprintw (winchoice,"%d",p->GetHealth());
 
  wmove (winchoice,2,0);
  wprintw_col (winchoice,"Loot:   ",4); 
  wprintw (winchoice,"%d",p->GetLootScore());
 
  for( int a = 0; a < max_npcs; a++ )
    {
      wmove (winchoice,a+3,0);
      //LIST HEALTH OF NPCS?
    };
     
  return;
}

//make sure any overloads of this func are up-to-date with the header
void DrawInv(container* c)
{
  for (int y = 0; y<3; y++)
    {
      //draw each item on this row to the screen
      for(int x = 0; x<3; x++)
	{	 
	  WINDOW* thisWindow = invwins_front[y][x];
         
	  werase(thisWindow);
	  wmove(thisWindow,0,0); 
      	 
	  item* itm = c->GetItem(x,y);      
       
	  //int colour = itm->colour;
	  std::string str = itm->name;
         
	  const char* symbol = str.c_str();

	  wprintw_col (thisWindow, symbol, 0);
	  //wrefresh(thisWindow);
	}; 
    };  
  return;
}

//make sure any overloads of this func are up-to-date with the header
void DrawInv(area* a)
{
  for (int y = 0; y<3; y++)
    {
      //draw each item on this row to the screen
      for(int x = 0; x<3; x++)
	{	 
	  WINDOW* thisWindow = invwins_front[y][x];
         
	  werase(thisWindow);
	  wmove(thisWindow,0,0); 
      	 
	  item* itm = a->GetItem(x,y); //works
       
	  int colour = itm->colour;

	  const char* name = itm->name.c_str();

	  wprintw_col (thisWindow, name, colour);
          wrefresh(thisWindow);
	}; 
    };  
  return;
}

int init_screen ()
{
  std::cout << "Starting ncurses";
  initscr();

  if (has_colors() == FALSE)
    {  
      endwin();
	  
      printf("Your terminal does not support color\n");
      getch();
	  
      return(1);
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


void UpdateUI()
{
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  wrefresh (invwins_front[x][y]);
	}
       
      wrefresh (equipwin_front[x]);  
    } 
	 
  wrefresh (titlewin);
    
  wrefresh (mainwin_front);
  wrefresh (consolewin_front);
  wrefresh (statwin_front);
  wrefresh (equipwin_outfit);
}
 
void GetPlayerInput(Player* p)
{
  p->Input (consolewin_front, invwins_front, mainwin_front); //Allows the player to move/take their turn
}


#include <iostream>
#include <curses.h>  

#include "ui.h"


using std::string;

void CursesUI :: InitWindows()
{
  titlewin=newwin(1,37,0,0); //Creates the stats window for content
 
  mainwin_rear=newwin(GRID_Y+2,GRID_X+2,1,1); //Creates a new window called new win at 1,1 that has the dimensions of GRID_X, and GRID_Y.
  mainwin_front=newwin(GRID_Y,GRID_X,2,2); //Creates a new window called new win at 2,2 that has the dimensions of GRID_X, and GRID_Y.
 
  consolewin_rear=newwin(6,GRID_X*2+7,GRID_Y+3,1); //Creates the console window for deco
  consolewin_front=newwin(4,GRID_X*2+5,GRID_Y+4,2); //Creates the console window for content
 
  statwin_rear=newwin(5,GRID_X+4,1,GRID_X+4); //Creates the stat window for deco
  statwin_front=newwin(3,GRID_X-2,2,GRID_X+5); //Creates the stats window for content

 
  invwins_rear = newwin(7,GRID_X+4,11,GRID_X+4); 

  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  invwins_front[x][y] = newwin(1,10, 12+(y*2), GRID_X+5+(x*11)); //first half as content window
	}
    }
  
  equipwin_rear = newwin(5,GRID_X+4,6,GRID_X+4); 
  equipwin_outfit = newwin(1,30,9,GRID_X+5); 
  
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<2; y++)
	{
	  equipwin_front[x] = newwin(1,10, 7, GRID_X+5+(x*11)); //first half as content window
	}
    }
  
  std::cout << "Display Initialised\n";
}
 
void CursesUI :: DestroyWindows()
{
  werase (titlewin);
  
  werase (mainwin_rear);
  werase (mainwin_front);
  
  werase (consolewin_rear);
  werase (consolewin_front);
  
  werase (statwin_rear);
  werase (statwin_front);
  
  werase (invwins_rear);
  
  for (int x=0; x<INV_X; x++)
    {
      for (int y=0; y<INV_Y; y++)
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
 
void CursesUI :: DecorateWindows()
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
 
void CursesUI :: Info()
{
  wprint_at (consolewin_front,(const char *)"Created by Rave Kutsuu",0,0); //Please leave this untouched as proof of origin
  wprint_at (consolewin_front,(const char *)"ARQ Learner project/Tech demo",2,0);
  wprint_at (consolewin_front,(const char *)"Made using C++ and ncurses",3,0);
  wgetch (consolewin_front);
}

int CursesUI :: wprintw_col (WINDOW* winchoice, const char *text, int color_choice) //allows ncurses window and colour selection when outputting characters
{
  wattron(winchoice,COLOR_PAIR(color_choice)); //turns on the chosen colour pair
  wprintw (winchoice, text); 
  wattroff(winchoice,COLOR_PAIR(color_choice)); //turns off the chosen colour pair
  return (0);
};             

int CursesUI :: wprint_at (WINDOW* winchoice, const char *text, int pos_y, int pos_x)
{
  wmove (winchoice,pos_y,pos_x);
  wprintw (winchoice, text); 
  wrefresh (winchoice);
  return (0);
}

int CursesUI :: wprintNoRefresh (WINDOW* win, std::string text)
{
  wmove (win,0,0);
  werase(win);
  
  wprintw (win, text.c_str());
  return 0;
}
 
void CursesUI :: DrawItems(Map* m)
{
  //iterate through the rows of the grid/map
  for (int y = 0; y < GRID_Y; y++)
    {	  
      for(int x = 0; x < GRID_X; x++)
	{
	  //create variables to hold item info
	  int colour;
	  const char *symbol;
        
	  //grab the info for each item from the library
	  item* i = m->GetItem(x,y);
      
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

void CursesUI :: DrawAreas(Map* m)
{
  //iterate through the rows of the grid/map
  for (int y = 0; y < GRID_Y; y++)
    {	  
      for(int x = 0; x < GRID_X; x++)
	{
	  //create variables to hold item info
	  int colour;
	  const char *symbol;
        
	  //grab the info for each item from the library
	  area* a = m->GetArea(x,y);
      
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
 
int CursesUI :: DrawMap (Map* m, bool fogOfWar)
{
  wmove (mainwin_front,0,0);
  
  int viewDistance = 3;
  
  int startX=0;
  int startY=0;
  int endX = GRID_X;
  int endY = GRID_Y;
  
  if (fogOfWar) {

      player->GetPos(&startX,&startY);
      
      if(startX<GRID_X-viewDistance && startY<GRID_Y-viewDistance) {
      endX = startX+viewDistance;
      endY = startY+viewDistance;
      }
      
      if(startX>viewDistance && startY>viewDistance) {
      startX-=viewDistance;
      startY-=viewDistance;
     }
      else {
          startX = 0;
          startY = 0;
          
      }
  }

  

  
  for (int y=startY; y<endY; y++)
    {
      wmove (mainwin_front,y,0);
  
      for(int x=startX; x<endX; x++)
	{
	  int maptile;
	  int maptile_colour;
	  const char *maptile_char;
      
	  maptile = m->GetTile(x,y);
     
	  maptile_colour = tile_library[maptile].color;
	  maptile_char = tile_library[maptile].symbol;
     
	  wmove (mainwin_front,y,x);
	  wprintw_col (mainwin_front, maptile_char, maptile_colour);
	};
	 
    };
<<<<<<< d3f3a215c1775b154dccf4b7b491a121e003c472
=======

>>>>>>> Major refactoring/Improvements, jumped to 0.90.0
   
  return (0);
};

void CursesUI :: DrawCharacter(int x, int y, int colour, char symbol)
{ 
  if ((x < 0) || (x > GRID_X))
    {
      return;
    }
  
  else if ((y < 0) || (y > GRID_Y))
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

void CursesUI :: DrawPlayerInv()
{
  for (int y = 0; y < 3; y++)
    {
      //draw each item on this row to the screen
      for(int x = 0; x < 3; x++)
	{
	  WINDOW* thisWindow = invwins_front[y][x];
         
	  werase(thisWindow);
	  wmove(thisWindow,0,0); 
		 
	  item* itm = player->GetFromInventory(x,y); 
		   
	  int colour = itm->colour;
	  std::string str = itm->name;
         
	  const char* symbol = str.c_str();

	  wprintw_col (thisWindow, symbol, colour);         
	}; 
    };  
  return;
};

void CursesUI :: DrawPlayerEquip ()
{
  //draw weapons
  for(int x = 0; x < INV_X; x++)
    {
      WINDOW* thisWindow = equipwin_front[x];
    
      werase(thisWindow);
      wmove(thisWindow,0,0); 
	
      weapon* weps = player->GetWeps();
		   
      int colour = weps[x].colour;
      std::string str = weps[x].name;
         
      const char* symbol = str.c_str();
      wprintw_col (thisWindow, symbol, colour);       
    }; 
   
    
  werase(equipwin_outfit);
  wmove(equipwin_outfit,0,0); 
  
  outfit currentOutfit = player->GetOutfit();
		   
  int colour = currentOutfit.colour;
  std::string str = currentOutfit.name;
         
  const char* nameChars = str.c_str();

  wprintw_col (equipwin_outfit, nameChars, colour);         
  
  return;
};
 
void CursesUI :: DrawPlayerStats ()
{ 
  WINDOW* winchoice = statwin_front;
	
  werase (winchoice);
  wmove (winchoice,0,1);
  wattron(winchoice,A_UNDERLINE); //turns on the chosen colour pair
  wprintw (winchoice,"%s",player->GetName().c_str());
  wattroff(winchoice,A_UNDERLINE); //turns off the chosen colour pair
 
  wmove (winchoice,1,0);
  wprintw_col (winchoice,"Health: ",4); 
  wprintw (winchoice,"%d",player->GetHealth());
 
  wmove (winchoice,2,0);
  wprintw_col (winchoice,"Loot:   ",4); 
  wprintw (winchoice,"%d",player->GetLootScore());
 
  for( int a = 0; a < MAX_NPCS; a++ )
    {
      wmove (winchoice,a+3,0);
      //LIST HEALTH OF NPCS?
    };
     
  return;
}

//make sure any overloads of this func are up-to-date with the header
void CursesUI :: DrawInv(container* c)
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
void CursesUI :: DrawInv(area* a)
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

int CursesUI :: init_screen ()
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


void CursesUI :: UpdateUI()
{
    DecorateWindows();
    
  for (int x=0; x<3; x++)
    {
      for (int y=0; y<3; y++)
	{
	  wrefresh (invwins_front[x][y]);
	}
       
      wrefresh (equipwin_front[x]);  
    } 
	 
  wrefresh (titlewin);

  wrefresh (mainwin_rear);
  wrefresh (consolewin_rear);
  wrefresh (statwin_rear);
  
  wrefresh (mainwin_front);
  wrefresh (consolewin_front);
  wrefresh (statwin_front);
  
  wrefresh (equipwin_outfit);
}

 
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CursesUI :: Battle (int npc_id)
{
  string p_name = player->GetName();
  string npc_name = npcs[npc_id].GetName();
  
  werase (consolewin_front); //Clears both the input and map windows to fit the battle scenario
  werase (mainwin_front);
 
  while (true)
    {
      //if either has died, kill them and return
      if (player->IsAlive() == false)
	{
	  werase (consolewin_front);
	  wprintw (consolewin_front, "You have been slain..");
	  player->Kill();
          
	  wgetch(consolewin_front);
	  return;
	}
  
      else if (npcs[npc_id].IsAlive() == false)
	{
	  werase (consolewin_front);
	  wprintw (consolewin_front, "Your foe has been slain..");
    
	  npcs[npc_id].Kill();
	  wgetch(consolewin_front);
    
	  return;
	}
  
      else
	{
	  //display the info of both combatants
	  wmove (consolewin_front,0,0);
	  wprintw (consolewin_front,"%s Health: %d \n",p_name.c_str(),player->GetHealth());
	  wprintw (consolewin_front,"%s Health: %d \n",npc_name.c_str(),npcs[npc_id].GetHealth());
	  wgetch (consolewin_front);
  
	  //Get both combatants weapons
	  weapon* p_weps = player->GetWeps();
	  weapon* npc_weps = npcs[npc_id].GetWeps();
  
	  //allow both combatants to make a move
	  int p_move = playerUI.BattleTurn(npc_id); 
	  int npc_move = npcs[npc_id].BattleTurn();  
    
	  if ( (p_move<=2) && (npc_move<=2) && (npcs[npc_id].IsAlive()) && (player->IsAlive()) ) 
	    {
	      //select the weapon using their move choice
	      weapon p_choice = p_weps[p_move];  
	      weapon npc_choice = npc_weps[npc_move];
  
	      //show each players choices
	      werase (consolewin_front);
      
	      if (p_move != 0)
		{
		  wprintw(consolewin_front,"You use your %s and strike the enemy for %i damage\n",p_choice.name.c_str(),p_choice.damage);
		}
      
	      else if (p_move == 0 && npc_move != 0)
		{
		  wprintw(consolewin_front,"You do nothing, and are struck for %i damage", npc_choice.damage);
		};
       
	      if (npc_move != 0 && p_move != 0)
		{
		  wprintw(consolewin_front,"Your enemy uses their %s and deals %i damage \n",npc_choice.name.c_str(),npc_choice.damage);
		}
       
	      else if (npc_move == 0 && p_move != 0)
		{
		  wprintw(consolewin_front,"Your enemy does nothing and takes %i damage \n",p_choice.damage);
		}
             
	      else
		{
		  wprintw(consolewin_front,"You both do nothing.");
		};
      
	      wgetch (consolewin_front);
	      player->SetHealth(player->GetHealth()-10);
	      npcs[npc_id].SetHealth(npcs[npc_id].GetHealth()-p_choice.damage);
	    }
     
	  else if (p_move == 3)
	    {
              return;  //default exit for now, need to refactor this
          
	     /** werase (consolewin_front);
	      wprintw(consolewin_front,"You attempt to flee."); 
	  
	      wgetch (consolewin_front);
	  
	      if (player->CanFlee(map))
		{
		  if (player->Flee(map) == 0)
		    {
		      wprintw(consolewin_front,"You manage to escape the fight."); 
		      wgetch (consolewin_front);
		      return;
		    }
		 
		  else wprintw(consolewin_front,"You try to escape but have nowhere to run to."); 

                  wgetch (consolewin_front); 

		} */
	  
	    }
	}
    }
  
}


item* CursesUI :: GetFromInventory ()
{
  area* a = player->GetInventory();

  std::string slc_string;
  char slc_char[20];
  
  int loc_x = 0;
  int loc_y = 0;
  
  //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
  bool selection = true;

  while (selection == true) 
    {
      for (int x=0; x<3; x++)
	{
	  for (int y=0; y<3; y++)
	    {
	      WINDOW* thisWin = invwins_front[y][x];
        
	      mvwchgat (thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
	      wrefresh (thisWin);
	    }
	}
      
      DrawInv(a);
      UpdateUI();

      WINDOW* selWin = invwins_front[loc_y][loc_x]; //create a holder for the currently selected window
    
      mvwchgat (selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
      wrefresh (selWin);
     
      werase (consolewin_front);
      wprint_at (consolewin_front, "Select an item with WASD.. 'put' to transfer to the container.", 0,0);
    
      wprint_at (consolewin_front,"ARQ:~$ ",2,0);
    
      wgetstr (consolewin_front, slc_char);
      slc_string = slc_char;
    
      if ((slc_string == "W") || (slc_string == "w")) 
	{
	  if (loc_x != 0)
	    {
	      loc_x--;
	    };
	}
  
      else if ((slc_string == "A") || (slc_string == "a")) 
	{
	  if (loc_y != 0)
	    {
	      loc_y--;
	    };
	}
     
      else if ((slc_string == "S") || (slc_string == "s")) 
	{
	  if (loc_x != 2)
	    {
	      loc_x++;
	    };
	}
  
      else if ((slc_string == "D") || (slc_string == "d")) 
	{
	  if (loc_y != 2)
	    {
	      loc_y++;
	    }
	}
    
      else if ((slc_string == "Exit") || (slc_string == "exit") || (slc_string == "EXIT")) 
	{
	  selection = false;
	  return (NULL);
	}
    
      else if (slc_string == "put") 
	{
	  item* thisItem = a->GetItem(loc_x,loc_y);
          a->RemoveItem(loc_x,loc_y);

          return thisItem;      
	}
            
      else 
	{
	  werase (consolewin_front);
	  wprint_at (consolewin_front, "Not a correct selection, try again.", 0, 0);
	  wgetch (consolewin_front);
	};
     
      std::cout << "\n " << loc_x << " " << loc_y;
    };

  return (NULL);
};

int CursesUI :: ItemProc (item* itm, int x, int y)
{
  string answer;
  char answerchar[20];
  string itm_name = itm->name; 
 
  werase (consolewin_front);
  wmove (consolewin_front,0,0);
  wprintw (consolewin_front,"There's a %s on the floor..", itm_name.c_str());
  wrefresh (consolewin_front);
 
  wgetch (consolewin_front); 
 
  wmove (consolewin_front,0,0);
  wprintw (consolewin_front,"Would you like to pick the %s? ",itm_name.c_str());
  wgetstr (consolewin_front, answerchar);
 
  answer = (answerchar);
      
  if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
    {
      if (player->AddToInventory (itm) == (1)) //If addToInventory unsuccessful
	{
	  werase (consolewin_front);
	  wprint_at (consolewin_front,(const char *)"Inventory full..",0,0);
	  wgetch (consolewin_front);
	  return (1);
	}
    
      else
	{
	  wmove (consolewin_front,0,0);
	  werase (consolewin_front);
	  wprintw (consolewin_front,"You pick up the %s..",itm_name.c_str());
	  wgetch (consolewin_front);
      
	  player->SetInventoryTile(x,y,new item(item_library[no_item]));
 
	  player->SetPos(x,y);

	  return (0);
	};
    }
   
  else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
    {
      wmove (consolewin_front,0,0);
      werase (consolewin_front);
      wprintw (consolewin_front,"You leave the %s untouched..",itm_name.c_str());
      wgetch (consolewin_front);
     
      player->SetPos(x,y); 
      return (0);
    }
    
  else 
    {
      wprint_at (consolewin_front, "Incorrect choice, please answer yes or no.. ",0,0);
      wgetch (consolewin_front);
      
      ItemProc (itm,x,y);
    };
    
  return (0);
};

int CursesUI :: LockProc (int door_y, int door_x, tile doortype, int doortile, string doorname )
{
  string answer;
  item* inv_tile;
  char answerchar[20];
  werase (consolewin_front);
  wprint_at (consolewin_front,"How would you like to unlock the door? ",0,0);
  wprint_at (consolewin_front,"1. Using Door Keys, 2. Using Lockpicks ",1,0);
  wprint_at (consolewin_front,"Enter a choice to continue, or 'exit' to cancel ",2,0);
  wprint_at (consolewin_front,"lock_proc:~$  ",3,0);
  wgetstr (consolewin_front, answerchar);
  answer = (answerchar);
      
  if ((answer == "1") || (answer == "Key") || (answer == "Keys") || (answer == "key") || (answer == "keys") || (answer == "Door Key") || (answer == "Door Keys") || (answer == "door key") || (answer == "door keys"))
    {
      int key_count;
      int count;
      count = 0;
      key_count = 0;
      
      for (int y = 0; y < INV_Y; y++)
	{
	  for(int x = 0; x < INV_X; x++)
	    {
	      inv_tile = player->GetFromInventory(x,y);
          
	      if (inv_tile->id == key)
		{
		  key_count = (key_count+1);
		};
          
	    };
	}; 
     
      if (key_count == (tile_library[doortile].locks))
	{
            
	  for (int y = 0; y < INV_Y; y++)
	    {
	      for(int x = 0; x < INV_X; x++)
		{
		  inv_tile = player->GetFromInventory(x,y);
           
		  if (inv_tile->id == (key))
		    {
		      player->SetInventoryTile(x,y,new item(item_library[no_item]));
		      count = (count+1);
		    };
                 
		  if (count == (tile_library[doortile].locks))
		    {
		      werase (consolewin_front);
		      wmove (consolewin_front,0,0);
		      wprintw (consolewin_front,(const char *)"You insert %d keys into the door and open it .. ",tile_library[doortile].locks);
		      wgetch (consolewin_front);
       
		      if (doortype == ld1) 
			{
			  map->SetTile(door_x,door_y,od1);
               
			  if (map->GetTile(door_x,door_y+1) == (ld1)) //Checking for surrounding door tiles
			    {
			      map->SetTile(door_x,door_y+1,od0);
			    }
          
			  else if (map->GetTile(door_x,door_y-1) == (ld1)) 
			    {
			      map->SetTile(door_x,door_y-1,od0);
			    }
          
			  else if (map->GetTile(door_x+1,door_y) == (ld1)) 
			    {
			      map->SetTile(door_x+1,door_y,od0);
			    }
            
			  else if (map->GetTile(door_x-1,door_y) == (ld1)) 
			    {
			      map->SetTile(door_x-1,door_y,od0);
			    };
			}
                  
		      else if (doortype == ld2)
			{
			  map->SetTile(door_x,door_y,od2);

			  if (map->GetTile(door_x,door_y+1) == (ld2)) //Checking for surrounding door tiles
			    {
			      map->SetTile(door_x,door_y+1,od0);
			    }
          
			  else if (map->GetTile(door_x,door_y-1) == (ld2)) 
			    {
			      map->SetTile(door_x,door_y-1,od0);
			    }
          
			  else if (map->GetTile(door_x+1,door_y) == (ld2)) 
			    {
			      map->SetTile(door_x+1,door_y,od0);
			    }
            
			  else if (map->GetTile(door_x-1,door_y) == (ld2)) 
			    {
			      map->SetTile(door_x-1,door_y,od0);
			    };
			};
             
		      player->SetPos(door_x,door_y);               
		      return (0);
		    };
            
		};
	    }; 
             
	};
           
      if (key_count != (tile_library[doortile].locks))
	{
	  werase (consolewin_front);
	  wmove (consolewin_front,0,0);
	  wprintw (consolewin_front,"You need %d keys to open this door.. ",tile_library[doortile].locks);
	  wgetch (consolewin_front);
	  return (0);
	};
      
      return (0);
    }
    
  else if ((answer == "2") || (answer == "Lockpick") || (answer == "Lockpicks") || (answer == "lockpick") || (answer == "lockpicks") || (answer == "Lock Pick") || (answer == "Lock Picks") || (answer == "lock pick") || (answer == "lock picks"))
    {
      int lockpick_count;
      int count;
      count = 0;
      lockpick_count = 0;
      
      for (int y = 0; y < INV_Y; y++)
	{
	  for(int x = 0; x < INV_X; x++)
	    {
	      inv_tile = player->GetFromInventory(x,y);
          
	      if (inv_tile->name == item_library[lockpick].name)
		{
		  (lockpick_count = (lockpick_count+1));
		};
          
	    };
	};
       
      if (lockpick_count >= (tile_library[doortile].locks))
	{
         
	  for (int y = 0; y < INV_Y; y++)
	    {
	      for(int x = 0; x < INV_X; x++)
		{
		  inv_tile = player->GetFromInventory(x,y);
            
		  if (inv_tile->name == item_library[lockpick].name)
		    {
		      int chance = rand() %100+1;
		      int lockno;
              
		      lockno = 1;
              
		      werase (consolewin_front);
              
		      wmove (consolewin_front,0,0);
		      wprintw (consolewin_front,"This door has %d lock(s).. ",tile_library[doortile].locks);
              
		      wmove (consolewin_front,1,0);
		      wprintw (consolewin_front,"You attempt to pick lock %d.. ", lockno);
              
		      wgetch (consolewin_front);
              
		      if (chance > 50)
			{
			  werase (consolewin_front);
			  wprint_at (consolewin_front,"You manage to open the lock with the lockpick.. ",0,0);
			  wgetch (consolewin_front);
                 
			  player->SetInventoryTile(x,y,new item (item_library[no_item]));
			  count = (count+1);
			  chance = rand() %100+1;
			  lockno = (lockno+1);
			}
              
		      else if (chance <= 50)
			{
			  werase (consolewin_front);
			  wprint_at (consolewin_front,"Your lockpick breaks as you attempt to open the lock.. ",0,0);
			  wgetch (consolewin_front);
                    
			  player->SetInventoryTile(x,y,new item(item_library[no_item]) );
			  chance = rand() %100+1;
			};
                  
		    };
                 
		  if (count == (tile_library[doortile].locks))
		    {
		      werase (consolewin_front);
		      wprint_at (consolewin_front, "You manage to unlock the door.. ",0,0);
		      wgetch (consolewin_front);
                  
		      if (doortype == ld1) 
			{
			  map->SetTile(door_x,door_y,od2);
			}
                  
		      else if (doortype == ld2)
			{
			   map->SetTile(door_x,door_y,od2);
			};
                                    
		      player->SetPos(door_x,door_y);  
		      return (0);
		    };
                 
		};
	    }; 
             
	} 
     
      else if (lockpick_count != (tile_library[doortile].locks))
	{
	  werase (consolewin_front);
	  wmove (consolewin_front,0,0);
	  wprintw (consolewin_front,(const char *)"You need %d lock picks to attempt to open this door.. ",tile_library[doortile].locks);
	  wgetch (consolewin_front);
	  return (0);
	};
     
      if (count != (tile_library[doortile].locks))
	{
	  werase (consolewin_front);
	  wprint_at (consolewin_front, "You have run out of lockpicks..",0,0);
	  wgetch (consolewin_front);
	  return (0);
	};
     
      return (0);
    }
            
  else if ((answer == "Exit") || (answer == "EXIT") || (answer == "exit"))
    {
      werase (consolewin_front);
      wmove (consolewin_front,0,0);
      wprintw (consolewin_front,(const char *)"You leave the %s untouched. ",doorname.c_str());
      wgetch (consolewin_front);
      return (0);
    }
	    
  else
    {
      wprint_at (consolewin_front,(const char *)"Not a correct choice, try again.. ",0,0);
      LockProc (door_y,door_x,doortype,doortile,doorname);
    };
    
  return (0);
};

PlayerUI* CursesUI :: getPlayerUI() {
    return &playerUI;
}
 







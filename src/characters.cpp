#include <iostream>
#include <string>
#include <curses.h>

#include "game.h"
#include "grid.h"

#include "ui.h"

using namespace std;

NPC * npcs[max_npcs];

void Battle (WINDOW* main_win, WINDOW* input_win, int npc_id, Player* player, NPC* npcs[])
{
  string p_name = player->GetName();
  string npc_name = npcs[npc_id]->GetName();
  
  werase (input_win); //Clears both the input and map windows to fit the battle scenario
  werase (main_win);
 
  while (true)
    {
      //if either has died, kill them and return
      if (player->IsAlive() == false)
	{
	  werase (input_win);
	  wprintw (input_win, "You have been slain..");
	  player->Kill();
	  wgetch(input_win);
	  return;
	}
  
      else if (npcs[npc_id]->IsAlive() == false)
	{
	  werase (input_win);
	  wprintw (input_win, "Your foe has been slain..");
    
	  npcs[npc_id]->Kill();
	  wgetch(input_win);
    
	  return;
	}
  
      else
	{
	  //display the info of both combatants
	  wmove (input_win,0,0);
	  wprintw (input_win,"%s Health: %d \n",p_name.c_str(),player->GetHealth());
	  wprintw (input_win,"%s Health: %d \n",npc_name.c_str(),npcs[npc_id]->GetHealth());
	  wgetch (input_win);
  
	  //Get both combatants weapons
	  weapon* p_weps = player->GetWeps();
	  weapon* npc_weps = npcs[npc_id]->GetWeps();
  
	  //allow both combatants to make a move
	  int p_move = player->BattleTurn(main_win,input_win,npc_id); 
	  int npc_move = npcs[npc_id]->BattleTurn();  
    
	  if ( (p_move<=2) && (npc_move<=2) && (npcs[npc_id]->IsAlive()) && (player->IsAlive()) ) 
	    {
	      //select the weapon using their move choice
	      weapon p_choice = p_weps[p_move];  
	      weapon npc_choice = npc_weps[npc_move];
  
	      //show each players choices
	      werase (input_win);
      
	      if (p_move != 0)
		{
		  wprintw(input_win,"You use your %s and strike the enemy for %i damage\n",p_choice.name.c_str(),p_choice.damage);
		}
      
	      else if (p_move == 0 && npc_move != 0)
		{
		  wprintw(input_win,"You do nothing, and are struck for %i damage", npc_choice.damage);
		};
       
	      if (npc_move != 0 && p_move != 0)
		{
		  wprintw(input_win,"Your enemy uses their %s and deals %i damage \n",npc_choice.name.c_str(),npc_choice.damage);
		}
       
	      else if (npc_move == 0 && p_move != 0)
		{
		  wprintw(input_win,"Your enemy does nothing and takes %i damage \n",p_choice.damage);
		}
             
	      else
		{
		  wprintw(input_win,"You both do nothing.");
		};
      
	      wgetch (input_win);
	      player->SetHealth(player->GetHealth()-10);
	      npcs[npc_id]->SetHealth(npcs[npc_id]->GetHealth()-p_choice.damage);
	    }
     
	  else if (p_move == 3)
	    {
	      werase (input_win);
	      wprintw(input_win,"You attempt to flee."); 
	  
	      wgetch (input_win);
	  
	      if (player->CanFlee())
		{
		  if (player->Flee() == 0)
		    {
		      wprintw(input_win,"You manage to escape the fight."); 
		      wgetch (input_win);
		  
		      return;
		    }
		 
		  else
		    {
		      wprintw(input_win,"You try to escape but have nowhere to run to."); 
		      wgetch (input_win);
		    }
		}
	  
	    }
	}
    }
  
}

bool Character :: IsAlive ()
{
  if (health<=0 && alive==true)
    {
      this->Kill();
      return false;
    }
  
  else
    {
      return alive;
    }
  
};
 
bool Character:: IsTraversable(tile t)
{	 
  if ((t != ntl) && ((t == cor) || (t == rom)))
    {
      return true;  
    }
  
  else 
    {
      return false;
    }
}
 
bool Character :: CanFlee()
{
  if (health>(health/100)*25)
    {
      return true;
    }
  
  else
    {
      return false;
    }
}
 
int Character :: Flee()
{
  int thisX = this->x;
  int thisY = this->y;
  
  if (IsTraversable(GetTile(thisX+1,y)) )
    {
      SetPos(x+1,y);
      return 0;
    }
   
  else if (IsTraversable (GetTile(thisX-1,y)) ) 
    {
      SetPos(x-1,y);
      return 0;
    }
   
  else if (IsTraversable(GetTile(x,thisY+1)) ) 
    {
      SetPos(x,y+1);
      return 0;
    }
   
  else if (IsTraversable(GetTile(x,thisY-1)) ) 
    {
      SetPos(x,y-1);
      return 0;
    }
 
  else
    {
      return 1;
    }
}

void Character :: Draw()
{
  DrawCharacter(x,y,colour, symbol);
}

void Character :: SetCharacter (char char_choice, int colour_choice, std::string name_choice, int health_choice)
{
  alive = true;
  this->symbol = char_choice;
  this->colour = colour_choice;
  this->name = name_choice;
  this->health = health_choice;
  
  void SetPos(int x, int y);
};

void Character :: Kill()
{
  alive = false;
  DropItems();
};

std::string Character :: GetName()
{
  return this->name;
} 

void Character :: GetPos (int *x, int *y)
{
  (*x = this->x);
  (*y = this->y);
};

int Character :: GetHealth ()
{
  return this->health;
};
  
void Character :: SetHealth(int i)
{
  this->health = i;
  return;
}

void Character :: SetPos(int x, int y)
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
      this->x = x;
      this->y = y;
    };
  return;
};

void Character :: DropItems()
{
  string name = this->name.c_str();
  string thisName = "Dead " + name;
  area* body = new area(1,thisName,"X",1,0,true);
  
  body->AddItem(new weapon(this->weps[1]));
  body->AddItem(new weapon(this->weps[2]));

  //thisContainer->ReplaceItem(1,1,new outfit(this->currentOutfit));

  SetArea(this->x,this->y,body);
}
 
////////////////////////////////////////////////

bool Player :: CanDropItem(item* thisItem)
{
  if (thisItem->lootable)
    {    
      return true;
    }
            
  else
    {
      return false;
    }; 
}

int Player :: DropItem(item* thisItem, int invX, int invY)
{    
  tile playerTile = GetTile(this->x,this->y);
 
  //if the player is at an area where items can be placed, add the item
  if ((playerTile == ntl) || (playerTile == rom) || (playerTile == cor))
    {	     
      AddToArea(x,y,thisItem); //replace the map tile with the item
    
      SetInventoryTile(invX,invY,new item(item_library[no_item])); //clear the inventory tile

      return 0;
    } 
     
  else
    {
      return 1;
    };
      
}



int Player :: AccessArea (WINDOW* input_win, WINDOW* inv_wins[3][3], area* a)
{
  std::string slc_string;
  char slc_char[20];
  
  int loc_x = 0;
  int loc_y = 0;
  
  //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
  bool selection = true;

    werase(input_win); wprintw(input_win,"Accessing Area");
    wgetch (input_win);

  while (selection == true) 
   { 
      item* thisItem = a->GetItem(loc_x,loc_y);

      for (int x=0; x<3; x++)
	{
	  for (int y=0; y<3; y++)
	    {
	      WINDOW* thisWin = inv_wins[y][x];
        
	      mvwchgat (thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
	      
	    }
	}

      //area b = *a;
      DrawInv(a);
      UpdateUI(); //ensures items display propely
      
      WINDOW* selWin = inv_wins[loc_y][loc_x]; //create a holder for the currently selected window
      
      mvwchgat (selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
      wrefresh (selWin);
     
      werase (input_win);
      wprint_at (input_win, "Select an item with WASD.. 'help' for commands", 0,0);
    
      wprint_at (input_win,"ARQ:~$ ",2,0);
    
      wgetstr (input_win, slc_char);
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
    
      else if (slc_string == "close") 
	{
	  selection = false;
	  
	  return (0);
	}
    
      else if (slc_string == "what") 
	{
	  wmove (input_win,0,0);
	  werase(input_win);
      
	  wprintw (input_win, "This is a %s.",thisItem->name.c_str());
	  wrefresh (input_win);
    
	  wgetch(input_win);      
	}
 
      else if (slc_string == "take") 
	{
	  if (IsLootable(thisItem))
	    {
	      this->inventory->AddItem(thisItem);
	      a->RemoveItem(loc_x,loc_y);

	      werase(input_win);
	      wprintw (input_win, "You put the %s into your inventory.",thisItem->name.c_str());
	      wgetch(input_win);
	    }
	}

      
      else if (slc_string == "put") 
	{
	  item* thisItem = GetFromInventory(input_win,inv_wins);
	  
	  werase (input_win);
	  wprintw (input_win, "You put the %s inside.",thisItem->name.c_str());
	  wrefresh (input_win);

	  wgetch(input_win);

	  if (thisItem != NULL)
	    {
	      a->AddItem(thisItem);
	    }
	}

      else if (slc_string == "help") 
	{
	  wmove (input_win,0,0);
	  werase(input_win);
      
	  wprintw (input_win, "what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container");
	  wrefresh (input_win);
    
	  wgetch(input_win);      
	}
   
      else 
	{
	  werase (input_win);
	  wprint_at (input_win, "Not a correct selection, try again.", 0, 0);
	  wgetch (input_win);
	};
     
      cout << "\n " << loc_x << " " << loc_y;
    };

  return (0);
};

int Player :: AccessContainer (WINDOW* input_win, WINDOW* inv_wins[3][3], container* c)
{
  std::string slc_string;
  char slc_char[20];
  
  int loc_x = 0;
  int loc_y = 0;
  
  //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
  bool selection = true;
  
  werase(input_win); wprintw(input_win,"Accessing Container");
  wgetch (input_win);

  while (selection == true) 
    {
      item* thisItem = c->GetItem(loc_x,loc_y);

      for (int x=0; x<3; x++)
	{
	  for (int y=0; y<3; y++)
	    {
	      WINDOW* thisWin = inv_wins[y][x];
        
	      mvwchgat (thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
	      wrefresh (thisWin);
	    }
	}
        
      DrawInv(c); //draw the current items
      UpdateUI(); //call a full system UI refresh to fix display bug

      WINDOW* selWin = inv_wins[loc_y][loc_x]; //create a holder for the currently selected window
    
      mvwchgat (selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
      wrefresh (selWin);
     
      werase (input_win);
      wprint_at (input_win, "Select an item with WASD.. 'help' for commands", 0,0);
    
      wprint_at (input_win,"ARQ:~$ ",2,0);
    
      wgetstr (input_win, slc_char);
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
    
      else if (slc_string == "close") 
	{
	  selection = false;
	  return (0);
	}
    
      else if (slc_string == "what") 
	{
	  wmove (input_win,0,0);
	  werase(input_win);
      
	  wprintw (input_win, "This is a %s.",thisItem->name.c_str());
	  wrefresh (input_win);
    
	  wgetch(input_win);      
	}
 
      else if (slc_string == "take") 
	{
	  this->inventory->AddItem(thisItem);
          c->RemoveItem(loc_x,loc_y);
	  
          werase(input_win);
	  wprintw (input_win, "You put the %s into your inventory.",thisItem->name.c_str());
	  wgetch(input_win);
	}

      
      else if (slc_string == "put") 
	{
	  item* thisItem = GetFromInventory(input_win,inv_wins);
	  
	  if (thisItem != NULL)
	    {
	      c->AddItem(thisItem);
	    }
	}

      else if (slc_string == "help") 
	{
	  wmove (input_win,0,0);
	  werase(input_win);
      
	  wprintw (input_win, "what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container");
	  wrefresh (input_win);
    
	  wgetch(input_win);      
	}
   
      else 
	{
	  werase (input_win);
	  wprint_at (input_win, "Not a correct selection, try again.", 0, 0);
	  wgetch (input_win);
	};
     
      cout << "\n " << loc_x << " " << loc_y;
    };

  return (0);
};

int Player :: AccessInventory (WINDOW* input_win, WINDOW* inv_wins[3][3])
{
  area* a = this->inventory;

  item* inv_tile;
  int invtile_colour;
  
  const char* thisChar;
  std::string slc_string;
  char slc_char[20];
  
  int loc_x = 0;
  int loc_y = 0;
  
  //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
  bool selection = true;

  while (selection == true) 
    {
      item* thisItem = a->GetItem(loc_x,loc_y);

      for (int x=0; x<3; x++)
	{
	  for (int y=0; y<3; y++)
	    {
	      WINDOW* thisWin = inv_wins[y][x];
        
	      mvwchgat (thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
	      wrefresh (thisWin);
	    }
	}

      WINDOW* selWin = inv_wins[loc_y][loc_x]; //create a holder for the currently selected window
    
      mvwchgat (selWin, 0, 0, 9, A_BLINK, 2, NULL); //make the current window blink red
      wrefresh (selWin);
     
      //DrawInv(a);
      UpdateUI();

      werase (input_win);
      wprint_at (input_win, "Select an item with WASD.. 'help' for commands.", 0,0);
    
      wprint_at (input_win,"ARQ:~$ ",2,0);
    
      wgetstr (input_win, slc_char);
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
    
      else if (slc_string == "close") 
	{
	  selection = false;
	  return (0);
	}
    
      else if (slc_string == "what") 
	{
	  wmove (input_win,0,0);
	  werase(input_win);
      
	  wprintw (input_win, "This is a %s.",thisItem->name.c_str());
	  wrefresh (input_win);
    
	  wgetch(input_win);      
	}
 
      else if (slc_string == "drop") 
	{
	  werase (input_win);
     
	  item* thisItem = a->GetItem(loc_x,loc_y);
      
	  if (CanDropItem(thisItem) == true)
	    {
	     
	      if (DropItem(thisItem, loc_x, loc_y) == 0)
	      {
		//update the current item details 
		item* thisItem = inventory->GetItem(loc_x,loc_y);
		const char* invtile_char = thisItem->name.c_str();
          
		WINDOW* thisWin = inv_wins[loc_x][loc_y];
          
		//update the inventory tile
		wmove (thisWin,0,0);
		wprintw_col (thisWin, invtile_char, thisItem->colour);
		wrefresh (thisWin);
           
		wprintw (input_win, "Item dropped.");
	      }
	      
	      else 
		{
		  wprintw (input_win, "Cannot drop that here.");
		}
	    }
         
      
	  else
	    {
	      wprintw (input_win, "Cannot drop this item.");
	    };
      
	  wgetch(input_win);
	}
     
      else if (slc_string == "wear") 
	{
	  if (IsLootable (a->GetItem(loc_x,loc_y) ))
	    {
	      inv_tile = a->GetItem(loc_x,loc_y); //grab the current inventory tile
                    
	      invtile_colour = inv_tile->colour;
	      thisChar = inv_tile->symbol;
        
	      WINDOW* thisWin = inv_wins[loc_x][loc_y];
           
	      const char *invtile_char = thisChar;
        
	      wmove (thisWin,0,0);
	      wprintw_col (thisWin, invtile_char, invtile_colour);
	      wrefresh (thisWin);
        
	      for (int i=0; i<outfit_size; i++)
		{
		  werase (input_win);
		  wprintw (input_win, "%s selected.",thisChar);
		  wgetch(input_win);
        
		  //if this item matches an outfit, assume it is and equip it
		  if (thisChar == outfit_library[i].symbol)
		    {
		      //store the old outfit
		      outfit oldOutfit = currentOutfit;
            
		      wprintw (input_win, "You change from %s into %s",currentOutfit.name.c_str(),thisChar);
		      wgetch(input_win);
            
		      //change into the new outfit
		      SetOutfit (outfit_library[i]);
            
            
		      wprintw (input_win, "You put your old %s into your inventory",oldOutfit.name.c_str());
		      wgetch(input_win);
            
		      //set the current inventory tile to the old outfit
		      inventory->ReplaceItem(loc_x,loc_y,new outfit(oldOutfit)); //instanciates a new outfit to fix polymorphism issues
            
       
	 	    }
		}
        
        
	    }
       
	  else
	    {
	      werase (input_win);
	      wprintw (input_win, "No item selected.");
	      wgetch(input_win);
	    };
        
	}
      
      else if (slc_string == "open")
	{
	  int thisId = inventory->GetItem(loc_x,loc_y)->id;
	  
	  //98 denotes container, which is a closed-ended area, we never want to access an area here
	  if (thisId == 98)
	    {
	      wprintNoRefresh(input_win,"You open the container");
	      container* c = (container*) inventory->GetItem(loc_x,loc_y);

	      AccessContainer(input_win,inv_wins,c);
	      DrawInv(a);
	    }
	}
	 
      else if (slc_string == "help") 
	{
	  wprintNoRefresh (input_win, "what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container");
	  wrefresh (input_win);
    
	  wgetch(input_win);      
	}
 
      else 
	{
	  werase (input_win);
	  wprint_at (input_win, "Not a correct selection, try again.", 0, 0);
	  wgetch (input_win);
	};
     
      cout << "\n " << loc_x << " " << loc_y;
    };

  return (0);
};


item* Player :: GetFromInventory (WINDOW* input_win, WINDOW* inv_wins[3][3])
{
  area* a = this->inventory;

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
	      WINDOW* thisWin = inv_wins[y][x];
        
	      mvwchgat (thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
	      wrefresh (thisWin);
	    }
	}
      
      DrawInv(a);
      UpdateUI();

      WINDOW* selWin = inv_wins[loc_y][loc_x]; //create a holder for the currently selected window
    
      mvwchgat (selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
      wrefresh (selWin);
     
      werase (input_win);
      wprint_at (input_win, "Select an item with WASD.. 'put' to transfer to the container.", 0,0);
    
      wprint_at (input_win,"ARQ:~$ ",2,0);
    
      wgetstr (input_win, slc_char);
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
	  werase (input_win);
	  wprint_at (input_win, "Not a correct selection, try again.", 0, 0);
	  wgetch (input_win);
	};
     
      cout << "\n " << loc_x << " " << loc_y;
    };

  return (NULL);
};

void Player :: AddToInventory(item* i)
{
  this->inventory->AddItem(i);
}

void Player :: SetInventoryTile(int x, int y, item* i)
{
  this->inventory->ReplaceItem(x,y,i); 
}

item* Player :: GetFromInventory(int x, int y)
{
  return this->inventory->GetItem(x,y);
}

area* Player :: GetInventory()
{
  return this->inventory;
}  

int Player :: ItemProc (WINDOW* winchoice, item* itm, int x, int y)
{
  string answer;
  char answerchar[20];
  string itm_name = itm->name; 
 
  werase (winchoice);
  wmove (winchoice,0,0);
  wprintw (winchoice,"There's a %s on the floor..", itm_name.c_str());
  wrefresh (winchoice);
 
  wgetch (winchoice); 
 
  wmove (winchoice,0,0);
  wprintw (winchoice,"Would you like to pick the %s? ",itm_name.c_str());
  wgetstr (winchoice, answerchar);
 
  answer = (answerchar);
      
  if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
    {
      if (this->inventory->AddItem (itm) == (1)) //If addToInventory unsuccessful
	{
	  werase (winchoice);
	  wprint_at (winchoice,(const char *)"Inventory full..",0,0);
	  wgetch (winchoice);
	  return (1);
	}
    
      else
	{
	  wmove (winchoice,0,0);
	  werase (winchoice);
	  wprintw (winchoice,"You pick up the %s..",itm_name.c_str());
	  wgetch (winchoice);
      
	  this->inventory->ReplaceItem(x,y,new item(item_library[no_item]));
 
	  SetPos(x,y);

	  return (0);
	};
    }
   
  else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
    {
      wmove (winchoice,0,0);
      werase (winchoice);
      wprintw (winchoice,"You leave the %s untouched..",itm_name.c_str());
      wgetch (winchoice);
     
      SetPos(x,y); 
      return (0);
    }
    
  else 
    {
      wprint_at (winchoice, "Incorrect choice, please answer yes or no.. ",0,0);
      wgetch (winchoice);
      ItemProc (winchoice,itm,x,y);
    };
    
  return (0);
};

//int Player :: CONTAINER PROC

void Character :: SetWeps (weaponIndex one, weaponIndex two, weaponIndex three)
{  
  this->weps[0] = weapon_library[one];
  this->weps[1] = weapon_library[two];
  this->weps[2] = weapon_library[three];
};
  
weapon* Character :: GetWeps()
{
  return this->weps;
}
 
void Character :: SetOutfit(outfit o)
{
  //remove the current AP buff
  this->max_health -= currentOutfit.armourPoints;
  cout << max_health;
  
  //remove any extra health gained from the armour if it exceeds the buff
  if (GetHealth()>max_health)
    {
      SetHealth(health-currentOutfit.armourPoints);
      cout <<health << "-" << currentOutfit.armourPoints; 
    }
   
  this->currentOutfit = o;
  
  //increase max health to suit the outfit
  this->max_health += o.armourPoints;
 
  //add the AP to the current health
  SetHealth(health+o.armourPoints);
  return;
}
 
outfit Character :: GetOutfit()
{
  return currentOutfit;
}
 


int Player :: LockProc (WINDOW * winchoice, int door_y, int door_x, tile doortype, int doortile, string doorname )
{
  string answer;
  item* inv_tile;
  char answerchar[20];
  werase (winchoice);
  wprint_at (winchoice,"How would you like to unlock the door? ",0,0);
  wprint_at (winchoice,"1. Using Door Keys, 2. Using Lockpicks ",1,0);
  wprint_at (winchoice,"Enter a choice to continue, or 'exit' to cancel ",2,0);
  wprint_at (winchoice,"lock_proc:~$  ",3,0);
  wgetstr (winchoice, answerchar);
  answer = (answerchar);
      
  if ((answer == "1") || (answer == "Key") || (answer == "Keys") || (answer == "key") || (answer == "keys") || (answer == "Door Key") || (answer == "Door Keys") || (answer == "door key") || (answer == "door keys"))
    {
      int key_count;
      int count;
      count = 0;
      key_count = 0;
      
      for (int y = 0; y < inv_y; y++)
	{
	  for(int x = 0; x < inv_x; x++)
	    {
	      inv_tile = inventory->GetItem(x,y);
          
	      if (inv_tile->id == key)
		{
		  key_count = (key_count+1);
		};
          
	    };
	}; 
     
      if (key_count == (tile_library[doortile].locks))
	{
            
	  for (int y = 0; y < inv_y; y++)
	    {
	      for(int x = 0; x < inv_x; x++)
		{
		  inv_tile = inventory->GetItem(x,y);
           
		  if (inv_tile->id == (key))
		    {
		      inventory->ReplaceItem(x,y,new item(item_library[no_item]));
		      count = (count+1);
		    };
                 
		  if (count == (tile_library[doortile].locks))
		    {
		      werase (winchoice);
		      wmove (winchoice,0,0);
		      wprintw (winchoice,(const char *)"You insert %d keys into the door and open it .. ",tile_library[doortile].locks);
		      wgetch (winchoice);
       
		      if (doortype == ld1) 
			{
			  SetTile(door_x,door_y,od1);
               
			  if (GetTile(door_x,door_y+1) == (ld1)) //Checking for surrounding door tiles
			    {
			      SetTile(door_x,door_y+1,od0);
			    }
          
			  else if (GetTile(door_x,door_y-1) == (ld1)) 
			    {
			      SetTile(door_x,door_y-1,od0);
			    }
          
			  else if (GetTile(door_x+1,door_y) == (ld1)) 
			    {
			      SetTile(door_x+1,door_y,od0);
			    }
            
			  else if (GetTile(door_x-1,door_y) == (ld1)) 
			    {
			      SetTile(door_x-1,door_y,od0);
			    };
			}
                  
		      else if (doortype == ld2)
			{
			  SetTile(door_x,door_y,od2);

			  if (GetTile(door_x,door_y+1) == (ld2)) //Checking for surrounding door tiles
			    {
			      SetTile(door_x,door_y+1,od0);
			    }
          
			  else if (GetTile(door_x,door_y-1) == (ld2)) 
			    {
			      SetTile(door_x,door_y-1,od0);
			    }
          
			  else if (GetTile(door_x+1,door_y) == (ld2)) 
			    {
			      SetTile(door_x+1,door_y,od0);
			    }
            
			  else if (GetTile(door_x-1,door_y) == (ld2)) 
			    {
			      SetTile(door_x-1,door_y,od0);
			    };
			};
             
		      this->SetPos(door_x,door_y);               
		      return (0);
		    };
            
		};
	    }; 
             
	};
           
      if (key_count != (tile_library[doortile].locks))
	{
	  werase (winchoice);
	  wmove (winchoice,0,0);
	  wprintw (winchoice,"You need %d keys to open this door.. ",tile_library[doortile].locks);
	  wgetch (winchoice);
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
      
      for (int y = 0; y < inv_y; y++)
	{
	  for(int x = 0; x < inv_x; x++)
	    {
	      inv_tile = inventory->GetItem(x,y);
          
	      if (inv_tile->name == item_library[lockpick].name)
		{
		  (lockpick_count = (lockpick_count+1));
		};
          
	    };
	};
       
      if (lockpick_count >= (tile_library[doortile].locks))
	{
         
	  for (int y = 0; y < inv_y; y++)
	    {
	      for(int x = 0; x < inv_x; x++)
		{
		  inv_tile = inventory->GetItem(x,y);
            
		  if (inv_tile->name == item_library[lockpick].name)
		    {
		      int chance = rand() %100+1;
		      int lockno;
              
		      lockno = 1;
              
		      werase (winchoice);
              
		      wmove (winchoice,0,0);
		      wprintw (winchoice,"This door has %d lock(s).. ",tile_library[doortile].locks);
              
		      wmove (winchoice,1,0);
		      wprintw (winchoice,"You attempt to pick lock %d.. ", lockno);
              
		      wgetch (winchoice);
              
		      if (chance > 50)
			{
			  werase (winchoice);
			  wprint_at (winchoice,"You manage to open the lock with the lockpick.. ",0,0);
			  wgetch (winchoice);
                 
			  inventory->ReplaceItem(x,y,new item (item_library[no_item]));
			  count = (count+1);
			  chance = rand() %100+1;
			  lockno = (lockno+1);
			}
              
		      else if (chance <= 50)
			{
			  werase (winchoice);
			  wprint_at (winchoice,"Your lockpick breaks as you attempt to open the lock.. ",0,0);
			  wgetch (winchoice);
                    
			  inventory->ReplaceItem(x,y,new item(item_library[no_item]) );
			  chance = rand() %100+1;
			};
                  
		    };
                 
		  if (count == (tile_library[doortile].locks))
		    {
		      werase (winchoice);
		      wprint_at (winchoice, "You manage to unlock the door.. ",0,0);
		      wgetch (winchoice);
                  
		      if (doortype == ld1) 
			{
			  SetTile(door_x,door_y,od2);
			}
                  
		      else if (doortype == ld2)
			{
			  SetTile(door_x,door_y,od2);
			};
                                    
		      this->SetPos(door_x,door_y);  
		      return (0);
		    };
                 
		};
	    }; 
             
	} 
     
      else if (lockpick_count != (tile_library[doortile].locks))
	{
	  werase (winchoice);
	  wmove (winchoice,0,0);
	  wprintw (winchoice,(const char *)"You need %d lock picks to attempt to open this door.. ",tile_library[doortile].locks);
	  wgetch (winchoice);
	  return (0);
	};
     
      if (count != (tile_library[doortile].locks))
	{
	  werase (winchoice);
	  wprint_at (winchoice, "You have run out of lockpicks..",0,0);
	  wgetch (winchoice);
	  return (0);
	};
     
      return (0);
    }
            
  else if ((answer == "Exit") || (answer == "EXIT") || (answer == "exit"))
    {
      werase (winchoice);
      wmove (winchoice,0,0);
      wprintw (winchoice,(const char *)"You leave the %s untouched. ",doorname.c_str());
      wgetch (winchoice);
      return (0);
    }
	    
  else
    {
      wprint_at (winchoice,(const char *)"Not a correct choice, try again.. ",0,0);
      this->LockProc (winchoice,door_y,door_x,doortype,doortile,doorname);
    };
    
  return (0);
};

void Player :: TileProc(WINDOW* winchoice,int y, int x,tile t)
{
  if (t == od0 || (t == od1) || (t == od2)) 
    {
      this->DoorProc (winchoice, y, x, t);
      return;
    }
  
  else if (t == cd0 || (t == cd1) || (t == cd2) || (t == ld1) || (t == ld2)) 
    {
      this->DoorProc (winchoice, y, x, t);
      return;
    }
  
  else if (t == ent) 
    {
      wprint_at (winchoice,(const char *)"The way you came in is locked..",0,0);
      wgetch (winchoice);
   
      return;
    }
 
  else if (t == ext) 
    {
      wprint_at (winchoice,(const char *)"You have reached the exit!",0,0);
      wgetch (winchoice);
   
      return;
    }
 
  else if (t == ded) 
    {
      wprint_at (winchoice,(const char *)"The floor caves in below one of your feet, injuring you..",0,0);
      wgetch (winchoice);
   
      SetHealth(this->GetHealth()-20);
  
      return;
    };
}
 
int Player :: BattleTurn (WINDOW* main_win, WINDOW* input_win, int npc_id)
{
  werase(input_win);
  
  for (int i=0; i<3; i++)
    {
      string s = weps[i].name;
      wprint_at(input_win,s.c_str(),0,9*i);
    }
   
  wprint_at(input_win,"Flee",0,9*3);
   
  ////input
  int index = 0;
  int scr_x = 9;
  int scr_y = 0;
  
  bool selection = true;
  
  while (selection == true) 
    {
      scr_x = index*9; //calculate the start of the item name
   
      mvwchgat (input_win, scr_y, 0, 27, A_NORMAL, 0, NULL); //remove fancy effects
      mvwchgat (input_win, scr_y, scr_x, 9, A_BLINK, 1, NULL); //add red blink to the current item
      wrefresh (input_win);
    
      wprint_at (input_win,"Use A and D to choose a weapon, 'use' to select",1,0);
      wprint_at (input_win,"ARQ:~$ ",2,0); //give us a prompt
    
      string choice; 
      char input[20];

      wgetstr (input_win, input); //store our grabbed chars
      choice = input; //store the chars as a string for comparison
     
      if ((choice == "a") && (index>0))
	{
	  index--;
	}
   
      else if ((choice=="d") && (index<3))
	{
	  index++;
	}
     
      else if (choice=="use")
	{
	  werase(input_win);
	  wprintw(input_win,"Weapon selected.");
	  return index;
	}
   
    } 
   
  return 0;
}

int Player :: DoorProc (WINDOW * winchoice, int y, int x, tile doortype)
{
  int map_tile = GetTile(x,y);
 
  string door_name = tile_library[map_tile].name;
 
  if ((doortype == od0) || (doortype == od1) || (doortype == od2))
    {
      wmove (winchoice,0,0);
      wprintw (winchoice,"You enter the doorway of the %s.. ",door_name.c_str());
      wgetch (winchoice);
      this->SetPos(x,y);  
   
      return (0);
    }
     
  else if ((doortype == cd0) || (doortype == cd1) || (doortype == cd2))
    {
      string answer;
      char answerchar[20];
      wmove (winchoice,0,0);
      wprintw (winchoice,"Would you like to open the %s? ",door_name.c_str());
      wgetstr (winchoice, answerchar);
      answer = (answerchar);
      
      if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
        {
	  wmove (winchoice,0,0);
	  wprintw (winchoice,"You open the %s and step into the doorway. ",door_name.c_str());
	  wgetch (winchoice);
         
	  if (GetTile(x,y) == (cd0)) //Checking main tile
	    {
	      SetTile(x,y,od0);
           
	      if (GetTile(x,y+1) == (cd0)) //Checking for surrounding door tiles
		{
		  SetTile(x,y+1,od0);
		}
          
	      else if (GetTile(x,y-1) == (cd0)) 
		{
		  SetTile(x,y-1,od0);
		}
          
	      else if (GetTile(x+1,y) == (cd0)) 
		{
		  SetTile(x+1,y,od0);
		}
            
	      else if (GetTile(x-1,y) == (cd0)) 
		{
		  SetTile(x-1,y,od0);
		};
	    };
         
	  if (GetTile(x,y) == (cd1)) //Checking main tile
	    {
	      SetTile(x,y,od0);
           
	      if (GetTile(x,y+1) == (cd1)) //Checking for surrounding door tiles
		{
		  SetTile(x,y+1,od0);
		}
          
	      else if (GetTile(x,y-1) == (cd1)) 
		{
		  SetTile(x,y-1,od0);
		}
          
	      else if (GetTile(x+1,y) == (cd1)) 
		{
		  SetTile(x+1,y,od0);
		}
            
	      else if (GetTile(x-1,y) == (cd1)) 
		{
		  SetTile(x-1,y,od0);
		};
	    };
         
	  if (GetTile(x,y) == (cd2)) //Checking main tile
	    {
	      SetTile(x,y,od0);
           
	      if (GetTile(x,y+1) == (cd2)) //Checking for surrounding door tiles
		{
		  SetTile(x,y+1,od0);
		}
          
	      else if (GetTile(x,y-1) == (cd2)) 
		{
		  SetTile(x,y-1,od0);
		}
          
	      else if (GetTile(x+1,y) == (cd2)) 
		{
		  SetTile(x+1,y,od0);
		}
            
	      else if (GetTile(x-1,y) == (cd2)) 
		{
		  SetTile(x-1,y,od0);
		};
	    };
          
	  return (0);
        }
	    
      else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
	{
	  werase (winchoice);
         
	  wmove (winchoice,0,0);
	  wprintw (winchoice,"You leave the %s untouched. ",door_name.c_str());
         
	  wgetch (winchoice);
	  return (0);
	}
	   
      else
        {
	  werase (winchoice);
         
	  wprint_at (winchoice,"Not a yes or no answer, try again..",0,0);
	  wgetch(winchoice);
         
	  DoorProc (winchoice, y, x, GetTile(x,y));
        };
        
      return (0);
    }
      
  else if ((doortype == ld1) || (doortype == ld2))
    {
      string answer;
      char answerchar[20];
      
      werase (winchoice);
      
      wprint_at (winchoice,"Would you like to open the door? ",0,0);
      wgetstr (winchoice, answerchar);
      
      answer = (answerchar);
      
      if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y"))
        {
	  LockProc (winchoice, y,x,doortype,map_tile,door_name);
	  return (0);
        }
   
            
      else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N"))
        {
	  wmove (winchoice,0,0);
         
	  wprintw (winchoice,(const char *)"You leave the %s untouched. ",door_name.c_str());
	  wgetch (winchoice);
         
	  return (0);
        }
	    
      else
        {
	  wprint_at (winchoice,(const char *)"Not a yes or no answer, try again..",0,0);
         
	  DoorProc (winchoice, y, x, GetTile(x,y));
        };
      return (0);
    };

  return (0);
};

int Player :: Move()
{
  return 1;
}

int Player :: Move(WINDOW * winchoice, WINDOW* mainwin, int y, int x)
{
  string move_tilename = tile_library [GetTile(x,y)].name;
  int pmove_x = (x);
  int pmove_y = (y);
 
  if ((x < 0) || (x > grid_x))
    {
      return (0);
    }
  
  else if ((y < 0) || (y > grid_y))
    {
      return (0);
    };
 

  //This checks to see if the player has walked into any enemies/characters
  for (int e_id = 0; e_id < max_npcs; e_id++)
    {
      //npc check
      if (npcs[e_id] != NULL)
	{
	  int enemy_x;
	  int enemy_y;
	  npcs[e_id]->GetPos (&enemy_x, &enemy_y); //Sets enemy x and y to those of npc[No] through pointers
  
	  int x;
	  int y;
	  this->GetPos (&x, &y); 
   
	  if (((pmove_x == enemy_x and pmove_y == enemy_y) || (x == enemy_x and y == enemy_y)) && (npcs[e_id]->IsAlive()))
	    {
	      string enemy_name = npcs[e_id]->GetName();
         
	      wmove (winchoice,0,0);
	      wprintw (winchoice,"You are confronted by a %s! ",enemy_name.c_str());
	      wgetch (winchoice);
		        
	      Battle (mainwin, winchoice, e_id, this, npcs);
     
	      return (0);
	    }
	  
	  //dead body check
	  else if (pmove_x == enemy_x and pmove_y == enemy_y)
	    {
	      string enemy_name = npcs[e_id]->GetName();

	      wmove (winchoice,0,0);
	      wprintw (winchoice,"There is the corpse of a %s here..",enemy_name.c_str());   
	      
	      wgetch (winchoice);
	      return 2;
	    };
	}
    };
 
  //container check
  switch (AreaProc(winchoice,x,y))
    {
      //returns 1 to indicate item
    case (1) :
      {
	return 1; 
      }
    
    //returns 2 to indicate area
    case (2) :
      {
	return 2;
      }
    
    default :
      {
	break;
      }
    }

  //main movement check
  if ( IsTraversable(GetTile(pmove_x,pmove_y)) )
    {
      wmove (winchoice,0,0);
      wprintw (winchoice,"You walk along the %s.. ",move_tilename.c_str());
   
      wgetch (winchoice);
   
      SetPos(pmove_x,pmove_y);  
      return(0);
    }     
 
  else
    {
      wmove (winchoice,0,0);
      wprintw (winchoice,"There's a %s here.",move_tilename.c_str());
   
      wgetch (winchoice);
   
      TileProc(winchoice,pmove_y,pmove_x,GetTile(pmove_x,pmove_y));
      return(0);
    }
 
  return (0);
};

void Player :: SetLoot(int i)
{
  this->loot = i;
  return;
}

int Player :: GetLootScore ()
{
  return this->loot;
};

int Player :: Input (WINDOW* winchoice, WINDOW* inv_wins[3][3], WINDOW* main_win)
{
  int x;
  int y;
  int thisX;
  int thisY;
   
  this->GetPos(&x,&y); /*Get current position for movement*/
 
  thisX = x; //set movement positions to default
  thisY = y;

  string answer;
  char answerchar[20];

  werase (winchoice);

  wmove (winchoice,0,0); //moves the cursor to the window choice inputted into the function, +1 either side to avoid borders
  wprintw (winchoice,"ARQ:~$ "); 
  wgetstr (winchoice, answerchar);
 
  answer = answerchar; 
     
  if (answer == "pos") 
    {
      cout << "player y :" << x;
      cout << "\nplayer x :" << y;
      return (0);
    }
  
  else if (answer == "help") 
    {
      wprint_at (winchoice,(const char *)"phelp - player help",0,0);
      wprint_at (winchoice,(const char *)"ihelp - interactions",1,0);
      wprint_at (winchoice,(const char *)"info - game info",2,0);
      wgetch (winchoice);
   
      return (0);
    }
 
  else if (answer == "phelp") 
    {
      wprint_at (winchoice,(const char *)"north - move north",0,0);
      wprint_at (winchoice,(const char *)"east- move east",1,0);
      wprint_at (winchoice,(const char *)"south - move south",2,0);
      wprint_at (winchoice,(const char *)"west - move west",3,0);
      wgetch (winchoice);
   
      return (0);
    }
  
  else if (answer == "ihelp") 
    {
      wprint_at (winchoice,(const char *)"drop - drop item (selection)",0,0);
      wgetch (winchoice);
   
      return (0);
    }
  
  else if (answer == "info") 
    {
      Info();
   
      return (0);
    }
  
  else if ((answer == "north") || (answer == "North") || (answer == "NORTH") || (answer == "n") || (answer == "N")) 
    {
      thisY--;
    }
  
  else if ((answer == "east") || (answer == "East") || (answer == "EAST") || (answer == "e") || (answer == "E"))
    {
      thisX++;
    }
  
  else if ((answer == "south") || (answer == "South") || (answer == "SOUTH") || (answer == "s") || (answer == "S")) 
    {
      thisY++;
    } 

  else if ((answer == "west") || (answer == "West") || (answer == "WEST") || (answer == "w") || (answer == "W")) 
    {
      thisX--;
    } 

  else if ((answer == "exit") || (answer == "Exit") || (answer == "EXIT") || (answer == "quit") || (answer == "Quit") || (answer == "QUIT"))
    {
      wprint_at (winchoice,(const char *)"Quitting.. ",0,0);
      wgetch (winchoice);
   
      SetRunning(false);
   
      return(0);
    }
  
  else if (answer == "inventory")
    {
      AccessInventory (winchoice, inv_wins);
      return(0);
    }
 
  
  else
    { 
      wprint_at (winchoice,(const char *)"unrecognised input, please input a command, or use 'help' for a list. ",0,0);
      wgetch (winchoice);
  
      Input (winchoice, inv_wins, main_win);
  
      return (0);
    }
 
  //handle any movement input
  if ((x != thisX) || (y != thisY))
    {
      switch (this->Move(winchoice,main_win,thisY,thisX))
	{
	case 2 :
	  {
	    area* thisArea = GetArea(thisX,thisY);
	    wprintw (winchoice, "Accessing %s at %d, %d", thisArea->name.c_str(),thisX,thisY);
	    wgetch (winchoice);

	    AccessArea(winchoice,inv_wins,thisArea);
	    break;
	  }
    
	case 1 :
	  {
	    //grab the item from the currunt area
	    item* i = GetItem(thisX,thisY);

	    ItemProc(winchoice,i,thisX,thisY);
	    break;
	  }
	 
	default :
	  {
	    break;
	  }
	}
    }

  return (0);
};

int Player :: LootCount ()
{
  int total = 0;

  for (int y = 0; y < inv_y; y++)
    {
      for(int x = 0; x < inv_x; x++)
	{
	  int value = inventory->value;
	  total+=value;
	};
    };

  this->SetLoot(total);
  return (0);
};

int Player :: AreaProc (WINDOW* winchoice, int x ,int y)
{
  area* a = GetArea(x,y);
  return a->HasItems();
}
 
/////////////////////////////////////////////////

//returns an int to represent a weapon choice from weps[] based on health
int NPC :: BattleTurn ()
{
  if (health>50)
    {
      return 1;
    }
   
  else if (health<50 && health>25)
    {
      return 2;
    }
   
  else
    {
      return 0;
    }
};
 
//randomly moves within the 6 local directions
int NPC :: Move ()
{
  int x = ((rand() % 3 - 1));
  int y = ((rand() % 3 - 1));
  
  int pos_x;
  int pos_y;
    
  this->GetPos(&pos_x,&pos_y);  

  int move_x = (pos_x+x);
  int move_y = (pos_y+y);
 
  if ((move_x == pos_x) and (move_y == pos_y))
    {
      return 1;
    }
  
  if ((move_x != pos_x) and (move_y != pos_y))
    {
      return 1;
    }
  
  for (int No = 0; No < max_npcs; No++)
    {
      for (int NPCNo = 0; NPCNo < max_npcs; NPCNo++)
	{
	  if ((npcs[No] != NULL) && (npcs[NPCNo] != NULL))
	    {
	      int player_x;
	      int player_y;
	      int character_x;
	      int character_y;
    
	      npcs[No]->GetPos (&player_x, &player_y);
	      npcs[NPCNo]->GetPos (&character_x, &character_y);
     
      
	      if (move_x == player_x and move_y == player_y)
		{
		  //Battle 
		};
     
	      if (move_x == character_x and move_y == character_y)
		{
		  return 0;
		};
	    }; 
	};
    };
  
  tile t = GetTile(move_x,move_y); //Grab the tile in advance to save repeated Get()'s
  
  if ((t != ntl) and (t != wa1) and (t != wa2) and (t != wa3) and (t != ent) and (t != ext))
    {
      this->SetPos (move_x,move_y);
      return 0;  
    }
    
  else if (t == win) 
    {
      return 1;
    }
  
  else if (t == cd0 || (t == cd1) || (t == cd2) || (t == ld1) || (t == ld2)) 
    {
      return 2;
    }
   
  else if (t == ded) 
    {
      return 3;
    };
  
  return 1;
};

void InitNpcs()
{
  for(int a = 0; a < max_npcs; a++) 
    { 
      npcs[a] = NULL;
      npcs[a] = new HornyGoblin(); //Fills the array with the information of the specific class
      npcs[a]->SetPos (2,1); //Sets the position of the templated NPC.
    };
   
  cout << "Characters generated\n"; 
  return;
}

void DrawNPCS()
{
  for(int a = 0; a < max_npcs; a++)
    {      
      if ((npcs[a] != NULL) && (npcs[a]->IsAlive()))
	{
	  npcs[a]->Move();
	  npcs[a]->Draw ();
	}
     
      else if (npcs[a] != NULL)
	{
	  npcs[a]->Draw ();
	};
    };
}

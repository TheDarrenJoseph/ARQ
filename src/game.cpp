#include <iostream>
#include "game.h"

void GameEngine :: InitNPCS()
{
    
    for(int a = 0; a < MAX_NPCS; a++) 
    { 
      npcs[a] = Goblin(); //Fills the array with the information of the specific class
      npcs[a].SetPos (3,1); //Sets the position of the templated NPC.
    };
}

int GameEngine :: MainMenu() {
    
    
    const int menuSize = 3;
    const char* text [menuSize] = {"Play! -> ","Settings ->","Info ->"};
    
    return displayUI->Menu(&text[0],menuSize);
    return 0;
}

void GameEngine :: StartGame()
{
  InitNPCS(); //inititalise all NPCs before doing anything
    
  map.InitAreas();
 // GenerateItems(mediumLoot); 
 
  displayUI->InitScreen (); //prep display
  displayUI->InitWindows(); 
  displayUI->DecorateWindows(); //Box/label the UI
  
  switch (MainMenu()) {
  case 0 :
     
      displayUI->ShowNotification("Welcome To ARQ! \n "
      "You are a humble warrior hoping to earn prestige "
      "and bring fame and fortune to your family! "
      "You have set forth into the dungeon in order"
      " to gather as many riches as possible before "
      "returning to your loved ones. You are on the"
      " first level of the dungeon, and should encounter"
      " little resistance to your pillaging here. "
      "The deeper you go, the more valuable the prizes that await you, "
      "but beware what lurks in the deeper levels! Good Luck!");
      
      while (GameLoop()); //Main game loop
          break;
      
  default :
      break;
  }
  
  
  
  //displayUI->DestroyWindows();  
  endwin ();
}


bool GameEngine :: GameLoop()
{
  //Draw main elements
  int playerX;
  int playerY;  
  
  player->GetPos(&playerX, &playerY);
  
  displayUI->DrawMap (&map,false,playerX,playerY,3); 
  displayUI->DrawItems (&map); 
  displayUI->DrawContainers(&map);
  playerUI->DrawNPCS();
  
  //Draw player UI elements
   playerUI->DrawPlayer(); //Draw the player
   
  displayUI->UpdateUI(); //refresh all windows
  displayUI->DrawPlayerStats (player->GetName(),player->GetHealth(),player->GetLootScore());   
  
  return playerUI->Input(); //take input from the player
}

 Player* GameEngine :: GetPlayer()
 {
     return player;
 }
 
 /* Sets the passed pointer to the maximum NPC number, and returns a pointer to the first index of the npcs array*/
 NPC* GameEngine :: GetNPCS(const int* size)
 {
     size = &MAX_NPCS;
     return &this->npcs[0];
 }
 
 Map* GameEngine :: GetMap()
 {
     return &map;
 }
 
void GameEngine :: GenerateItems(lootChance thisChance)
{
  for (int y = 0; y < map.GetGridY(); y++)
    {
      for (int x = 0; x < map.GetGridX(); x++)
	{
	  //add items to room tiles
	  if (map.GetTile(x,y) == rom)
	    {
	      //for each of the items in the library
	      for (int  i=0; i<item_size; i++)
		{
		  //generate a random chance out of 100 for each item
		  int chance = rand() %100+1;
	      
		  //convert the current index in the library to an item enum 
		  Item thisItem = item_library[i];
	      
		  //if the random chance < the number of items (e.g 10, 10%)
		  if (chance<=thisChance)  
		    {
		      //then add it to the item grid
		      map.AddToContainer(x,y,new Item(thisItem));
	        
		      //break the for loop
		      break; 
		    }
		}
	    
	      /*
	      //spawn each weapon based on a chance out of 100
	      for (int  i=0; i<weapon_size; i++)
	      {
	      weapon thisWeapon = weapon_library[i];
	      
	      int chance = rand() %100+1;
	      
	      //use the number of weapons as the chance of appearing
	      
	      if (chance<=weapon_size && IsLootable(thisWeapon)) 
	      {
	      item_grid[y][x] = new weapon(thisWeapon);
	      cout << thisWeapon.name;
	      break;
	      };
	      }
	      */
	  
	    }
																	
	  else
	    {
	      map.AddToContainer(x,y,new Item(item_library[no_item]));
	    }
	}
    }
   
  std::cout << "Items generated\n"; 
  return;
}
 


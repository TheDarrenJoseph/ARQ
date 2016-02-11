#include "position.h"
#include "game.h"
#include <vector>


void GameEngine :: InitNPCS()
{
    
    for(int a = 0; a < MAX_NPCS; a++) 
    { 
      npcs[a] = Goblin(); //Fills the array with the information of the specific class
    };
}

void GameEngine :: SettingsMenu() {
    //const int menuSize = 3; 
    //1const char* text [menuSize] 
     while(true) {
         
    std::string fogOfWar = std::string(map->GetFogOfWar() ? "ON" : "OFF");
    std::vector<std::pair<std::string, std::string>> text = { {"Fog of war",fogOfWar},
                                           {"Blank",">:D"},
                                           {"Close Settings",""}
                                         };   
   
    switch(displayUI->Menu(text)) {
        case 0 :
            map->ToggleFogOfWar();
            break;
            
        default :
            return;
            break;
        
    }
    }

}

int GameEngine :: MainMenu(bool gameRunning) {
    std::string contextText = std::string(gameRunning ? "Resume" : "Play" );
    
    std::vector<std::pair<std::string, std::string>> text = { {contextText,"->"},
                                           {"Settings","->"},
                                           {"Info","->"},
                                           {"Quit","->"}
                                         };   

    
    return displayUI->Menu(text);
}

/** Sets the entry and exit points for this level, also places the player at the entry position  */
void GameEngine :: spawnPlacePlayer() {
    //int roomCount = map.GetRoomCount();
    Room* rooms = map->GetRooms();
    int roomCount = map->GetRoomCount();
    
    std::vector<Position> possibleSpawns;
    
    //int randomRoom = rooms[rand()%roomCount];
    
    //Save the co-ordinates of every room's tiles
    for (int roomNo=0; roomNo<roomCount; roomNo++) {
        Room room = rooms[roomNo];
        Position roomPos = room.GetStartPos();
        
        for (unsigned int x=roomPos.x; x<(room.GetEndPos().x); x++) {
            for (unsigned int y=roomPos.y; y<(room.GetEndPos().y); y++) {
                if (map->GetTile(x,y) == rom) {
                    possibleSpawns.push_back(Position(x,y));
                } 
            }
            
        }
 
    }

    if( possibleSpawns.size() > 0 ) {
      Position chosenEntry = possibleSpawns[rand()%possibleSpawns.size()]; //Pick a random position
      player->SetPos(chosenEntry.x, chosenEntry.y); //place the player there
      map->SetTile(chosenEntry.x,chosenEntry.y,ent);
      
      Position chosenExit = chosenEntry;
      while (chosenExit == chosenEntry) {
          chosenExit = possibleSpawns[rand()%possibleSpawns.size()]; //pick another for our exit
      }
      
      map->SetTile(chosenExit.x,chosenExit.y,ext);
      map->SetEntryPositions(chosenEntry,chosenExit);
    }
}

//The main menu screen for game-start
int GameEngine :: MenuScreen(bool gameRunning) {
    while (true) {
        switch (MainMenu(gameRunning)) {
            case 0 :
                if(!gameRunning) {
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
                } 
            
                return 0;
                break;
                
            case 1 :
                SettingsMenu();
                break;
                
            case 2 :
                displayUI->ShowNotification("Game made by RaveKutsuu/Darren Joseph using C++ and ncurses as a learner project.");
                break;
            
            case 3 : 
                return 1; //Anything that isn't 0 quits
        }
    }
}

void GameEngine :: StartGame()
{
  InitNPCS(); //inititalise all NPCs before doing anything
   
  spawnPlacePlayer();
 // GenerateItems(mediumLoot); 
 
  displayUI->InitScreen (); //prep display
  displayUI->InitWindows(); 
  displayUI->DecorateWindows(); //Box/label the UI
  
  //Start the main menu
  if (MenuScreen(false) == 0) {
      npcs[0].Kill();
      bool levelEnded=false;
      bool newLevel=true;
      
      while (GameLoop(&levelEnded,&newLevel) && !levelEnded); //Main game loop
  } 
      
      //displayUI->DestroyWindows();  
      endwin ();
      return;
}

bool GameEngine :: GameLoop(bool* levelEnded, bool* downLevel){
  //Draw main elements
  int playerX;
  int playerY;  

  player->GetPos(&playerX, &playerY);
  
  displayUI->DrawMap (map,map->GetFogOfWar(),playerX,playerY,3); 
  displayUI->DrawItems (map); 
  displayUI->DrawContainers(map);
  playerUI->DrawNPCS();
  
  //Draw player UI elements
   playerUI->DrawPlayer(); //Draw the player
   
  displayUI->UpdateUI(); //refresh all windows
  displayUI->DrawPlayerStats (player->GetName(),player->GetHealth(),player->GetLootScore());   
  
  //take input from the player
  if (!(*levelEnded)) {
      if(!playerUI->Input(levelEnded,downLevel)) {
          if (!player->IsAlive())  {
              //deathscreen 
          } 
          
          if (MenuScreen(true) == 1) return false; 
      } 
  } 
  
  if (*levelEnded) {
      Position newPosition = map->GetEntryPosition(); //Set to entrance by default
      
      if ((*downLevel)) {
          if(levels.empty()) {
              levels.push_front(map);
              currentLevel = levels.begin();
          }
          
          //Make a new level (levelIndex starts from 0)
          if ((levelIndex == levels.size()-1)) {
              displayUI->ClearConsole();
              displayUI->ConsolePrint("Generating a new level..",0,0);
              
              //Re-run map init to place player and exit (spawnPlacePlayer and extras?)
              map = new Map(50,15,MAX_NPCS,&npcs[0],player); //Rebuild the map
              levels.push_back(map);
              
              displayUI->ConsolePrintWithWait("DONE [Enter]",0,12);

              delete(playerUI);
              playerUI = new PlayerUI(MAX_NPCS,displayUI,map,player,&npcs[0]);
              //InitNPCS(); 
              spawnPlacePlayer();
          }
          
          displayUI->ShowNotification("You go down a level!");
          currentLevel++; //Go down a level
          levelIndex++;
         
          newPosition = (*currentLevel)->GetEntryPosition();
      } else if (!(*downLevel)) { //UP -- Otherwise check if we've gone back a level
          
          displayUI->ShowNotification("You go up a level!");

          if (levels.size()>0 && levelIndex>0) {
              // std::cout<<"Going up a level..\n";
              currentLevel--;//Go up a level
              levelIndex--;
              
              newPosition = (*currentLevel)->GetExitPosition();
          } else {
              displayUI->ShowNotification("You escaped to the surface!");
              return false; //Left the dungeon, woo!   
          }
      } 
      
      map = *currentLevel;
      
      delete(playerUI);
      playerUI = new PlayerUI(MAX_NPCS,displayUI,map,player,&npcs[0]);
      player->SetPos(newPosition.x,newPosition.y);
      
      *levelEnded=false;
      //  *downLevel=false;
      GameLoop(levelEnded,downLevel);
  }
  
  return true;
}

 Player* GameEngine :: GetPlayer()
 {
     return player;
 }
 
 /* Sets the passed pointer to the maximum NPC number, and returns a pointer to the first index of the npcs array*/
 NPC* GameEngine :: GetNPCS(int* size)
 {
     (*size) = MAX_NPCS;
     return &this->npcs[0];
 }
 
 Map* GameEngine :: GetMap()
 {
     return map;
 }
 
void GameEngine :: GenerateItems(lootChance thisChance)
{
  for (int y = 0; y < map->GetGridY(); y++)
    {
      for (int x = 0; x < map->GetGridX(); x++)
	{
	  //add items to room tiles
	  if (map->GetTile(x,y) == rom)
	    {
	      //for each of the map->items in the library
	      for (int  i=0; i<item_size; i++)
		{
		  //generate a random chance out of 100 for each item
		  int chance = rand() %100+1;
	      
		  Item thisItem = item_library[i];
	      
		  //if the random chance < the number of items (e.g 10, 10%)
		  if (chance<=thisChance)  
		    {
		      //then add it to the item grid
		     // map.AddToContainer(x,y,new Item(thisItem));
                      map->AddToContainer(x,y,&item_library[i]);
                   
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
	      map->AddToContainer(x,y,&item_library[no_item]);
	    }
	}
    }
   
  std::cout << "Items generated\n"; 
  return;
}
 


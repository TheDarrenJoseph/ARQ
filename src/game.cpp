#include "game.h"

void GameEngine :: SetMap(Map* map) {
  this -> map = map;
}

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
         
      std::string fogStatus = std::string(fogOfWar? "ON" : "OFF");
      std::vector<std::pair<std::string, std::string>> text = { 	
                                    {"Fog of war",fogStatus},
                                    {"Close Settings",""}
                                  };   
       
      switch(displayUI->Menu(text)) {
        case 0 :
          if (fogOfWar) {
            displayUI -> DisableFogOfWar(map);
          } else {
            displayUI -> EnableFogOfWar(map, player -> GetPosition());
          } 
          fogOfWar = !fogOfWar;
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

void GameEngine :: SpawnPlacePlayer(spawn_position spawnPosition) {
  Position spawnPos;
  switch (spawnPosition) {
    case ENTRY:
      logging -> logline("Spawning player at entry");
      spawnPos = this -> map -> GetEntryPosition();
      break;
  case EXIT:
      logging -> logline("Spawning player at exit");
      spawnPos = this -> map -> GetExitPosition();
      break;
  } 
  player->SetPos(spawnPos.x, spawnPos.y); //place the player there

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
  GenerateLevel();
  LoadLevel(0);

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

void GameEngine :: InitialiseMap(spawn_position spawnPosition) {
  logging -> logline("Generating NPCs...");
  InitNPCS(); //inititalise all NPCs before doing anything
   
  logging -> logline("Placing the player...");
  SpawnPlacePlayer(spawnPosition);
  //GenerateItems(mediumLoot); 
      
  delete(playerUI);
  playerUI = new PlayerUI(MAX_NPCS,displayUI, this -> map,player,&npcs[0]);
  //player->SetPos(newPosition.x,newPosition.y);
  
  Pathfinding* pathfinding = new Pathfinding(map);
  std::list<Path> roomPaths = pathfinding -> BuildPathsBetweenRooms();
  for (Path path : roomPaths) {
    logging -> logline("Adding a path to the map...");
    map -> AddPath(path);
  }
}

Map* GameEngine :: GenerateLevel() {
  logging -> logline("Generating Map...");
  Map* newMap = new Map(MAX_NPCS, &npcs[0], player);
  levels.push_back(newMap);
  //SetMap(newMap); //Rebuild the map
  logging -> logline("Created map of size: " + std::to_string(newMap -> GetGridX()+1) + ", " + std::to_string(newMap -> GetGridY()+1)); 
  return newMap;
}

void GameEngine :: LoadLevel(int levelIdx) {
  if (levelIndex == (std::numeric_limits< long int>::max()) ) {
      displayUI->ShowNotification("The door is useless! (You have gone too deep!)");
      return; //Return without changing level or resetting the levelEnded flag, ends the main loop
  }

  if (levelIdx >= 0 && (long unsigned int) levelIdx < levels.size()) {
      
      std::list<Map*>::iterator levelIterator = levels.begin();
      for (int i=0; i < levelIdx; i++) {
        levelIterator++;
      }
      
      SetMap(*levelIterator);  
      spawn_position spawnPosition = (levelIdx == levelIndex || levelIdx > levelIndex) ? ENTRY : EXIT;
      InitialiseMap(spawnPosition);
      levelIndex = levelIdx;

  }
}

/** Loads a new level, either creating a new Map, or loading a pre-existing one.
 * 
 * @param levelEnded whether or not an exit has been reached
 * @param downLevel whether or not to go down a level
 */
void GameEngine :: ChangeLevel(bool* levelEnded, bool* downLevel) {
    if (levelIndex == (std::numeric_limits< long int>::max()) ) {
        displayUI->ShowNotification("The door is useless! (You have gone too deep!)");
        return; //Return without changing level or resetting the levelEnded flag, ends the main loop
    }
        
    if ((*downLevel)) {
        displayUI->ShowNotification("You go down a level!");
        displayUI->ClearConsole();
        displayUI->ConsolePrint("Generating a new level..",0,0);
        displayUI->UpdateUI();
        
        // Generate a new level
        GenerateLevel();
        LoadLevel(levelIndex+1);
    } else if (!(*downLevel)) { //UP -- Otherwise check if we've gone back a level
        displayUI->ShowNotification("You go up a level!");
        if (levels.size()>0 && levelIndex>0) {
          LoadLevel(levelIndex-1);
        } else {
            displayUI->ShowNotification("You escaped to the surface!");
            return; //Return without changing level or resetting the levelEnded flag, ends the main loop
        }
    } 
    
    *levelEnded=false;
}

/**
 * 
 * @param levelEnded whether or not the current level has ended, the game ends if this is true when this function returns
 * @param downLevel
 * @return 
 */
bool GameEngine :: GameLoop(bool* levelEnded, bool* downLevel){
  Position playerPos = player -> GetPosition();
  
  displayUI->DrawMap (map, fogOfWar, playerPos); 
  displayUI->DrawItems (map); 
  displayUI->DrawContainers(map);
  playerUI->DrawNPCS();
  
  //Draw player UI elements
   playerUI->DrawPlayer(); //Draw the player
   
  displayUI->UpdateUI(); //refresh all windows
  displayUI->DrawPlayerStats (player->GetName(),player->GetHealth(),player->GetLootScore(),GetLevelIndex());   
  
  //take input from the player
  if (!(*levelEnded)) {
      if(!playerUI->Input(levelEnded,downLevel)) {
          if (!player->IsAlive())  {
              //deathscreen 
              displayUI->ShowNotification("You died!");
              MenuScreen(false);
          } 
          
          if (MenuScreen(true) == 1) return false; 
      } 
  } 
  
  if (*levelEnded) {
      ChangeLevel(levelEnded,downLevel);
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
 
 long int GameEngine :: GetLevelIndex() {
     return levelIndex;
 }
 
void GameEngine :: GenerateItems(lootChance thisChance)
{
  int itemCount = 0;
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
                    
                  Item thisItem = ItemLibrary.item_library[i];
                    
                  //if the random chance < the number of items (e.g 10, 10%)
                  if (chance<=thisChance)  
                    {
                      map -> AddToContainer(x,y,new Item(thisItem));
                      itemCount++;
                      //map->AddToContainer(x,y,&item_library[i]);
                      break; 
                    }
                }
            }
                                        
          else
            {
              map->AddToContainer(x,y,&ItemLibrary.item_library[no_item]);
            }
        }
    }
   
  logging -> logline(itemCount + " items generated\n"); 
  return;
}
 
    

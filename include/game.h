#ifndef GAME_H
#define GAME_H

#define VERSION "0.89 Linux Native"

#include <vector>
#include <limits> //numeric limits for safety

#include "logging.h"
#include "characters.h"
#include "playerUI.h"
#include "ui.h"
#include "position.h"
#include "map.h"
#include "pathfinding.h"

void SetRunning(bool state);
std::string GetVersion();

class GameEngine
{  
    private:
        Logging* logging = &logging -> getInstance();

        int MAX_NPCS=0;

        Map* map=NULL;
        std::list<Map*> levels = {};
        
         long int levelIndex=0;
        
        Player* player=NULL;
        NPC* npcs=NULL;
        UI* displayUI=NULL;
        PlayerUI* playerUI=NULL; 
        std::list<Item*> generatedItems = std::list<Item*>(); //A list of all items generated (memory allocated instead of references)
        
        bool fogOfWar = false;
        
    public:
        void SetMap(Map* map);

        void InitNPCS();
        
        void SettingsMenu();
        int MenuScreen(bool gameRunning);
        int MainMenu(bool gameRunning);
        
        void SpawnPlacePlayer(spawn_position spawnPosition);
        
        void StartGame();
        
        bool GameLoop(bool* levelEnded, bool* newLevel);
        void InitialiseMap(spawn_position spawnPosition);
        Map* GenerateLevel();
        void LoadLevel(int levelIdx);
        void ChangeLevel(bool* levelEnded, bool* downLevel);
                
        Player* GetPlayer();
        NPC* GetNPCS(int* size);
        
        Map* GetMap();
        long int GetLevelIndex(); //Get the current level number
                
        void GenerateItems(lootChance thisChance);
        
        /** Shallow-copies from another game-engine to preserve external data control
         * 
         * 
         * @param engine Another game engine to copy from
         */
        void copyGameEngine(const GameEngine& engine) {
            this->MAX_NPCS = engine.MAX_NPCS;
                        
            this->displayUI = engine.displayUI;
            this->generatedItems = engine.generatedItems;
            this->map = engine.map;
            this->npcs = engine.npcs;
            this->player = engine.player;
            this->playerUI = engine.playerUI;
            
            std::list<Map*> oldLevels = engine.levels;
            
            //Assign a begin an end iterator for our copy
            std::list<Map*>::iterator oldLevelsIt = oldLevels.begin();
            const std::list<Map*>::iterator toCopyEnd = oldLevels.end(); //One past the end 
            
             std::list<Map*>::iterator newLevelsIt = levels.begin();

           // std::list<Map*>::iterator copiedLevels = levels.begin()
            //Assign the currentLevel pointer, which we shall use to advance through all of the toCopy 
            //(++currentLevel for efficiency)
            //!= (since our loop stops when it reaches the end+1 then, and thus we don't miss the last item)
            for (; oldLevelsIt != toCopyEnd; ++oldLevelsIt) {
                *newLevelsIt =*oldLevelsIt; //Initiate Map copy constructor
                
                ++newLevelsIt;
            }
        }
    
    //Overriding assignment operator
    GameEngine& operator=(const GameEngine& engine) {  
        copyGameEngine(engine);
        return *this;
    }
    
    //Copy constructor  
    GameEngine(const GameEngine& engine) {  
        copyGameEngine(engine);
    }
        

    GameEngine(Player* player, NPC* npcs, const int maxNPCS, UI* ui, PlayerUI* playerUI) {
        
        this -> player = player;
        this -> npcs = &npcs[0];
        
        this -> MAX_NPCS = maxNPCS; 
        
        this -> map = new Map(MAX_NPCS, &npcs[0], player);

        srand(time(NULL)); //set time for randomiser
                
        this->displayUI = ui;
        this->playerUI = playerUI;
    }
    
    ~GameEngine() {
        delete(playerUI);
        
       if(levels.size()==0) delete(map); //Current map, not in level list.
        
        for(Map* m : levels) {
            delete(m);
        }

    }
 
  
};

#endif

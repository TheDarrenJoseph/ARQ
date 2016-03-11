#ifndef GAME_H
#define GAME_H

#define VERSION "0.89 Linux Native"


#include "characters.h"
#include "playerUI.h"
#include "ui.h"

void SetRunning(bool state);
std::string GetVersion();

class GameEngine
{  
    private:
        int MAX_NPCS=0;

        Map* map=NULL;
        std::list<Map*> levels;
        
        long unsigned int levelIndex=0;
        std::list<Map*>::iterator currentLevel = levels.begin();
        
        Player* player=NULL;
        NPC* npcs=NULL;
        UI* displayUI=NULL;
        PlayerUI* playerUI=NULL; 
        std::list<Item*> generatedItems = std::list<Item*>(); //A list of all items generated (memory allocated instead of references)
        
    public:
        void InitNPCS();
        
        void SettingsMenu();
        int MenuScreen(bool gameRunning);
        int MainMenu(bool gameRunning);
        
        void spawnPlacePlayer();
        
        
        void StartGame();
        
        bool GameLoop(bool* levelEnded, bool* newLevel);
        void ChangeLevel(bool* levelEnded, bool* downLevel);
                
        Player* GetPlayer();
        NPC* GetNPCS(int* size);
        
        Map* GetMap();
        
        int EncounterCheck(int x, int y, int* npcID);
        
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
        
    
        GameEngine(Player* p, NPC* n, const int maxNPCS, Map* m, UI* ui, PlayerUI* playerUI) {
            
            player = p;
            npcs = &n[0];
            
            MAX_NPCS = maxNPCS; 
            
            srand(time(NULL)); //set time for randomiser
            
            map = (m);
            
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

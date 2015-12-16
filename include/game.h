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
        Map map = Map();
        Player* player=NULL;
        NPC* npcs=NULL;
        UI* displayUI=NULL;
        PlayerUI* playerUI=NULL; 
        std::list<Item*> generatedItems = std::list<Item*>(); //A list of all items generated (memory allocated instead of references)
        
    public:
        void InitNPCS();
        
        int MainMenu();
        
        void StartGame();
        
        bool GameLoop();
        
        Player* GetPlayer();
        NPC* GetNPCS(const int* size);
        
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
            
            map = (*m);
            
            this->displayUI = ui;
            this->playerUI = playerUI;
        }
        
        ~GameEngine() {
            
        }
     
  
};

#endif

#ifndef GAME_H
#define GAME_H

#define VERSION "0.89 Linux Native"

#include "characters.h"
#include "ui.h" //includes grid
#include "playerUI.h"

void SetRunning(bool state);
std::string GetVersion();

class GameEngine
{  
    private:
        int MAX_NPCS;
        Map map;
        Player* player;
        NPC* npcs;
        CursesUI* displayUI;
        PlayerUI* playerUI; 
        
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
    
        GameEngine(Player* p, NPC* n, const int maxNPCS, Map* m, CursesUI* ui, PlayerUI* playerUI) {
            
            player = p;
            
            npcs = &n[0];
            
            MAX_NPCS = maxNPCS; 
            
            srand(time(NULL)); //set time for randomiser
            
            map = (*m);
            
            this->displayUI = ui;
            this->playerUI = playerUI;
        }

};

#endif

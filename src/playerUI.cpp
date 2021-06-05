#include "playerUI.h"
#include "stringUtils.h"

void CursesUI::ShowInfo() {
    wprint_at(consolewin_front, (const char *) "Created by Darren Joseph", 0, 0);
    wprint_at(consolewin_front, (const char *) "ARQ Learner project/Tech demo", 2, 0);
    wprint_at(consolewin_front, (const char *) "Made using C++ and ncurses", 3, 0);
    wgetch(consolewin_front);
}

void PlayerUI::Battle(int npc_id)
{
    std::string p_name = player->GetName();
    std::string npc_name = npcs[npc_id].GetName();

    mainUI->ClearConsole();

    //werase (consolewin_front); //Clears both the input and map windows to fit the battle scenario
    //werase (mainwin_front);

    while (true) {
        //if either has died, kill them and return
        if (player->IsAlive() == false) {

            /** virtual void ClearConsole();
      virtual void ConsolePrint(std::string text, int posX, int posY);
      virtual void ConsolePrintWithWait(std::string text, int posX, int posY);
      virtual std::string ConsoleGetString();*/

            player->Kill();

            mainUI->ConsolePrintWithWait("You have been slain..", 0, 0);
            return;
        } else if (npcs[npc_id].IsAlive() == false) {
            npcs[npc_id].Kill();
            mainUI->ConsolePrintWithWait("Your foe has been slain..", 0, 0);
            return;
        } else {
            std::ostringstream playerHealthStream;
            std::ostringstream enemyHealthStream;

            playerHealthStream << p_name.c_str() << " Health: " << player->GetHealth() << "\n";
            enemyHealthStream << npc_name.c_str() << " Health: " << npcs[npc_id].GetHealth() << "\n";
            const std::string playerHealthText = playerHealthStream.str();
            const std::string enemyHealthText = playerHealthStream.str();

            //display the info of both combatants
            mainUI->ClearConsole();
            mainUI->ConsolePrint(playerHealthText, 0, 0);
            mainUI->ConsolePrintWithWait(enemyHealthText, 0, 1);

            //Get both combatants weapons
            const weapon* p_weps = player->GetWeps();
            const weapon* npc_weps = npcs[npc_id].GetWeps();

            //allow both combatants to make a move
            int p_move = BattleTurn(npc_id);
            int npc_move = npcs[npc_id].BattleTurn();

            if ((p_move <= 2) && (npc_move <= 2) && (npcs[npc_id].IsAlive()) && (player->IsAlive())) {
                //select the weapon using their move choice
                weapon p_choice = p_weps[p_move];
                weapon npc_choice = npc_weps[npc_move];

                //show each players choices
                mainUI->ClearConsole();
                std::ostringstream hitTextStream;
                int playerDamage = p_choice.damage;
                int npcDamage = npc_choice.damage;

                //Building battle context text! String streams perhaps not the best here,
                //maybe find a way to handle params like wprintw("%param",var)??

                //Player move
                if (p_move != 0) {
                    npcs[npc_id].SetHealth(npcs[npc_id].GetHealth() - playerDamage);

                    hitTextStream << "You use your " << p_choice.name.c_str() << " and strike the enemy for " << playerDamage << " damage " << "\n";
                    mainUI->ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                } else if (p_move == 0 && npc_move != 0) {
                    player->SetHealth(player->GetHealth() - npcDamage);

                    hitTextStream << "You do nothing, and are struck for " << npcDamage << " damage." << "\n";
                    mainUI->ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                };

                //Enemy move
                if (npc_move != 0 && p_move != 0) {
                    player->SetHealth(player->GetHealth() - npcDamage);

                    hitTextStream << "Your enemy uses their " << npc_choice.name.c_str() << " and deals " << npcDamage << " damage " << "\n";
                    mainUI->ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                } else if (npc_move == 0 && p_move != 0) {
                    npcs[npc_id].SetHealth(npcs[npc_id].GetHealth() - playerDamage);

                    hitTextStream << "Your enemy does nothing, and takes " << playerDamage << " damage " << "\n";
                    mainUI->ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                }
                else {
                    mainUI->ConsolePrintWithWait("You both do nothing.", 0, 0);
                };
            }
            else if (p_move == 3) {
                return; //default exit for now, need to refactor this

                //FLEE CODE HERE
                mainUI->ClearConsole();
                mainUI->ConsolePrintWithWait("You attempt to flee.", 0, 0);

                /**
	  
                if (player->CanFlee(map))
                    if (player->Flee(map) == 0)
                       {
                         wprintw(consolewin_front,"You manage to escape the fight."); 
                         wgetch (consolewin_front);
                         return;
                        }
		 
                else..  */
                mainUI->ClearConsole();
                mainUI->ConsolePrintWithWait("You try to escape but have nowhere to run to.", 0, 0);

            }
        }
    }

}

void PlayerUI::AccessPlayerInv() {
    AccessContainer(player->GetInventory(),true);
    mainUI->UpdateUI();
}

void PlayerUI::DrawPlayerEquipment() {
//    weapon* weps = player->GetWeps();
//
//    for (int x = 0; x < INV_SIZE - 1; x++) {
//        std::string name = weps[x].name;
//        mainUI->DrawPlayerEquipmentSlot(x, name);
//    };
//
//    mainUI->DrawPlayerEquipmentSlot(INV_SIZE, player->GetOutfit().name);
}

int PlayerUI::BattleTurn(int npc_id) {
    mainUI->ClearConsole();

    for (int i = 0; i < 3; i++) {
        const weapon* weps = player->GetWeps();
        std::string wepName = weps[i].name;
        mainUI->ConsolePrint(wepName, 0, (9 * i));
    }

    mainUI->ConsolePrint("Flee", 0, (9 * 3));

    ////input
    int index = 1;

    bool selection = true;

    while (selection == true) {
        int scr_x = index * 9; //calculate the start of the item name
        int scr_y = 0;
        mainUI->HighlightConsole(scr_x, scr_y);

        mainUI->ConsolePrint("Use A and D to choose a weapon, 'use' to select", 1, 0);
        mainUI->ConsolePrint(PROMPT_TEXT, 2, 0); //give us a prompt
        
        std::string choice = mainUI->ConsoleGetString();
        if ((choice == "a") && (index > 0)) {
            index--;
        } else if ((choice == "d") && (index < 3)) {
            index++;
        } else if (choice == "use") {
            mainUI->ClearConsole();
            mainUI->ConsolePrintWithWait("Weapon Selected", 0, 0);
            return index;
        }

    }

    return 0;
}

int PlayerUI::processYesOrNoChoice(std::string choice) {
    if(choice.length() == 0 || choice.length() > 3) return -1;
    std::string lowerCased = StringUtils::toLowerCase(choice);
    if ((lowerCased == "yes") || (lowerCased == "y")) {
      return 0;
    } else if ((lowerCased == "no") || (lowerCased == "n")) {
      return 1;
    } else {
      return -1;
    }
}



int PlayerUI::DoorProc(int y, int x) {
    int map_tile = map -> GetTile(x, y);
    Door door = (*map -> GetDoor(x, y));
    std::string door_name = door.name;

    if (door.isLocked) {
        mainUI->ConsolePrintWithWait("The door here is locked..", 0, 0);
        int unlockChoice = -1;
        while (unlockChoice < 0) {
          mainUI->ClearConsole();
          std::string answer;
          std::string output = "Would you like unlock the " + door_name + "? ";
          mainUI->ConsolePrint(output, 0, 0);
          answer = mainUI->ConsoleGetString();
          int answerChoice = processYesOrNoChoice(answer);
          mainUI->ClearConsole();
          if (answerChoice == 0) {
            LockProc(y, x);
            unlockChoice = answerChoice;
          } else if (answerChoice == 1) {
            std::string output = "You leave the " + door_name + " untouched.";
            mainUI->ConsolePrintWithWait(output, 0, 0);
            unlockChoice = answerChoice;
          } else {
            mainUI->ConsolePrint("Not a yes or no answer, try again..", 0, 0);
            DoorProc(y, x);
          };
        } 
    } else if (door.isOpen) {
        std::string output = "You enter the doorway of the " + door_name; //Building our message
        mainUI->ConsolePrintWithWait(output, 0, 0);
        player->SetPos(x, y);

    } else {
        mainUI->ConsolePrintWithWait("The door here is closed..", 0, 0);

        int openChoice = -1;
        while (openChoice < 0) {
          mainUI->ClearConsole();
          std::string answer;
          std::string output = "Would you like to open the " + door_name + "? ";
          mainUI->ConsolePrint(output, 0, 0);
          answer = mainUI->ConsoleGetString();
          int answerChoice = processYesOrNoChoice(answer);
          mainUI->ClearConsole();
          if (answerChoice == 0) {
              std::string output = "You open the " + door_name;
              mainUI -> ConsolePrintWithWait(output, 0, 0);
              map -> OpenDoorTile(x, y);
              openChoice = answerChoice;
          } else if (answerChoice == 1) {
              std::string output = "You leave the " + door_name + " untouched.";
              mainUI->ConsolePrintWithWait(output, 0, 0);
              openChoice = answerChoice;
          } else {
              mainUI->ConsolePrint("Not a yes or no answer, try again..", 0, 0);
          };
        }

    }

    return (0);
}

void PlayerUI::LockProc(int y, int x) {
    int map_tile = map -> GetTile(x, y);
    Door door = map -> GetDoor(x, y);

    const Item* inv_tile;
    int keyCount = player->GetKeyCount();
    int requiredCount = door.lockCount;

    mainUI->ClearConsole();
    mainUI->ConsolePrint("How would you like to unlock the door? ", 0, 0);
    mainUI->ConsolePrint("1. Using Door Keys, 2. Using Lockpicks ", 1, 0);
    mainUI->ConsolePrint("Enter a choice to continue, or 'exit' to cancel ", 2, 0);
    mainUI->ConsolePrint("lock_proc:~$  ", 3, 0);
    std::string answer = StringUtils::toLowerCase(mainUI->ConsoleGetString());
    if ((answer == "1") || (answer == "key") || (answer == "keys") || (answer == "door key") || (answer == "door keys")) {
        std::ostringstream lockContextStream;
        mainUI->ClearConsole(); //Clear ready for response

        //Check we have enough keys for the number of locks on this door
        if (keyCount >= requiredCount) {
            player->RemoveKeyCount(requiredCount); //Use up/delete these keys
            lockContextStream << "You insert " << requiredCount << " keys into the door and open it .. ";
            mainUI->ConsolePrintWithWait(lockContextStream.str(),0,0);
            map -> UnlockDoorTile(x, y);
            return;
        } else {
            lockContextStream << "You need " << requiredCount << " keys to open this door.. ";
            mainUI->ConsolePrintWithWait(lockContextStream.str(),0,0);
            return;
        };

        return;
    } else if (answer == "2") {
        int lockpick_count = 0;
        int count = 0;

            for (int i = 0; i < INV_SIZE; i++) {
                inv_tile = player->GetFromInventory(i);

                if (inv_tile->name == item_library[lockpick].name) {
                    lockpick_count++; //Add to our lockpick count
                }

            }
       

        if (lockpick_count >= door.lockCount) {

                for (int i = 0; i < INV_SIZE; i++) {
                    inv_tile = player->GetFromInventory(i);

                    if (inv_tile->name == item_library[lockpick].name) {
                        int lockno = 1;
                        std::ostringstream lockContextStream;
                        mainUI->ClearConsole();
                        
                        lockContextStream << "This door has " << requiredCount << " lock(s).. ";
                        mainUI->ConsolePrint(lockContextStream.str(),0,0);
                        lockContextStream.str("");
                        lockContextStream.clear();
                        
                        lockContextStream << "You attempt to pick lock " << lockno;
                        mainUI->ConsolePrintWithWait(lockContextStream.str(),1,0);

                        int chance = rand() % 100 + 1;
                        if (chance > 50) {
                            mainUI->ConsolePrintWithWait("You manage to open the lock with the lockpick.. ", 0, 0);
                            count++;
                            lockno++;
                            
                        } else {
                            mainUI->ConsolePrintWithWait("Your lockpick breaks as you attempt to open the lock.. ", 0, 0);
                        }
                        
                        player->GetInventory()->RemoveItem(i);
                        chance = rand() % 100 + 1;
                    }

                    if (count == door.lockCount) {
                        mainUI->ClearConsole();
                        mainUI->ConsolePrintWithWait("You manage to unlock the door.. ", 0, 0);
                        map -> UnlockDoorTile(x, y);
                        player->SetPos(x, y);
                        return;
                    }

                }
           

        } else if (lockpick_count != door.lockCount) {
            std::ostringstream lockContextStream;
            mainUI->ClearConsole(); //Clear ready for response
            
            lockContextStream << "You need " << requiredCount << " lock picks to attempt to open this door.. ";
            mainUI->ConsolePrintWithWait(lockContextStream.str(),0,0);
            
            return;
        };

        if (count != door.lockCount) {
            std::ostringstream lockContextStream;
            mainUI->ClearConsole(); //Clear ready for response
            mainUI->ConsolePrintWithWait("You have run out of lockpicks..", 0, 0);

            return;
        };

        return;
    } else if ((answer == "exit")) {
        std::ostringstream lockContextStream;
        mainUI->ClearConsole(); //Clear ready for response
        
        lockContextStream << "You leave the " << door.name.c_str() << " untouched.";
        
        mainUI->ConsolePrintWithWait(lockContextStream.str(),0,0);
        return;
    } else {
        mainUI->ConsolePrintWithWait("Not a correct choice, try again.. ", 0, 0);
        LockProc(y, x);
    };

    return;
}

int PlayerUI::PlayerItemProc(Player* p, Item* itm, int x, int y)
{
    std::string answer;
    std::string itm_name = itm->name.c_str();

    char promptbuffer[30];
    sprintf(&promptbuffer[0], "There's a %s on the floor..", itm_name);
    std::string prompt = promptbuffer;
    mainUI->ConsolePrintWithWait(prompt, 0, 0);


    sprintf(&promptbuffer[0], "Would you like to pick up the %s?", itm_name);
    prompt = promptbuffer;
    mainUI->ConsolePrintWithWait(prompt, 0, 0);
    answer = mainUI->ConsoleGetString();
    int answerChoice = processYesOrNoChoice(answer);

    if (answerChoice == 0) {
        p->AddToInventory(itm);

        std::string prompt;
        sprintf(&promptbuffer[0], "You pick up the %s..", itm_name);
        prompt = promptbuffer;
        mainUI->ConsolePrintWithWait(prompt, 0, 0);
       // p->SetInventoryTile(x, y, new Item(item_library[no_item]));
        p->SetPos(x, y);
        return (0);
 
    } else if (answerChoice == 1) {
        sprintf(&promptbuffer[0], "You leave the %s untouched..", itm_name);
        prompt = promptbuffer;
        mainUI->ConsolePrintWithWait(prompt, 0, 0);
        p->SetPos(x, y);
        return (0);
    } else {
        mainUI->ConsolePrintWithWait("Incorrect choice, please answer yes or no.. ", 0, 0);
        PlayerItemProc(p, itm, x, y);
    }

    return (0);
}

/** Wrapper for the Player Move() method, making use of return values for UI context
 * 
 * @param x 
 * @param y
 */
void PlayerUI::PlayerMoveTurn(int x, int y, bool* levelEnded, bool* downLevel)
{
    std::string output;
    int eid;

    //Sanity check!
    if ((x < 0) | (y < 0) | (x >= GRID_X) | (y >= GRID_Y)) {
        mainUI->ConsolePrintWithWait("There's a large Granite wall here!", 0, 0); //Just give an out of bounds message that sounds vaguely believable
        return;
    }

    std::string move_tilename = tile_library [map->GetTile(x, y)].name;
    std::string enemy_name;// = npcs[eid].GetName(); // eid is undefined this is dangerous

    //main movement check
    if (map->IsTraversable(x, y)) {
        output = "You walk along the " + move_tilename;

        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait(output, 0, 0);

        //player->SetPos(x,y);  
    } else {
        output = "There's a " + move_tilename + " here";

        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait(output, 0, 0);

        TileProc(y, x, map->GetTile(x, y));
    }

    ////////////////////
    switch (map->MovePlayer(x, y, &eid)) {
        //Door    
    case 1:
        DoorProc(y, x);
        break;

        //Trap    
    case 2:
        break;

    //Enemy    
    case 3:
        enemy_name = npcs[eid].GetName();
        output = "You are confronted by a " + enemy_name;
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait(output, 0, 0);

        Battle(eid);
        break;

      //Dead body
    case 4:
        enemy_name = npcs[eid].GetName();

        output = "There is the corpse of a" + enemy_name + "here..";
        
        //Do you want to search it?
        //mainUI->AccessContainer(container_grid[x][y]);
        
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait(output, 0, 0);

        break;
        
        case 5: //Entrance
         mainUI->ClearConsole();
         mainUI->ConsolePrintWithWait("The way you came in is locked..", 0, 0);
         *levelEnded = true;
         *downLevel = false; //False takes us up to a previous level
         break;
        
        case 6: //Exit
            mainUI->ClearConsole();
            mainUI->ConsolePrintWithWait("You have reached the exit!", 0, 0);
            *levelEnded = true;
            *downLevel = true;
            return;
            break;
    }

    return;
}

/** A function to handle command input from the player.
 * 
 * @return true to continue, false to quit the game
 */
bool PlayerUI::TextInput()
{
    echo(); // enable echoing of text inputs on the UI
    mainUI->ClearConsole();
    mainUI->ConsolePrint(PROMPT_TEXT, 0, 0);
    std::string lowerCasedAnswer = StringUtils::toLowerCase(mainUI->ConsoleGetString());
    if (lowerCasedAnswer == "help") {
        mainUI->ConsolePrint("ihelp - interactions", 1, 0);
        mainUI->ConsolePrintWithWait("info - game info", 2, 0); //getch on last line 

    } else if (lowerCasedAnswer == "ihelp") {
        mainUI->ConsolePrintWithWait("drop - drop item (selection)", 0, 0);
    } else if (lowerCasedAnswer == "info") {
        mainUI->ShowInfo();
    } else if ((lowerCasedAnswer == "exit") || (lowerCasedAnswer == "quit")) {
        mainUI->ConsolePrintWithWait("Quitting.. ", 0, 0);
        return false;
    } else if (lowerCasedAnswer == "inventory") {
      AccessPlayerInv();
    } else {
        mainUI->ConsolePrintWithWait("unrecognised input, use 'help' for a examples. ", 0, 0);
    }

    return true;
}

/** A function to display the standard interface of controls
 * 
 */
void PlayerUI::ShowControls()
{
    mainUI->ConsolePrint("c - command prompt",0,0);
    mainUI->ConsolePrint("Arrow Keys to move.. ", 0, 1);
}

/**
 * 
 * @return false to exit or change level, true otherwise.
 */
bool PlayerUI::Input(bool* levelEnded, bool* downLevel)
{
    int x;
    int y;
    player->GetPos(&x, &y); /*Get current position for movement*/

    int thisX = x; //set movement positions to default
    int thisY = y;

    ShowControls();
    int choice = mainUI->ConsoleGetInput();

    if (choice == KEY_UP || choice == 'w') thisY--;
    else if (choice == KEY_RIGHT || choice == 'd') thisX++;
    else if (choice == KEY_DOWN || choice == 's') thisY++;
    else if (choice == KEY_LEFT || choice == 'a') thisX--;
    else if (choice == 'q' || choice == KEY_EXIT ) return false;
    else if (choice == 'c') return TextInput();
    else if (choice == 'i') AccessPlayerInv();

        //handle any movement input
    if ((x != thisX) || (y != thisY)) {
        PlayerMoveTurn(thisX, thisY,levelEnded,downLevel);
        
        if (*levelEnded) return false;
    }
    
    return true;
}

void PlayerUI::DrawNPCS()
{
    for (int a = 0; a < MAX_NPCS; a++) {
        int x, y, colour;
        char symbol;

        if (&npcs[a] != NULL) {
            npcs[a].GetPos(&x, &y);

            colour = npcs[a].GetColour();
            symbol = npcs[a].GetSymbol();

            mainUI->DrawCharacter(x, y, colour, symbol);
        };
    }

}

void PlayerUI::DrawPlayer()
{
    int x, y, colour;
    char symbol;

    player->GetPos(&x, &y);

    colour = player->GetColour();
    symbol = player->GetSymbol();

     mainUI->DrawCharacter(x, y, colour, symbol);
}
//
//void PlayerUI :: moveItemCommand(int index) {
//    Item* movingItem;
//   
//   mainUI->ClearConsole();
//   mainUI->ConsolePrintWithWait("Where would you like to move this to?", 0, 0);
//   mainUI->ConsolePrintWithWait("Where would you like to move this to?", 0, 0);
//   std::string input = mainUI->ConsoleGetString(); 
//    
//}

/** Allows the user to enter a command to perform operations on things in a list/inventory
 *  This allows dropping/moving items, etc
 * @param index The currently selected item index
 * @param mainUI The UI to call to
 * @param playerInv whether or not this is the player's inventory (to disable "take", etc)
 * @return 
 */
void PlayerUI :: AccessListCommand(Container* c, int index, bool playerInv) {
    echo();

    mainUI->ConsolePrintWithWait(PROMPT_TEXT, 0, 0);
    std::string input = mainUI->ConsoleGetString();

    if (input == "drop") {
        
    } else if (input == "move") {
        //1. Player Inv
        //2.. Location Inv
    } else if (input == "open") {
        //if (inventory->)
        const Item* itm = c->GetItem(index);   
        
        //Dynamic cast to type check
        const Container* c = dynamic_cast<const Container*> (itm);
        
        //If incorrectly cast (item), pointer will be NULL
        if (c != NULL) {
            AccessContainer((Container*)itm,false); //Cast and pass
        }
      
    } else {
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait("Huh?",0,0);
    }
    
    mainUI->ClearConsole(); //Clear ready for output
}

/**
 * 
 * @param choice int code for the player input
 * @param index The current selected item index in this list
 * @param c A pointer to this container
 * @param playerInv Whether or not this is the player's inventory or another container
 * @return 
 */
bool PlayerUI::InventoryInput(int choice, int index, Container* c, bool playerInv) {
     switch(choice) {
            case ('q') : {
                return false;
                break;
            }
            case ('c') : {
                AccessListCommand(c, index, playerInv);
                break;
            }
           case ('i') : { //Item info
                break;
            }
        }
     return true;
}


/**
 * 
 * @param c         
 * @param playerInv whether or not we are current looking at the player's inventory, disables 'take'
 * @return 
 */
void PlayerUI::AccessContainer(Container * c, bool playerInv)
{
    bool selection = true;
    long unsigned int index = 0;
    long unsigned int invIndex = 0; //The index of the topmost item on the screen, alows scrolling
    
    //Selection loop
    while(selection == true) {
        
        mainUI->ListInv(c,invIndex);
        mainUI->HighlightInvLine(index);      
           
        int choice =mainUI->ConsoleGetInput();
        long unsigned int invSize = c->GetSize()-1; 
        
        if (choice == KEY_UP) {
            if (index>0) {
                index--; 
            } else if (index==0 && invIndex>0) {
                invIndex--;
            }
        } else if(choice == KEY_DOWN) { 
            unsigned long int invWindowLimit = INVWIN_FRONT_Y-2;
            //Checking against the window boundary. invSize-1 allows
            if (index<invWindowLimit && index<invSize) {
                if (index==invWindowLimit-1 && invIndex<invSize) {
                    invIndex++;
                } else {
                    index++; 
                }
            } 
        } else {
            selection = InventoryInput(choice, index, c, playerInv);
        }
    }
    
}

void PlayerUI::TileProc(int y, int x, tile t) {
    if (t == ent) {
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait("The way you came in is locked..", 0, 0);
        return;
    } else if (t == ext) {
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait("You have reached the exit!", 0, 0);
        return;
    } else if (t == ded) {
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait("The floor caves in below one of your feet, injuring you..", 0, 0);
        player->SetHealth(player->GetHealth() - 20);
        return;
    };

}


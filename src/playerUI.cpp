#include "playerUI.h"

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

                    hitTextStream << "You use your " << p_choice.GetName().c_str() << " and strike the enemy for " << playerDamage << " damage " << "\n";
                    mainUI->ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                } else if (p_move == 0 && npc_move != 0) {
                    player->SetHealth(player->GetHealth() - npcDamage);

                    hitTextStream << "You do nothing, and are struck for " << npcDamage << " damage." << "\n";
                    mainUI->ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                };

                //Enemy move
                if (npc_move != 0 && p_move != 0) {
                    player->SetHealth(player->GetHealth() - npcDamage);

                    hitTextStream << "Your enemy uses their " << npc_choice.GetName().c_str() << " and deals " << npcDamage << " damage " << "\n";
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
    inventoryUI -> AccessContainer(player->GetInventory(), true);
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
        std::string wepName = weps[i].GetName();
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


int PlayerUI::DoorProc(int x, int y) {
  if (map -> HasDoorAt(x, y)) {
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
            DoorProc(x, y);
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
  } else {
    logging -> logline("Player tried to open a door at: " + std::to_string(x) + ", " + std::to_string(y));
  }
    return (0);
}

void PlayerUI::LockProc(int y, int x) {
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

                if (inv_tile->GetName() == item_library[lockpick].GetName()) {
                    lockpick_count++; //Add to our lockpick count
                }

            }
       

        if (lockpick_count >= door.lockCount) {

                for (int i = 0; i < INV_SIZE; i++) {
                    inv_tile = player->GetFromInventory(i);

                    if (inv_tile->GetName() == item_library[lockpick].GetName()) {
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

void PlayerUI::PlayerContainerProc(Player* p, Container* container)
{
    std::string answer;
    std::string containerName = container->GetName().c_str();
    logging -> logline("Processing encountered container: " + containerName);
    
    std::string prompt;
    char promptbuffer[30];
    ContainerType containerType = container -> GetContainerType();
    int answerChoice = 0;
    if (containerType == OBJECT) {
      sprintf(&promptbuffer[0], "There's a %s here..", containerName);
      std::string prompt = promptbuffer;
      mainUI->ConsolePrintWithWait(prompt, 0, 0);

      sprintf(&promptbuffer[0], "Would you like to open the %s?", containerName);
      prompt = promptbuffer;
      mainUI->ConsolePrintWithWait(prompt, 0, 0);
      answer = mainUI->ConsoleGetString();
      answerChoice = processYesOrNoChoice(answer);

    if (answerChoice == 0) {
       inventoryUI -> AccessContainer(container, false); 
    } else if (answerChoice == 1) {
        sprintf(&promptbuffer[0], "You leave the %s untouched..", containerName);
        prompt = promptbuffer;
        mainUI->ConsolePrintWithWait(prompt, 0, 0);
    } else {
        mainUI->ConsolePrintWithWait("Incorrect choice, please answer yes or no.. ", 0, 0);
        PlayerContainerProc(p, container);
    }
  }
}

int PlayerUI::PlayerItemProc(Player* p, const Item* itm, Position itemPosition)
{
    std::string answer;
    const char* itm_name_buf = itm->GetName().c_str();
    std::string itm_name = itm_name_buf;
    logging -> logline("Processing encountered item: " + itm_name);
    char promptbuffer[30];
    sprintf(&promptbuffer[0], "There's a %s on the floor..", itm_name_buf);
    std::string prompt = promptbuffer;
    mainUI->ConsolePrintWithWait(prompt, 0, 0);


    sprintf(&promptbuffer[0], "Would you like to pick up the %s?", itm_name_buf);
    prompt = promptbuffer;
    mainUI->ConsolePrintWithWait(prompt, 0, 0);
    answer = mainUI->ConsoleGetString();
    int answerChoice = processYesOrNoChoice(answer);

    if (answerChoice == 0) {
        // TODO remvove from map
        p->AddToInventory(itm);

        std::string prompt;
        sprintf(&promptbuffer[0], "You pick up the %s..", itm_name_buf);
        prompt = promptbuffer;
        mainUI->ConsolePrintWithWait(prompt, 0, 0);
        p->SetPos(itemPosition.x, itemPosition.y);
        return (0);
 
    } else if (answerChoice == 1) {
        sprintf(&promptbuffer[0], "You leave the %s untouched..", itm_name_buf);
        prompt = promptbuffer;
        mainUI->ConsolePrintWithWait(prompt, 0, 0);
        return (0);
    } else {
        mainUI->ConsolePrintWithWait("Incorrect choice, please answer yes or no.. ", 0, 0);
        PlayerItemProc(p, itm, itemPosition);
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
    if ((x < 0) | (y < 0) | (x >= map -> GetGridX()) | (y >= map -> GetGridY())) {
        mainUI->ConsolePrintWithWait("You can't see anything!", 0, 0); //Just give an out of bounds message that sounds vaguely believable
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

        TileProc(map->GetTile(x, y));
    }

    ////////////////////
    switch (map->MovePlayer(x, y, &eid)) {
        //Door    
    case 1:
        DoorProc(x, y);
        break;

        //Trap    
    case 2:
        break;

    //Enemy    
    case 3: {
        enemy_name = npcs[eid].GetName();
        output = "You are confronted by a " + enemy_name;
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait(output, 0, 0);

        Battle(eid);
        break;
      }
    // Containers 
    case 4: {
        Container container = map -> GetContainer(x,y);
        PlayerContainerProc(player, &container);
        break;
    }
    case 5: //Entrance
      mainUI->ClearConsole();
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

void PlayerUI::Interact() {
    mainUI->ClearConsole();
    
    Position playerPos = player -> GetPosition();
    Position inputPos = Position(playerPos);
    int choice = mainUI->ConsolePrintWithWait("What would you like to interact with? (use directions to select, q to cancel)", 0, 0); 
    ProcessMovementInput(choice, &inputPos);
    if (choice == 'q') return;
    
    if (inputPos != playerPos) {
      logging -> logline("Player pos: " + std::to_string(playerPos.x) + ", " + std::to_string(playerPos.y));
      logging -> logline("Player tried to interact at: " + std::to_string(inputPos.x) + ", " + std::to_string(inputPos.y));
      Container container = map -> GetContainer(inputPos.x, inputPos.y);
      if (map -> HasDoorAt(inputPos.x, inputPos.y)) {
        DoorProc(inputPos.x, inputPos.y);
      } else if (container.IsOpenable()){ 
        this -> inventoryUI -> AccessContainer(&container, false);
      } else {
        mainUI->ClearConsole();
        mainUI->ConsolePrintWithWait("There's nothing to interact with here.", 0, 0); 
      }
    }
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
        mainUI->ConsolePrintWithWait("inventory - access inventory (i)", 0, 0);
        mainUI->ConsolePrintWithWait("use - interact with a door/container (u)", 0, 0);
        mainUI->ConsolePrintWithWait("Pause menu (q)", 0, 0);
    } else if (lowerCasedAnswer == "inventory") {
      AccessPlayerInv();
    } else if (lowerCasedAnswer == "use") {
      Interact();
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

void PlayerUI::ProcessMovementInput(int choice, Position* p) {
  if (choice == KEY_UP || choice == 'w') p->y--;
  else if (choice == KEY_DOWN || choice == 's') p->y++;
  else if (choice == KEY_RIGHT || choice == 'd') p->x++;
  else if (choice == KEY_LEFT || choice == 'a') p->x--;
}

/**
 * 
 * @return false to exit or change level, true otherwise.
 */
bool PlayerUI::Input(bool* levelEnded, bool* downLevel)
{
    Position playerPos = player->GetPosition(); /*Get current position for movement*/

    Position inputPos = Position(playerPos);

    ShowControls();
    int choice = mainUI->ConsoleGetInput();
    if (choice == 'q' || choice == KEY_EXIT ) return false;
    else if (choice == 'c') return TextInput();
    else if (choice == 'i') AccessPlayerInv();
    else if (choice == 'u') Interact();
    ProcessMovementInput(choice, &inputPos);

    if (inputPos != playerPos) PlayerMoveTurn(inputPos.x, inputPos.y, levelEnded, downLevel);
    return !(*levelEnded);
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


void PlayerUI::TileProc(tile t) {
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

int PlayerUI::TakeItem(Container* container, int index)
{
  const Item* item = container -> GetItem(index);
  player -> AddToInventory(item);
  container -> RemoveItem(index);
}

int PlayerUI::DropPlayerItem(const Item* thisItem)
{
    int x, y;
    player -> GetPos(&x, &y);
    //if the player is at an area where items can be placed, add the item
    if (map -> CanPlaceItems(x, y)) {
        map -> AddToContainer(x, y, thisItem); //replace the map tile with the item
        player -> GetInventory() -> RemoveItem(thisItem); //clear the inventory tile
        return 0;
    }

    return 1;
}

int PlayerUI::MoveContainer(Container* container)
{   
    return 1;
}



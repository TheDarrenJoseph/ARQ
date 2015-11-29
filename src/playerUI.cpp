#include "playerUI.h"

void PlayerUI::Battle(int npc_id)
{
    std::string p_name = player->GetName();
    std::string npc_name = npcs[npc_id].GetName();

    mainUI.ClearConsole();

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

            mainUI.ConsolePrintWithWait("You have been slain..", 0, 0);
            return;
        } else if (npcs[npc_id].IsAlive() == false) {
            npcs[npc_id].Kill();
            mainUI.ConsolePrintWithWait("Your foe has been slain..", 0, 0);
            return;
        } else {
            std::ostringstream playerHealthStream;
            std::ostringstream enemyHealthStream;

            playerHealthStream << p_name.c_str() << " Health: " << player->GetHealth() << "\n";
            enemyHealthStream << npc_name.c_str() << " Health: " << npcs[npc_id].GetHealth() << "\n";
            const std::string playerHealthText = playerHealthStream.str();
            const std::string enemyHealthText = playerHealthStream.str();

            //display the info of both combatants
            mainUI.ClearConsole();
            mainUI.ConsolePrint(playerHealthText, 0, 0);
            mainUI.ConsolePrintWithWait(enemyHealthText, 0, 1);

            //Get both combatants weapons
            weapon* p_weps = player->GetWeps();
            weapon* npc_weps = npcs[npc_id].GetWeps();

            //allow both combatants to make a move
            int p_move = BattleTurn(npc_id);
            int npc_move = npcs[npc_id].BattleTurn();

            if ((p_move <= 2) && (npc_move <= 2) && (npcs[npc_id].IsAlive()) && (player->IsAlive())) {
                //select the weapon using their move choice
                weapon p_choice = p_weps[p_move];
                weapon npc_choice = npc_weps[npc_move];

                //show each players choices
                mainUI.ClearConsole();
                std::ostringstream hitTextStream;
                int playerDamage = p_choice.damage;
                int npcDamage = npc_choice.damage;

                //Building battle context text! String streams perhaps not the best here,
                //maybe find a way to handle params like wprintw("%param",var)??

                //Player move
                if (p_move != 0) {
                    npcs[npc_id].SetHealth(npcs[npc_id].GetHealth() - playerDamage);

                    hitTextStream << "You use your " << p_choice.name.c_str() << " and strike the enemy for " << playerDamage << " damage " << "\n";
                    mainUI.ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                } else if (p_move == 0 && npc_move != 0) {
                    player->SetHealth(player->GetHealth() - npcDamage);

                    hitTextStream << "You do nothing, and are struck for " << npcDamage << " damage." << "\n";
                    mainUI.ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                };

                //Enemy move
                if (npc_move != 0 && p_move != 0) {
                    player->SetHealth(player->GetHealth() - npcDamage);

                    hitTextStream << "Your enemy uses their " << npc_choice.name.c_str() << " and deals " << npcDamage << " damage " << "\n";
                    mainUI.ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                } else if (npc_move == 0 && p_move != 0) {
                    npcs[npc_id].SetHealth(npcs[npc_id].GetHealth() - playerDamage);

                    hitTextStream << "Your enemy does nothing, and takes " << playerDamage << " damage " << "\n";
                    mainUI.ConsolePrintWithWait(hitTextStream.str(), 0, 0);
                }
                else {
                    mainUI.ConsolePrintWithWait("You both do nothing.", 0, 0);
                };
            }
            else if (p_move == 3) {
                return; //default exit for now, need to refactor this

                //FLEE CODE HERE
                mainUI.ClearConsole();
                mainUI.ConsolePrintWithWait("You attempt to flee.", 0, 0);

                /**
	  
                if (player->CanFlee(map))
                    if (player->Flee(map) == 0)
                       {
                         wprintw(consolewin_front,"You manage to escape the fight."); 
                         wgetch (consolewin_front);
                         return;
                        }
		 
                else..  */
                mainUI.ClearConsole();
                mainUI.ConsolePrintWithWait("You try to escape but have nowhere to run to.", 0, 0);

            }
        }
    }

}

void PlayerUI::DrawPlayerInv()
{
    mainUI.DrawInv(player->GetInventory());
};

void PlayerUI::DrawPlayerEquipment()
{
    weapon* weps = player->GetWeps();

    for (int x = 0; x < INV_X - 1; x++) {
        std::string name = weps[x].name;
        mainUI.DrawPlayerEquipmentSlot(x, name);
    };

    mainUI.DrawPlayerEquipmentSlot(INV_X, player->GetOutfit().name);
};

int PlayerUI::BattleTurn(int npc_id)
{
    mainUI.ClearConsole();

    for (int i = 0; i < 3; i++) {
        Item* weps = player->GetWeps();
        std::string wepName = weps[i].name;
        mainUI.ConsolePrint(wepName, 0, (9 * i));
    }

    mainUI.ConsolePrint("Flee", 0, (9 * 3));

    ////input
    int index = 1;

    bool selection = true;

    while (selection == true) {
        int scr_x = index * 9; //calculate the start of the item name
        int scr_y = 0;
        mainUI.HighlightConsole(scr_x, scr_y);

        mainUI.ConsolePrint("Use A and D to choose a weapon, 'use' to select", 1, 0);
        mainUI.ConsolePrint("ARQ:~$ ", 2, 0); //give us a prompt

        std::string choice = mainUI.ConsoleGetString();
        if ((choice == "a") && (index > 0)) {
            index--;
        } else if ((choice == "d") && (index < 3)) {
            index++;
        } else if (choice == "use") {
            mainUI.ClearConsole();
            mainUI.ConsolePrintWithWait("Weapon Selected", 0, 0);
            return index;
        }

    }

    return 0;
}

int PlayerUI::DoorProc(int y, int x, tile doortype)
{
    int map_tile = map->GetTile(x, y);

    std::string door_name = tile_library[map_tile].name;

    if ((doortype == od0) || (doortype == od1) || (doortype == od2)) {
        std::string output = "You enter the doorway of the" + door_name; //Building our message

        mainUI.ConsolePrintWithWait(output, 0, 0);

        player->SetPos(x, y);

        return (0);
    } else if ((doortype == cd0) || (doortype == cd1) || (doortype == cd2)) {
        std::string answer;

        std::string output = "Would you like to open the " + door_name + "?";
        mainUI.ConsolePrint(output, 0, 0);
        answer = mainUI.ConsoleGetString();

        if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y")) {
            std::string output = "You open the " + door_name + " and step into the doorway";
            mainUI.ConsolePrint(output, 0, 0);


            tile door = map->GetTile(x, y);
            if (door == (cd0) || door == cd1 || door == cd2) //Checking main tile
            {
                map->SetTile(x, y, od0);

                if (map->GetTile(x, y + 1) == (door)) //Checking for surrounding door tiles
                {
                    map->SetTile(x, y + 1, od0);
                } else if (map->GetTile(x, y - 1) == (door)) {
                    map->SetTile(x, y - 1, od0);
                } else if (map->GetTile(x + 1, y) == (door)) {
                    map->SetTile(x + 1, y, od0);
                } else if (map->GetTile(x - 1, y) == (door)) {
                    map->SetTile(x - 1, y, od0);
                };
            };


            return (0);
        } else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N")) {
            std::string output = "You leave the " + door_name + " untouched";

            mainUI.ClearConsole();
            mainUI.ConsolePrintWithWait(output, 0, 0);

            return (0);
        } else {
            mainUI.ClearConsole();
            mainUI.ConsolePrint("Not a yes or no answer, try again..", 0, 0);

            DoorProc(y, x, map->GetTile(x, y));
        };

        return (0);
    } else if ((doortype == ld1) || (doortype == ld2)) {
        std::string answer;

        mainUI.ClearConsole();
        mainUI.ConsolePrint("Would you like to open the door? ", 0, 0);
        answer = mainUI.ConsoleGetString();

        if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y")) {
            LockProc(y, x, doortype, map_tile, door_name);
            return (0);
        } else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N")) {
            std::string output = "You leave the " + door_name + " untouched";

            mainUI.ConsolePrintWithWait(output, 0, 0);

            return (0);
        } else {
            mainUI.ConsolePrintWithWait("Not a yes or no answer, try again..", 0, 0);

            DoorProc(y, x, map->GetTile(x, y));
        };
        return (0);
    };

    return (0);
}

void PlayerUI::LockProc(int door_y, int door_x, tile doortype, int doortile, std::string doorname)
{
    std::string answer;
    Item* inv_tile;
    int keyCount = player->GetKeyCount();
    int requiredCount = tile_library[doortile].locks;

    mainUI.ClearConsole();
    mainUI.ConsolePrint("How would you like to unlock the door? ", 0, 0);
    mainUI.ConsolePrint("1. Using Door Keys, 2. Using Lockpicks ", 1, 0);
    mainUI.ConsolePrint("Enter a choice to continue, or 'exit' to cancel ", 2, 0);
    mainUI.ConsolePrint("lock_proc:~$  ", 3, 0);

    answer = mainUI.ConsoleGetString();

    if ((answer == "1") || (answer == "Key") || (answer == "Keys") || (answer == "key") || (answer == "keys") || (answer == "Door Key") || (answer == "Door Keys") || (answer == "door key") || (answer == "door keys")) {
        std::ostringstream lockContextStream;
        mainUI.ClearConsole(); //Clear ready for response

        //Check we have enough keys for the number of locks on this door
        if (keyCount >= requiredCount) {
            player->RemoveKeyCount(requiredCount); //Use up/delete these keys
            
            lockContextStream << "You insert " << requiredCount << " keys into the door and open it .. ";
            mainUI.ConsolePrintWithWait(lockContextStream.str(),0,0);
            
            if (doortype == ld1) {
                
                if (map->GetTile(door_x, door_y + 1) == (ld1)) //Checking for surrounding door tiles
                {
                    map->SetTile(door_x, door_y + 1, od0);
                } else if (map->GetTile(door_x, door_y - 1) == (ld1)) {
                    map->SetTile(door_x, door_y - 1, od0);
                } else if (map->GetTile(door_x + 1, door_y) == (ld1)) {
                    map->SetTile(door_x + 1, door_y, od0);
                } else if (map->GetTile(door_x - 1, door_y) == (ld1)) {
                    map->SetTile(door_x - 1, door_y, od0);
                };
            } else if (doortype == ld2) {
                map->SetTile(door_x, door_y, od2);

                if (map->GetTile(door_x, door_y + 1) == (ld2)) //Checking for surrounding door tiles
                {
                    map->SetTile(door_x, door_y + 1, od0);
                } else if (map->GetTile(door_x, door_y - 1) == (ld2)) {
                    map->SetTile(door_x, door_y - 1, od0);
                } else if (map->GetTile(door_x + 1, door_y) == (ld2)) {
                    map->SetTile(door_x + 1, door_y, od0);
                } else if (map->GetTile(door_x - 1, door_y) == (ld2)) {
                    map->SetTile(door_x - 1, door_y, od0);
                };
            };

            player->SetPos(door_x, door_y);
            return;
        } else {
            lockContextStream << "You need " << requiredCount << " keys to open this door.. ";
            mainUI.ConsolePrintWithWait(lockContextStream.str(),0,0);
            return;
        };

        return;
    } else if (answer == "2") {
        int lockpick_count = 0;
        int count = 0;

        for (int y = 0; y < INV_Y; y++) {
            for (int x = 0; x < INV_X; x++) {
                inv_tile = player->GetFromInventory(x, y);

                if (inv_tile->name == item_library[lockpick].name) {
                    lockpick_count +=1; //Add to our lockpick count
                };

            };
        };

        if (lockpick_count >= (tile_library[doortile].locks)) {

            for (int y = 0; y < INV_Y; y++) {
                for (int x = 0; x < INV_X; x++) {
                    inv_tile = player->GetFromInventory(x, y);

                    if (inv_tile->name == item_library[lockpick].name) {
                        int chance = rand() % 100 + 1;
                        int lockno = 1;
                        std::ostringstream lockContextStream;
                        mainUI.ClearConsole();
                        
                        lockContextStream << "This door has " << requiredCount << " lock(s).. ";
                        mainUI.ConsolePrint(lockContextStream.str(),0,0);
                        lockContextStream.str("");
                        lockContextStream.clear();
                        
                        lockContextStream << "You attempt to pick lock " << lockno;
                        mainUI.ConsolePrintWithWait(lockContextStream.str(),1,0);

                        if (chance > 50) {
                            mainUI.ConsolePrintWithWait("You manage to open the lock with the lockpick.. ", 0, 0);
                            count++;
                            lockno++;
                            
                        } else {
                            mainUI.ConsolePrintWithWait("Your lockpick breaks as you attempt to open the lock.. ", 0, 0);
                        };
                        
                       player->SetInventoryTile(x, y, new Item(item_library[no_item]));
                       chance = rand() % 100 + 1;
                    };

                    if (count == (tile_library[doortile].locks)) {
                        mainUI.ClearConsole();
                        mainUI.ConsolePrintWithWait("You manage to unlock the door.. ", 0, 0);
                        
                        //Setting the correct open door
                        if (doortype == ld1) {
                            map->SetTile(door_x, door_y, od1);
                        } else if (doortype == ld2) {
                            map->SetTile(door_x, door_y, od2);
                        };

                        player->SetPos(door_x, door_y);
                        return;
                    };

                };
            };

        } else if (lockpick_count != (tile_library[doortile].locks)) {
            std::ostringstream lockContextStream;
            mainUI.ClearConsole(); //Clear ready for response
            
            lockContextStream << "You need " << requiredCount << " lock picks to attempt to open this door.. ";
            mainUI.ConsolePrintWithWait(lockContextStream.str(),0,0);
            
            return;
        };

        if (count != (tile_library[doortile].locks)) {
            std::ostringstream lockContextStream;
            mainUI.ClearConsole(); //Clear ready for response
            mainUI.ConsolePrintWithWait("You have run out of lockpicks..", 0, 0);

            return;
        };

        return;
    } else if ((answer == "Exit") || (answer == "EXIT") || (answer == "exit")) {
        std::ostringstream lockContextStream;
        mainUI.ClearConsole(); //Clear ready for response
        
        lockContextStream << "You leave the " << doorname.c_str() << " untouched.";
        
        mainUI.ConsolePrintWithWait(lockContextStream.str(),0,0);
        return;
    } else {
        mainUI.ConsolePrintWithWait("Not a correct choice, try again.. ", 0, 0);
        LockProc(door_y, door_x, doortype, doortile, doorname);
    };

    return;
};

/*Wrapper for the Player Move() method, making use of return values for UI context*/
void PlayerUI::PlayerMoveTurn(int x, int y)
{
    std::string output;
    int eid;

    if ((x < 0) | (y < 0) | (x > GRID_X) | (y > GRID_Y)) {

        mainUI.ConsolePrintWithWait("There's a large Granite wall here!", 0, 0);
        return;
    }

    std::string move_tilename = tile_library [map->GetTile(x, y)].name;
    std::string enemy_name = npcs[eid].GetName();

    //main movement check
    if (map->IsTraversable(x, y)) {

        output = "You walk along the " + move_tilename;
        mainUI.ConsolePrintWithWait(output, 0, 0);

        //player->SetPos(x,y);  
    } else {

        output = "There's a " + move_tilename + " here";
        mainUI.ConsolePrintWithWait(output, 0, 0);

        TileProc(y, x, map->GetTile(x, y));
    }

    ////////////////////
    switch (map->MovePlayer(x, y, &eid)) {
        //Door    
    case 1:
        DoorProc(y, x, map->GetTile(x, y));
        break;

        //Trap    
    case 2:
        //Trap found!!

        //Enemy    
    case 3:
        output = "You are confronted by a " + enemy_name;
        mainUI.ConsolePrintWithWait(output, 0, 0);

        Battle(eid);
        break;

        //Dead body
    case 4:
        std::string enemy_name = npcs[eid].GetName();

        output = "There is the corpse of a" + enemy_name + "here..";
        mainUI.ConsolePrintWithWait(output, 0, 0);

        break;
    }
}

bool PlayerUI::TextInput()
{

    echo();

    std::string answer;

    mainUI.ClearConsole();
    mainUI.ConsolePrint("ARQ:~$ ", 0, 0);
    answer = mainUI.ConsoleGetString();

    if (answer == "help") {
        mainUI.ConsolePrint("ihelp - interactions", 1, 0);
        mainUI.ConsolePrintWithWait("info - game info", 2, 0); //getch on last line 

    } else if (answer == "ihelp") {
        mainUI.ConsolePrintWithWait("drop - drop item (selection)", 0, 0);

    } else if (answer == "info") {
        mainUI.ShowInfo();
    } else if ((answer == "exit") || (answer == "Exit") || (answer == "EXIT") || (answer == "quit") || (answer == "Quit") || (answer == "QUIT")) {
        mainUI.ConsolePrintWithWait("Quitting.. ", 0, 0);
        return false;
    } else if (answer == "inventory") AccessInventory();

    else {
        mainUI.ConsolePrintWithWait("unrecognised input, please input a command, or use 'help' for a list. ", 0, 0);
        Input();
    }

    return true;
}

void PlayerUI::ShowControls()
{
     mainUI.ConsolePrintWithWait("CONTROLS GO HERE :'D", 0, 0);
}

bool PlayerUI::Input()
{
    int x;
    int y;
    int thisX;
    int thisY;

    player->GetPos(&x, &y); /*Get current position for movement*/

    thisX = x; //set movement positions to default
    thisY = y;

    ShowControls();
    int choice = mainUI.ConsoleGetInput();

    if (choice == KEY_UP || choice == 'w') thisY--;
    else if (choice == KEY_RIGHT || choice == 'd') thisX++;
    else if (choice == KEY_DOWN || choice == 's') thisY++;
    else if (choice == KEY_LEFT || choice == 'a') thisX--;
    else if (choice == 'i') return TextInput();

    //handle any movement input
    if ((x != thisX) || (y != thisY)) {
        PlayerMoveTurn(thisX, thisY);
    }

    return true;
};

void PlayerUI::DrawNPCS()
{
    for (int a = 0; a < MAX_NPCS; a++) {
        int x, y, colour;
        char symbol;

        if (&npcs[a] != NULL) {
            npcs[a].GetPos(&x, &y);

            colour = npcs[a].GetColour();
            symbol = npcs[a].GetSymbol();

            mainUI.DrawCharacter(x, y, colour, symbol);
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

     mainUI.DrawCharacter(x, y, colour, symbol);
}

int PlayerUI::AccessArea(Area * a)
{
    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    while (selection == true) {
        Item* thisItem = a->GetItem(loc_x, loc_y);
        
        mainUI.ClearInvHighlighting();


        //area b = *a;
        mainUI.DrawInv(a);
        mainUI.UpdateUI(); //ensures items display propely

        mainUI.HighlightInv(loc_x,loc_y);

        mainUI.ClearConsole();
        mainUI.ConsolePrint("Select an item with WASD.. 'help' for commands", 0, 0);
        mainUI.ConsolePrintWithWait("ARQ:~$ ", 2, 0);

        std::string input = mainUI.ConsoleGetString();
        mainUI.ClearConsole(); //Get ready for our output!
        
        if ((input == "W") || (input == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((input == "A") || (input == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((input == "S") || (input == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((input == "D") || (input == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if (input == "close") {
            selection = false;

            return (0);
        } else if (input == "what") {
            std::string output = "This is a "+thisItem->name+".";
            mainUI.ConsolePrintWithWait(output,0,0);
        } else if (input == "take") {
            if (IsLootable(thisItem)) {
                player->AddToInventory(thisItem);
                a->RemoveItem(loc_x, loc_y);
                
                std::string output = "You put the "+thisItem->name+" into your inventory.";

                mainUI.ConsolePrintWithWait(output,0,0);
            }
        } else if (input== "put") {
            Item* thisItem = player->GetFromInventory(loc_y, loc_x);
            std::string output = "You put the "+thisItem->name+" inside.";

            mainUI.ConsolePrintWithWait(output,0,0);

            if (thisItem != NULL) {
                a->AddItem(thisItem);
            }
        } else if (input== "help") {
            mainUI.ConsolePrintWithWait("what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container",0,0);
        } else {
            mainUI.ConsolePrintWithWait("Not a correct selection, try again.",0,0);
        };

        //std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (0);
};

int PlayerUI::AccessContainer(Container * c)
{
    std::string input;

    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    while (selection == true) {
        Item* thisItem = c->GetItem(loc_x, loc_y);

        mainUI.DrawInv(c); //draw the current items
        mainUI.UpdateUI(); //call a full system UI refresh to fix display bug

        mainUI.ClearInvHighlighting();

        //Highlight item

        mainUI.ConsolePrint("Select an item with WASD.. 'help' for commands", 0, 0);
        mainUI.ConsolePrintWithWait("ARQ:~$ ", 2, 0);
        input = mainUI.ConsoleGetString();
        mainUI.ClearConsole(); //Clear ready for output

        if ((input == "W") || (input == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((input == "A") || (input == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((input == "S") || (input == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((input == "D") || (input == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if (input == "close") {
            selection = false;
            return (0);
        } else if (input == "what") {
            
            std::string output = "This is a " +thisItem->name;
            mainUI.ConsolePrintWithWait(output,0,0);
            
        } else if (input == "take") {
            player->AddToInventory(thisItem);
            c->RemoveItem(loc_x, loc_y);

            std::string output = "You put the "+thisItem->name+" into your inventory.";
            mainUI.ConsolePrintWithWait(output,0,0);
            
        } else if (input == "put") {
            Item* thisItem = player->GetFromInventory(loc_x, loc_y);

            if (thisItem != NULL) {
                c->AddItem(thisItem);
            }
        } else if (input == "help") {
            mainUI.ConsolePrintWithWait("what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container", 0, 0);
        } else {
            mainUI.ConsolePrintWithWait("Not a correct selection, try again.", 0, 0);
        };

        //std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (0);
};

int PlayerUI::AccessInventory()
{
    Item* inv_tile;
    int invtile_colour;

    const char* thisChar;

    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    while (selection == true) {
        Item* thisItem = player->GetFromInventory(loc_x, loc_y);

        mainUI.ClearInvHighlighting();
        mainUI.HighlightInv(loc_x,loc_y);

        //DrawInv(a);
        mainUI.UpdateUI();
        mainUI.ClearConsole();
        mainUI.ConsolePrint("Select an item with WASD.. 'help' for commands.", 0, 0);
        mainUI.ConsolePrint("ARQ:~$ ", 2, 0);

        std::string input = mainUI.ConsoleGetString();

        if ((input == "W") || (input == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((input == "A") || (input == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((input == "S") || (input == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((input == "D") || (input == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if (input == "close") {
            selection = false;
            return (0);
        } else if (input == "what") {
            mainUI.ClearConsole();
            
            std::string output = "This is a "+thisItem->name;
            mainUI.ConsolePrintWithWait(output,0,0);
           
        } else if (input == "drop") {
            mainUI.ClearConsole();

            Item* thisItem = player->GetFromInventory(loc_x, loc_y);

            if (CanDropItem(thisItem)) {

                if (map->DropPlayerItem(player, thisItem, loc_x, loc_y) == 0) {
                    //update the current item details 
                    //const char* invtile_char = thisItem->name.c_str();

                    //WINDOW* thisWin = invwins_front[loc_x][loc_y];

                    //update the inventory tile
                    //wmove(thisWin, 0, 0);
                    //wprintw_col(thisWin, invtile_char, thisItem->colour);
                    //wrefresh(thisWin);

                    mainUI.ConsolePrintWithWait("Item dropped",0,0);
                } else {
                    mainUI.ConsolePrintWithWait("Cannot drop that here.",0,0);
                }
            } else {
                mainUI.ConsolePrintWithWait("Cannot drop this item.",0,0);
            };

        } else if (input == "wear") {
            if (IsLootable(player->GetFromInventory(loc_x, loc_y))) {
                inv_tile = player->GetInventory()->GetItem(loc_x, loc_y); //grab the current inventory tile

                invtile_colour = inv_tile->colour;
                thisChar = inv_tile->symbol;

                const char *invtile_char = thisChar;

                mainUI.DrawInvWindow(loc_x,loc_y,invtile_char, invtile_colour);

                for (int i = 0; i < outfit_size; i++) {
                    mainUI.ClearConsole();
                    std::string output;
                    std::ostringstream outputStream; 
                    outputStream << thisChar << " selected";
                    mainUI.ConsolePrintWithWait(outputStream.str(),0,0);

                    //if this item matches an outfit, assume it is and equip it
                    if (thisChar == outfit_library[i].symbol) {
                        mainUI.ClearConsole();
                        //store the old outfit
                        outfit oldOutfit = player->GetOutfit();
                        output = "You change from "+oldOutfit.name+" into "+thisChar;
                        mainUI.ConsolePrintWithWait(output,0,0);

                        //change into the new outfit
                        player->SetOutfit(outfit_library[i]);

                        mainUI.ClearConsole();
                        output = "You put your old "+oldOutfit.name+" into your inventory";
                        mainUI.ConsolePrintWithWait(output,0,0);
                        
                        //set the current inventory tile to the old outfit
                        player->SetInventoryTile(loc_x, loc_y, new outfit(oldOutfit)); //instanciates a new outfit to fix polymorphism issues
                    }
                }


            } else {
                mainUI.ClearConsole();
                mainUI.ConsolePrint("No item selected",0,0);
            };

        } else if (input == "open") {
            int thisId = player->GetFromInventory(loc_x, loc_y)->id;

            //98 denotes container, which is a closed-ended area, we never want to access an area here
            if (thisId == 98) {
                mainUI.ClearConsole();
                mainUI.ConsolePrintWithWait("You open the container",0,0);
                Container* c = (Container*) player->GetFromInventory(loc_x, loc_y);

                AccessContainer(c);

                mainUI.DrawInv(player->GetInventory());
            }
        } else if (input == "help") {
            mainUI.ClearConsole();
            mainUI.ConsolePrintWithWait("what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container",0,0);
            
        } else {
            mainUI.ClearConsole();
            mainUI.ConsolePrintWithWait("Not a correct selection, try again.", 0, 0);
        };

        //std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (0);
};

void PlayerUI::TileProc(int y, int x, tile t)
{

    if (t == od0 || (t == od1) || (t == od2)) {
        DoorProc(y, x, t);
        return;
    } else if (t == cd0 || (t == cd1) || (t == cd2) || (t == ld1) || (t == ld2)) {
        DoorProc(y, x, t);
        return;
    } else if (t == ent) {
        mainUI.ClearConsole();
        mainUI.ConsolePrintWithWait("The way you came in is locked..", 0, 0);
        return;
    } else if (t == ext) {
         mainUI.ClearConsole();
        mainUI.ConsolePrintWithWait("You have reached the exit!", 0, 0);
        return;
    } else if (t == ded) {
        mainUI.ClearConsole();
        mainUI.ConsolePrintWithWait("The floor caves in below one of your feet, injuring you..", 0, 0);
        player->SetHealth(player->GetHealth() - 20);
        return;
    };

}


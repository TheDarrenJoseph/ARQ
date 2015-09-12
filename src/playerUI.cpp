#include "playerUI.h"

int PlayerUI::BattleTurn(int npc_id)
{
    werase(consolewin_front);

    for (int i = 0; i < 3; i++) {
        item* weps = player->GetWeps();
        string s = weps[i].name;
        wprint_at(consolewin_front, s.c_str(), 0, 9 * i);
    }

    wprint_at(consolewin_front, "Flee", 0, 9 * 3);

    ////input
    int index = 0;
    int scr_x = 9;
    int scr_y = 0;

    bool selection = true;

    while (selection == true) {
        scr_x = index * 9; //calculate the start of the item name

        mvwchgat(consolewin_front, scr_y, 0, 27, A_NORMAL, 0, NULL); //remove fancy effects
        mvwchgat(consolewin_front, scr_y, scr_x, 9, A_BLINK, 1, NULL); //add red blink to the current item
        wrefresh(consolewin_front);

        wprint_at(consolewin_front, "Use A and D to choose a weapon, 'use' to select", 1, 0);
        wprint_at(consolewin_front, "ARQ:~$ ", 2, 0); //give us a prompt

        string choice;
        char input[20];

        wgetstr(consolewin_front, input); //store our grabbed chars
        choice = input; //store the chars as a string for comparison

        if ((choice == "a") && (index > 0)) {
            index--;
        } else if ((choice == "d") && (index < 3)) {
            index++;
        } else if (choice == "use") {
            werase(consolewin_front);
            wprintw(consolewin_front, "Weapon selected.");
            return index;
        }

    }

    return 0;
}

int PlayerUI::DoorProc(int y, int x, tile doortype)
{
    int map_tile = map->GetTile(x, y);

    string door_name = tile_library[map_tile].name;

    if ((doortype == od0) || (doortype == od1) || (doortype == od2)) {
        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "You enter the doorway of the %s.. ", door_name.c_str());
        wgetch(consolewin_front);
        player->SetPos(x, y);

        return (0);
    } else if ((doortype == cd0) || (doortype == cd1) || (doortype == cd2)) {
        string answer;
        char answerchar[20];
        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "Would you like to open the %s? ", door_name.c_str());
        wgetstr(consolewin_front, answerchar);
        answer = (answerchar);

        if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y")) {
            wmove(consolewin_front, 0, 0);
            wprintw(consolewin_front, "You open the %s and step into the doorway. ", door_name.c_str());
            wgetch(consolewin_front);


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
            werase(consolewin_front);

            wmove(consolewin_front, 0, 0);
            wprintw(consolewin_front, "You leave the %s untouched. ", door_name.c_str());

            wgetch(consolewin_front);
            return (0);
        } else {
            werase(consolewin_front);

            wprint_at(consolewin_front, "Not a yes or no answer, try again..", 0, 0);
            wgetch(consolewin_front);

            DoorProc(y, x, map->GetTile(x, y));
        };

        return (0);
    } else if ((doortype == ld1) || (doortype == ld2)) {
        string answer;
        char answerchar[20];

        werase(consolewin_front);

        wprint_at(consolewin_front, "Would you like to open the door? ", 0, 0);
        wgetstr(consolewin_front, answerchar);

        answer = (answerchar);

        if ((answer == "Yes") || (answer == "YES") || (answer == "yes") || (answer == "y") || (answer == "Y")) {
            LockProc(y, x, doortype, map_tile, door_name);
            return (0);
        } else if ((answer == "No") || (answer == "NO") || (answer == "no") || (answer == "n") || (answer == "N")) {
            wmove(consolewin_front, 0, 0);

            wprintw(consolewin_front, (const char *) "You leave the %s untouched. ", door_name.c_str());
            wgetch(consolewin_front);

            return (0);
        } else {
            wprint_at(consolewin_front, (const char *) "Not a yes or no answer, try again..", 0, 0);

            DoorProc(y, x, map->GetTile(x, y));
        };
        return (0);
    };

    return (0);
}


//container check
//switch (map->AreaProc(x, y)) {
//case 1:
//{
//    //grab the item from the current area
//    item* i = map->GetItem(x, y);
//
//    ItemProc(i, x, y);
//    break;
//}
//
//case 2:
//{
//    area* thisArea = map->GetArea(x, y);
//    wprintw(consolewin_front, "Accessing %s at %d, %d", thisArea->name.c_str(), x, y);
//    wgetch(consolewin_front);
//
//    AccessArea(thisArea);
//    break;
//}
//}
//
//};

/*Wrapper for the Player Move() method, making use of return values for UI context*/
void PlayerUI::PlayerMoveTurn(int x, int y)
{
    int eid;

    if ((x < 0) | (y < 0) | (x > GRID_X) | (y > GRID_Y)) {
        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "There's a large Granite wall here!");

        wgetch(consolewin_front);
        return;
    }

    std::string move_tilename = tile_library [map->GetTile(x, y)].name;
    string enemy_name = npcs[eid].GetName();

    //main movement check
    if (map->IsTraversable(x, y)) {

        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "You walk along the %s.. ", move_tilename.c_str());

        wgetch(consolewin_front);

        //player->SetPos(x,y);  
    } else {
        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "There's a %s here.", move_tilename.c_str());

        wgetch(consolewin_front);

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
        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "You are confronted by a %s! ", enemy_name.c_str());
        wgetch(consolewin_front);

        Battle(eid);
        break;

        //Dead body
    case 4:
        string enemy_name = npcs[eid].GetName();

        wmove(consolewin_front, 0, 0);
        wprintw(consolewin_front, "There is the corpse of a %s here..", enemy_name.c_str());

        wgetch(consolewin_front);
        break;

    }
}

bool PlayerUI::TextInput()
{

    echo();

    string answer;
    char answerchar[20];

    werase(consolewin_front);

    wmove(consolewin_front, 0, 0); //moves the cursor to the window choice inputted into the function, +1 either side to avoid borders
    wprintw(consolewin_front, "ARQ:~$ ");
    wgetstr(consolewin_front, answerchar);

    answer = answerchar;

    if (answer == "help") {
        wprint_at(consolewin_front, (const char *) "phelp - player help", 0, 0);
        wprint_at(consolewin_front, (const char *) "ihelp - interactions", 1, 0);
        wprint_at(consolewin_front, (const char *) "info - game info", 2, 0);
        wgetch(consolewin_front);
    } else if (answer == "phelp") {
        wgetch(consolewin_front);
    } else if (answer == "ihelp") {
        wprint_at(consolewin_front, (const char *) "drop - drop item (selection)", 0, 0);
        wgetch(consolewin_front);
    } else if (answer == "info") {
        Info();
    } else if ((answer == "exit") || (answer == "Exit") || (answer == "EXIT") || (answer == "quit") || (answer == "Quit") || (answer == "QUIT")) {
        wprint_at(consolewin_front, (const char *) "Quitting.. ", 0, 0);
        wgetch(consolewin_front);

        return false;
    } else if (answer == "inventory") AccessInventory();

    else {
        wprint_at(consolewin_front, (const char *) "unrecognised input, please input a command, or use 'help' for a list. ", 0, 0);
        wgetch(consolewin_front);

        Input();
    }

    return true;
}

void PlayerUI::ShowControls()
{
    wprint_at(consolewin_front, "test", 0, 0);
}

bool PlayerUI::Input()
{
    int x;
    int y;
    int thisX;
    int thisY;

    ShowControls();

    keypad(consolewin_front, TRUE);
    noecho(); //stops input printing on screen

    player->GetPos(&x, &y); /*Get current position for movement*/

    thisX = x; //set movement positions to default
    thisY = y;

    int choice = wgetch(consolewin_front);

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

void PlayerUI::DrawNPCS() {
    for (int a = 0; a < MAX_NPCS; a++) {
        int x, y, colour;
        char symbol;

        if (&npcs[a] != NULL) {
            npcs[a].GetPos(&x, &y);

            colour = npcs[a].GetColour();
            symbol = npcs[a].GetSymbol();

            DrawCharacter(x, y, colour, symbol);
        };
    }

}

void PlayerUI::DrawPlayer() {
    int x, y, colour;
    char symbol;

    player->GetPos(&x, &y);

    colour = player->GetColour();
    symbol = player->GetSymbol();

    DrawCharacter(x, y, colour, symbol);
}

void PlayerUI::ShowNotification(const char* text)
{
    WINDOW* nWinRear = newwin(UI_Y + 2, UI_X + 2, 1, 1);
    WINDOW* nWin = newwin(UI_Y, UI_X, 2, 2);

    box(nWinRear, 0, 0);
    wrefresh(nWinRear);

    wprint_at(nWin, text, 5, 5);
    wprint_at(nWin, "[Enter]", UI_Y - 1, UI_X - 12);


    wrefresh(nWin);

    wgetch(nWin);

}

/** Displays a simple vertical menu of choices and returns the number of the chosen element */
int PlayerUI::Menu(const char** text, int buttons){

    WINDOW* nWinRear = newwin(UI_Y + 2, UI_X + 2, 1, 1);
    WINDOW* nWin = newwin(UI_Y, UI_X, 2, 2);

    keypad(nWin, TRUE);
    noecho(); //stops input printing on screen

    box(nWinRear, 0, 0);
    wrefresh(nWinRear);

    werase(nWin);

    int scr_x = UI_X / 2 - 8; //ui size/2 - 8 (8 being maximum letters displayed)

    int index = 0;

    for (int i = 0; i < buttons; i++) {
        wprint_at(nWin, text[i], i, scr_x);
    }

    while (true) {
        wchgat(nWin, 12, A_NORMAL, 0, NULL);

        mvwchgat(nWin, index, scr_x, 9, A_NORMAL, 1, NULL); //add red blink to the current item
        wrefresh(nWin);

        int choice = wgetch(nWin);

        if ((choice == KEY_UP) && (index > 0)) {
            index--;
        } else if ((choice == KEY_DOWN) && (index < buttons - 1)) {
            index++;
        } else if (choice == KEY_ENTER) {
            return index;
        }

    }

    return 0;

}

int PlayerUI::AccessArea(area * a){
    std::string slc_string;
    char slc_char[20];

    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    werase(consolewin_front);
    wprintw(consolewin_front, "Accessing Area");
    wgetch(consolewin_front);

    while (selection == true) {
        item* thisItem = a->GetItem(loc_x, loc_y);

        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                WINDOW* thisWin = invwins_front[y][x];

                mvwchgat(thisWin, 0, 0, 9, A_NORMAL, 0, NULL);

            }
        }

        //area b = *a;
        DrawInv(a);
        UpdateUI(); //ensures items display propely

        WINDOW* selWin = invwins_front[loc_y][loc_x]; //create a holder for the currently selected window

        mvwchgat(selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
        wrefresh(selWin);

        werase(consolewin_front);
        wprint_at(consolewin_front, "Select an item with WASD.. 'help' for commands", 0, 0);

        wprint_at(consolewin_front, "ARQ:~$ ", 2, 0);

        wgetstr(consolewin_front, slc_char);
        slc_string = slc_char;

        if ((slc_string == "W") || (slc_string == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((slc_string == "A") || (slc_string == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((slc_string == "S") || (slc_string == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((slc_string == "D") || (slc_string == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if (slc_string == "close") {
            selection = false;

            return (0);
        } else if (slc_string == "what") {
            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);

            wprintw(consolewin_front, "This is a %s.", thisItem->name.c_str());
            wrefresh(consolewin_front);

            wgetch(consolewin_front);
        } else if (slc_string == "take") {
            if (IsLootable(thisItem)) {
                player->AddToInventory(thisItem);
                a->RemoveItem(loc_x, loc_y);

                werase(consolewin_front);
                wprintw(consolewin_front, "You put the %s into your inventory.", thisItem->name.c_str());
                wgetch(consolewin_front);
            }
        } else if (slc_string == "put") {
            item* thisItem = player->GetFromInventory(loc_y, loc_x);

            werase(consolewin_front);
            wprintw(consolewin_front, "You put the %s inside.", thisItem->name.c_str());
            wrefresh(consolewin_front);

            wgetch(consolewin_front);

            if (thisItem != NULL) {
                a->AddItem(thisItem);
            }
        } else if (slc_string == "help") {
            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);

            wprintw(consolewin_front, "what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container");
            wrefresh(consolewin_front);

            wgetch(consolewin_front);
        } else {
            werase(consolewin_front);
            wprint_at(consolewin_front, "Not a correct selection, try again.", 0, 0);
            wgetch(consolewin_front);
        };

        std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (0);
};

int PlayerUI::AccessContainer(container * c){
    std::string slc_string;
    char slc_char[20];

    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    werase(consolewin_front);
    wprintw(consolewin_front, "Accessing Container");
    wgetch(consolewin_front);

    while (selection == true) {
        item* thisItem = c->GetItem(loc_x, loc_y);

        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                WINDOW* thisWin = invwins_front[y][x];

                mvwchgat(thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
                wrefresh(thisWin);
            }
        }

        DrawInv(c); //draw the current items
        UpdateUI(); //call a full system UI refresh to fix display bug

        WINDOW* selWin = invwins_front[loc_y][loc_x]; //create a holder for the currently selected window

        mvwchgat(selWin, 0, 0, 9, A_BLINK, 1, NULL); //make the current window blink red
        wrefresh(selWin);

        werase(consolewin_front);
        wprint_at(consolewin_front, "Select an item with WASD.. 'help' for commands", 0, 0);

        wprint_at(consolewin_front, "ARQ:~$ ", 2, 0);

        wgetstr(consolewin_front, slc_char);
        slc_string = slc_char;

        if ((slc_string == "W") || (slc_string == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((slc_string == "A") || (slc_string == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((slc_string == "S") || (slc_string == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((slc_string == "D") || (slc_string == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if (slc_string == "close") {
            selection = false;
            return (0);
        } else if (slc_string == "what") {
            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);

            wprintw(consolewin_front, "This is a %s.", thisItem->name.c_str());
            wrefresh(consolewin_front);

            wgetch(consolewin_front);
        } else if (slc_string == "take") {
            player->AddToInventory(thisItem);
            c->RemoveItem(loc_x, loc_y);

            werase(consolewin_front);
            wprintw(consolewin_front, "You put the %s into your inventory.", thisItem->name.c_str());
            wgetch(consolewin_front);
        } else if (slc_string == "put") {
            item* thisItem = player->GetFromInventory(loc_x, loc_y);

            if (thisItem != NULL) {
                c->AddItem(thisItem);
            }
        } else if (slc_string == "help") {
            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);

            wprintw(consolewin_front, "what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container");
            wrefresh(consolewin_front);

            wgetch(consolewin_front);
        } else {
            werase(consolewin_front);
            wprint_at(consolewin_front, "Not a correct selection, try again.", 0, 0);
            wgetch(consolewin_front);
        };

        std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (0);
};

int PlayerUI::AccessInventory(){
    item* inv_tile;
    int invtile_colour;

    const char* thisChar;
    std::string slc_string;
    char slc_char[20];

    int loc_x = 0;
    int loc_y = 0;

    //IF INV IS EMPTY, DO NOT ALLOW SELECTION?
    bool selection = true;

    while (selection == true) {
        item* thisItem = player->GetFromInventory(loc_x, loc_y);

        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                WINDOW* thisWin = invwins_front[y][x];

                mvwchgat(thisWin, 0, 0, 9, A_NORMAL, 0, NULL);
                wrefresh(thisWin);
            }
        }

        WINDOW* selWin = invwins_front[loc_y][loc_x]; //create a holder for the currently selected window

        mvwchgat(selWin, 0, 0, 9, A_BLINK, 2, NULL); //make the current window blink red
        wrefresh(selWin);

        //DrawInv(a);
        UpdateUI();

        werase(consolewin_front);
        wprint_at(consolewin_front, "Select an item with WASD.. 'help' for commands.", 0, 0);

        wprint_at(consolewin_front, "ARQ:~$ ", 2, 0);

        wgetstr(consolewin_front, slc_char);
        slc_string = slc_char;

        if ((slc_string == "W") || (slc_string == "w")) {
            if (loc_x != 0) {
                loc_x--;
            };
        } else if ((slc_string == "A") || (slc_string == "a")) {
            if (loc_y != 0) {
                loc_y--;
            };
        } else if ((slc_string == "S") || (slc_string == "s")) {
            if (loc_x != 2) {
                loc_x++;
            };
        } else if ((slc_string == "D") || (slc_string == "d")) {
            if (loc_y != 2) {
                loc_y++;
            }
        } else if (slc_string == "close") {
            selection = false;
            return (0);
        } else if (slc_string == "what") {
            wmove(consolewin_front, 0, 0);
            werase(consolewin_front);

            wprintw(consolewin_front, "This is a %s.", thisItem->name.c_str());
            wrefresh(consolewin_front);

            wgetch(consolewin_front);
        } else if (slc_string == "drop") {
            werase(consolewin_front);

            item* thisItem = player->GetFromInventory(loc_x, loc_y);

            if (CanDropItem(thisItem)) {

                if (map->DropPlayerItem(player, thisItem, loc_x, loc_y) == 0) {
                    //update the current item details 
                    const char* invtile_char = thisItem->name.c_str();

                    WINDOW* thisWin = invwins_front[loc_x][loc_y];

                    //update the inventory tile
                    wmove(thisWin, 0, 0);
                    wprintw_col(thisWin, invtile_char, thisItem->colour);
                    wrefresh(thisWin);

                    wprintw(consolewin_front, "Item dropped.");
                } else {
                    wprintw(consolewin_front, "Cannot drop that here.");
                }
            } else {
                wprintw(consolewin_front, "Cannot drop this item.");
            };

            wgetch(consolewin_front);
        } else if (slc_string == "wear") {
            if (IsLootable(player->GetFromInventory(loc_x, loc_y))) {
                inv_tile = player->GetInventory()->GetItem(loc_x, loc_y); //grab the current inventory tile

                invtile_colour = inv_tile->colour;
                thisChar = inv_tile->symbol;

                WINDOW* thisWin = invwins_front[loc_x][loc_y];

                const char *invtile_char = thisChar;

                wmove(thisWin, 0, 0);
                wprintw_col(thisWin, invtile_char, invtile_colour);
                wrefresh(thisWin);

                for (int i = 0; i < outfit_size; i++) {
                    werase(consolewin_front);
                    wprintw(consolewin_front, "%s selected.", thisChar);
                    wgetch(consolewin_front);

                    //if this item matches an outfit, assume it is and equip it
                    if (thisChar == outfit_library[i].symbol) {
                        //store the old outfit
                        outfit oldOutfit = player->GetOutfit();

                        wprintw(consolewin_front, "You change from %s into %s", player->GetOutfit().name.c_str(), thisChar);
                        wgetch(consolewin_front);

                        //change into the new outfit
                        player->SetOutfit(outfit_library[i]);


                        wprintw(consolewin_front, "You put your old %s into your inventory", oldOutfit.name.c_str());
                        wgetch(consolewin_front);

                        //set the current inventory tile to the old outfit
                        player->SetInventoryTile(loc_x, loc_y, new outfit(oldOutfit)); //instanciates a new outfit to fix polymorphism issues


                    }
                }


            } else {
                werase(consolewin_front);
                wprintw(consolewin_front, "No item selected.");
                wgetch(consolewin_front);
            };

        } else if (slc_string == "open") {
            int thisId = player->GetFromInventory(loc_x, loc_y)->id;

            //98 denotes container, which is a closed-ended area, we never want to access an area here
            if (thisId == 98) {
                wprintNoRefresh(consolewin_front, "You open the container");
                container* c = (container*) player->GetFromInventory(loc_x, loc_y);

                AccessContainer(c);

                DrawInv(player->GetInventory());
            }
        } else if (slc_string == "help") {
            wprintNoRefresh(consolewin_front, "what - display item name\ndrop - drop item\nwear - wear outfit\nclose - close this container");
            wrefresh(consolewin_front);

            wgetch(consolewin_front);
        } else {
            werase(consolewin_front);
            wprint_at(consolewin_front, "Not a correct selection, try again.", 0, 0);
            wgetch(consolewin_front);
        };

        std::cout << "\n " << loc_x << " " << loc_y;
    };

    return (0);
};

void PlayerUI :: TileProc(int y, int x,tile t)
{

  if (t == od0 || (t == od1) || (t == od2)) 
    {
      DoorProc (y, x, t);
      return;
    }
  
  else if (t == cd0 || (t == cd1) || (t == cd2) || (t == ld1) || (t == ld2)) 
    {
      DoorProc (y, x, t);
      return;
    }
  
  else if (t == ent) 
    {
      wprint_at (consolewin_front,(const char *)"The way you came in is locked..",0,0);
      wgetch (consolewin_front);
   
      return;
    }
 
  else if (t == ext) 
    {
      wprint_at (consolewin_front,(const char *)"You have reached the exit!",0,0);
      wgetch (consolewin_front);
   
      return;
    }
 
  else if (t == ded) 
    {
      wprint_at (consolewin_front,(const char *)"The floor caves in below one of your feet, injuring you..",0,0);
      wgetch (consolewin_front);
   
      player->SetHealth(player->GetHealth()-20);
  
      return;
    };
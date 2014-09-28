#ifndef UI_H
#define UI_H

#include "characters.h"

void InitWindows();
void DestroyWindows();
void DecorateWindows();

void DrawItems();
void DrawContainers();

void DrawPlayerInv(Player* p);
void DrawPlayerEquip(Player* p);
void DrawPlayerStats (Player* p);

void DrawInv(container* c);

int DrawMap ();
void DrawCharacter (int x, int y, int colour, char symbol);


void Info();

void UpdateUI();

void GetPlayerInput(Player* p);

int wprintw_col (WINDOW* winchoice, const char *text, int color_choice);
int wprint_at (WINDOW* winchoice, const char *text, int pos_y, int pos_x);

int init_screen ();
#endif

ASCII Rougelike Quester -- Experimental project [C++, Ncurses]
 
A small learner project/tech demo using C++ and Ncurses. ARQ is essentially a very basic rougelike being used to understand and learn new coding and appplication development techniques.

--LINUX BUILD-----------------------------------------------------------
Author: Rave Kutsuu   
Version : 0.89                              
Created: Dec 16, 2012
Last Modified: Aug 10, 2014                
------------------------------------------------------------------------

--TODO------------------------------------------------------------------
2. Equipment (Implementation of weapons)
3. Health System (Player Death, NPCs become lootable)
4. NPCs (improved spawning, AI, bosses)
5. Items (potions, scrolls, etc)
6. Level Progression (Break down doors, disarm/dodge traps, Multiple Levels,etc)
7. Full ending (Dungeon escape?)
8. Menu
9. Load game/item maps from files
10. Multiple dungeon levels

--Extras (Windows, Fog of War, Menu, Sound? (Ambient music)

--Cleanup needed (Break things into headers first)
//OLD STYLE/NESTED IFS
-Lockproc()
-DoorProc()

//RUSHED/OVERSIZED
-AccessInventory()

--Changelist (0.89)-----------------------------------------------------
--GAMEPLAY--
1.Added lootable containers/dead bodies
2.Added the ability to put player inv items into containers
3.Characters drop their equipment on death

--CODE--
1.Added containers
2.Reworked inventory system to work using containers
3.Added a secondary AcessInv function specifically for the player's inv
4.Further improvements to player movement
5.Changed indent style to GNU standard.
6.Cleaned up UI code/fixed refresh issues

--Changelist (0.88)-----------------------------------------------------
//GAMEPLAY
1. Added the ability to Flee when in combat

//CODE
1. Improved player Move(WINDOW * winchoice, WINDOW* mainwin, int y, int x)
2. Added Traversable Tile check and TileProc() (Abstracted from Move())
3. SPLIT CODE INTO HEADERS AND MULTIPLE SOURCE FILES
4. REWORKED MAJOR FUNCTIONS TO SUPPORT THE ABOVE

--Changelist (0.87)-----------------------------------------------------
//GAMEPLAY
1.Fixed bug with NPCs not dying
2.Added corpses/modified NPC symbol
3.Small improvements to movement feedback
4.Reworked inventory layout to fix item name bug
5.Fixed an item pickup bug caused by 0.85-2
6.Added equipment window
7.Added outfits/armour

//CODE
1.Updated item system enums and listings 
2.Fixed bad naming conventions
3.Cleaned up main() and added init functions
4.Gave WINDOW*'s global scope

--Changelist (0.86)-----------------------------------------------------
1.Fixed item pickup bug

--Changelist (0.85)-----------------------------------------------------
1.Improved documentation
2.Ablity to walk over items
3.Fixed inventory to work with the new item system
4.Added the ability to fight/kill NPCs
5.Fixed death

--CREDITS---------------------------------------------------------------
1. The Beginner's Guide to Roguelike Development in C/C++ -- 
http://www.kathekonta.com/rlguide/index.html 

2. NCURSES Programming HOWTO --  
http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/                      

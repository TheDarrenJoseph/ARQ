ASCII Rougelike Quester -- Experimental project [C++, Ncurses]
 
A small learner project/tech demo using C++ and Ncurses. ARQ is essentially a very basic rougelike being used to understand and learn new coding and appplication development techniques.

--LINUX BUILD--
Author: Rave Kutsuu   
Version : 0.88                              
Created: Dec 16, 2012
Last Modified: 31 July, 2014                

--Changelist (0.88)--

--Changelist (0.87)--
//GAMEPLAY
1.Fixed bug with NPCs not dying
2.Added corpses/modified NPC symbol
3.Small improvements to movement feedback
4.Reworked inventory layout to fix item name bug
5.Fixed an item pickup bug caused by 0.85-2
6.Added equipment window
7.Added outfits

//CODE
1.Updated item system enums and listings 
2.Fixed bad naming conventions
3.Cleaned up main() and added init functions
4.Gave WINDOW*'s global scope

--Changelist (0.86)--
1.Fixed item pickup bug

--Changelist (0.85)--
1.Improved documentation
2.Ablity to walk over items
3.Fixed inventory to work with the new item system
4.Added the ability to fight/kill NPCs
5.Fixed death

--TODO--
1. Fighting (Finish Player/NPC conflicts)
2. Equipment (Implementation of weapons (armour? Could buff HP?))
2. Health System (Death, etc)
3. NPCs (improved spawning, AI, bosses)
4. Items (lootable corpses, weapons, potions, scrolls, etc)
5. Level Progression (Break down doors, disarm/dodge traps, etc)

6. Full ending (Dungeon escape?)
7. Menu

8. Load game/item maps from files
9. Multiple dungeon levels

--Extras (Windows, Fog of War, Menu, Sound? (Ambient music)

--Cleanup needed (Break things into headers first)
//OLD STYLE/NESTED IFS
-Lockproc()
-DoorProc()

//RUSHED/OVERSIZED
-AccessInventory()

--CREDITS-- 
1. The Beginner's Guide to Roguelike Development in C/C++ -- http://www.kathekonta.com/rlguide/index.html 
2. NCURSES Programming HOWTO --  http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/                      

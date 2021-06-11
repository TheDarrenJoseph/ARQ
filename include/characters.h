#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <string>

#define INV_SIZE 3

#include "tile.h"
#include "items.h"
#include "containers.h"
#include "position.h"

class Character
{
 protected:
  Container* inventory = NULL;
  int posX = 0;
  int posY = 0;
  
  char symbol = ' ';
  int colour  = 0;
  std::string name = "";
  int health = 0;
  int max_health = 0;
  
  bool alive = true;

  const weapon* weps [3];
  outfit currentOutfit = outfit();
  
 public:
  
  bool IsAlive();

  void Kill();
  
  bool IsTraversable(tile t);
  bool CanFlee();
  int Flee();
  
  void Draw();

  void SetCharacter(char char_choice, int colour_choice, std::string name_choice, int health_choice);

  void GetPos (int *x, int *y);
  void SetPos(int x, int y);
  Position GetPosition();

  int GetColour();
  char GetSymbol();

  std::string GetName();
  int GetMaxHealth();
  int GetHealth ();
  
  void AddToInventory(const Item* i);
  void SetInventory(int index, const Item* i);

  Container* GetInventory();
  const Item* GetFromInventory(int index);

  void SetWeps(weaponIndex one, weaponIndex two, weaponIndex three);
  const weapon* GetWeps(); //returns a pointer to a 1D array of weapons
  
  void SetOutfit(outfit o);
  outfit GetOutfit();
  
  void SetHealth(int);

  Container* DropItems(); //Drops the players inv on death as a dead body
   
  Character (char charSymbol, int col, std::string name, int health) 
    {
      alive = true;
      health=0;
      posX=0;
      posY=0;
           
      max_health = health;
      SetCharacter(charSymbol,col,name,health);
   	
      for (int i=0; i<3; i++){
          weps[i] = &weapon_library[no_weapon];
      }
   	 
      SetOutfit(outfit_library[no_outfit]);

      this->inventory = new Container(0, OBJECT, name + "'s Inventory","X",2,0,100,0,true);
    };
    
  void copyCharacter(const Character& c) {  
      alive =c.alive;
      health=c.health;
      this->posX = c.posX;
      this->posY = c.posY;
      
      max_health = c.max_health;
      
      //assigning pointers
      for (int i=0; i<3; i++) {
        weps[i] = c.weps[i];
      }
   	 
      this->currentOutfit = c.currentOutfit;
      this->inventory = new Container(c.inventory);
  }
    
  //Override assignment operator
  Character& operator=(const Character& c) {  
      copyCharacter(c);
      
      return *this;
  }
    
  //Copy constructor  
  Character(const Character& c) {  
      copyCharacter(c);
  }
    
  virtual ~Character() {  
  }
};

class NPC : public Character
{
 private:
   // NPC* npcs;
    
 public :
  int BattleTurn ();
  void Move(int* x, int* y); //returns chosen movement co-ordinates for the NPC

  NPC (char c, int col, std::string name, int health) : Character(c,col,name,health)
    {
      //this->SetPos(6,6);
    }; 
};

class Player : public Character
{
 private:
 // NPC* npcs;
  unsigned long int lootScore=0;
  
  public :
    void SetLoot(unsigned long int x);
    unsigned long int GetLootScore();

    int GetLootCount ();
    int GetKeyCount();
    void RemoveKeyCount(int keyCount);
    
    //int AreaProc (int x ,int y);
   
    Player (char c, int col, std::string name, int health) : Character(c,col,name,health)
      {
        //initialise the item inventory
        inventory = new Container(0, OBJECT, "Player's Inventory","X",2,0,100,0,true);
    
      for (long unsigned int i=0; i<INV_SIZE; i++)
        {
          //inventory->AddItem(new Item(item_library[2]));
              //  inventory->AddItem ((Item*) &item_library[2]);
              // Item itm = Item(98,"Shoop","X",2,2,0,true); //inventory testing
               // inventory->AddItem(&itm);
        }
    
      };
      
    Player& operator=(const Player& c) {  
        this->inventory = c.inventory;
        this->lootScore = c.lootScore;
        
        return *this;
    }
      
    //Copy constructor  
    Player(const Player& p) : Character(p.symbol,p.colour,p.name,p.health) {  
        
        this->inventory = p.inventory;
        this->lootScore = p.lootScore;
    }
      
    ~Player() {
        delete(inventory); //Calls the destructor for this container
    }
};

class Warrior : public Player
{
 public:
     
  Warrior () : Player('@',6,"Warrior",100)
    {
      SetWeps(no_weapon,fist,sword);
      SetOutfit(outfit_library[warrior]); 
    };
  
};
 
class Goblin : public NPC
{
 public:

 Goblin() : NPC ('@',2,"Wild Goblin",30)
    {
      SetWeps (no_weapon,claw,sword);
      SetOutfit(outfit_library[goblin]);
    };

 Goblin(char c, int col, std::string name, int health) : NPC (c,col,name,health)
    {
      SetWeps (no_weapon,claw,sword);
      SetOutfit(outfit_library[goblin]);
    };
  
};

class GoblinBeserker : public Goblin
{
 public:
  
    GoblinBeserker() : Goblin ('@',1,"Goblin Beserker",5) 
    {
      SetWeps (no_weapon,claw,axe);
    };
  
};

class Slime : public NPC
{
 public:
  Slime () : NPC ('*',1,"Oozy Slime",10)
    {
      SetWeps (no_weapon,acid,claw);
    };
};

void InitNpcs();
void DrawNPCS();

#endif

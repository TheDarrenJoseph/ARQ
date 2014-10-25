 #include "grid.h"

tile game_grid[grid_y][grid_x] = 
   { 
    {wa1,wa1,ent,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1},
    {wa1,cor,cor,cor,cor,cor,cor,cor,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,cd0,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,ded,wa2,wa2,cd0,cd0,wa2,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,rom,rom,rom,rom,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,rom,rom,rom,wa1},
    {wa1,cor,wa2,wa2,wa2,wa2,wa2,wa2,cor,wa2,wa2,wa2,cd0,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,cd0,wa2,wa2,wa2,wa2,wa1},
    {wa1,cor,cor,cor,cor,cor,cor,cor,cor,wa2,cor,cor,cor,ded,ded,ded,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,cor,wa1},
    {wa1,wa2,wa2,wa2,wa2,wa2,wa2,wa2,cd1,wa2,cor,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,wa2,cd0,wa2,wa2,wa2,cd0,wa2,wa2,wa2,wa1},
    {wa1,rom,ded,rom,rom,rom,ded,wa2,cor,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,rom,ded,rom,rom,rom,ded,wa2,cor,wa2,cor,wa2,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,ded,ded,ded,rom,ded,ded,wa2,cor,ld1,cor,wa2,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,ded,ded,rom,rom,rom,rom,rom,cor,ld1,cor,cd0,rom,rom,rom,rom,rom,rom,wa2,rom,rom,rom,rom,wa2,rom,rom,rom,rom,rom,wa1},
    {wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,wa1,ext,wa1,wa1,wa1},
   };

area* container_grid[grid_y][grid_x];

tile GetTile(int x, int y)
{ 
  return game_grid[y][x];
}

void SetTile(int x, int y, tile t)
{
  game_grid[y][x] = t;
};  

item* GetItem(int x, int y)
{
  return container_grid[y][x]->GetItem(0,0);
} 

item* GetItem(int x, int y,int i, int j)
{
  return container_grid[y][x]->GetItem(i,j);
} 

//Adds an item to the area
void AddToArea(int x, int y, item* i)
{
  container_grid[y][x]->AddItem(i);
}

area* GetArea(int x,int y)
{
  return container_grid[y][x];
}

void SetArea(int x,int y, area* a)
{
  container_grid[y][x] = a;
} 

void InitAreas()
{
  for (int x=0; x<grid_x; x++)
    {
      for (int y=0; y<grid_y; y++)
	{
	  container_grid[y][x] = new area(99,"Empty"," ",0,0,false);
	}
    }
}
/*
void DrawInv(area* a)
{
  for (int y = 0; y<3; y++)
    {
      //draw each item on this row to the screen
      for(int x = 0; x<3; x++)
	{	 
	  WINDOW* thisWindow = invwins_front[y][x];
         
	  werase(thisWindow);
	  wmove(thisWindow,0,0); 
      	 
	  item* itm = a->GetItem(x,y);      
       
	  //int colour = itm->colour;
	  std::string str = itm->name;
         
	  const char* symbol = str.c_str();

	  wprintw_col (thisWindow, symbol, 0);
          wrefresh(thisWindow);
	}; 
    };  
  return;
}
*/

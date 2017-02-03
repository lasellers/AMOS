// ALife.cpp: implementation of the ALife class.
//
//////////////////////////////////////////////////////////////////////

/*
A-MOS
Lewis A. Sellers
intrafoundation.com
March 2003

  A 2-day experiment with A-Life.
*/

#include "stdafx.h"
#include "stdlib.h" //for rand
#include <time.h> //for time for srand

#include "ALife.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ALife::ALife()
{
	srand( (unsigned)time( NULL ) ); //seed randomize
	
	number_of_tribes=MAX_DEFAULT_TRIBES; //6 is the default and maximum
	
	wraparound=true;
	migration=MIGRATION_OUTWARD;
	birth_food=BIRTH_FOOD;
	allow_war=true;
	allow_rebellion=true;
	allow_cities=true;
	allow_carrying=true;
	allow_wastelands=true;
	
	foodX=0;
	foodY=0;
	
	visualmultiplier256=16;
	scan_rotation=0;
	
	// for export
	width=WIDTH;
	height=HEIGHT;
	
	Reset();
}


ALife::~ALife()
{
	
}

//
int ALife::Reset()
{
	int y,x;
	int t;
	
	//
	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			AWorld[y][x].resources=STARTING_RESOURCES;
			AWorld[y][x].food=STARTING_FOOD;
			AWorld[y][x].tribe=NO_TRIBE;
			AWorld[y][x].turnover=false;
			if(migration==MIGRATION_OFFPATH || migration==MIGRATION_PATH)
				AWorld[y][x].direction=(short)(8.0L*((double)rand()/(double)RAND_MAX)); //abs(x+y+count)&7; //(count++)&7 //(x+(y*WIDTH))&7;
			else
				AWorld[y][x].direction=0;
		}
	}
	
	// set the migration weighting
	for(t=0;t<number_of_tribes;t++) 
	{
		Tribes[t].population=1;
		Tribes[t].migrationX=int(width/2);
		Tribes[t].migrationY=int(height/2);
		Tribes[t].death_event=false;
		Tribes[t].direction=0;
	}
	
	// make the direction moving table. this is simply an optimization for speed.
	DXY[0].x=0; DXY[0].y=-1;
	DXY[1].x=1; DXY[1].y=-1;
	DXY[2].x=1; DXY[2].y=0;
	DXY[3].x=1; DXY[3].y=1;
	DXY[4].x=0; DXY[4].y=1;
	DXY[5].x=-1; DXY[5].y=1;
	DXY[6].x=-1; DXY[6].y=0;
	DXY[7].x=-1; DXY[7].y=-1;
	
	DXY[8].x=0; DXY[8].y=0; //in case a NO_DIRECTION accidentally is passed for lookup
	
	//
	generation=0;
	births=number_of_tribes;
	deaths=0;
	fights=0;
	starvations=0;
	migration_decisions=0;
	total_food=0;
	total_resources=0;
	rebellions=0;
	cities=0;
	multiple_births[0]=0;
	multiple_births[1]=0;
	multiple_births[2]=0;
	
	// seed random tribe spots
	for(t=0;t<number_of_tribes;t++) 
	{
		int x=int(((double)rand()/(double)RAND_MAX)*WIDTH);
		int y=int(((double)rand()/(double)RAND_MAX)*HEIGHT);
		if(y>=HEIGHT) y=HEIGHT-1;
		if(x>=WIDTH) x=WIDTH-1;
		AWorld[y][x].tribe=t;
	}
	
	return 0;
}


//
inline void ALife::look_in_direction(int y, int x, int d, int& newy, int& newx)
{
	newy=y+DXY[d].y;
	newx=x+DXY[d].x;
	
	if(wraparound)
	{ //wraparound. can't cross square edge boundries.
		if(newy<0) newy=HEIGHT-1;
		else if(newy>=HEIGHT) newy=0;
		if(newx<0) newx=WIDTH-1;
		else if(newx>=WIDTH) newx=0;
	}
	else
	{ //no wrap around
		if(newy<0) newy=0;
		else if(newy>=HEIGHT) newy=HEIGHT-1;
		if(newx<0) newx=0;
		else if(newx>=WIDTH) newx=WIDTH-1;
	}
}

//
inline void ALife::look_at(int y, int x, int& newy, int& newx)
{
	if(wraparound==true)
	{ //wraparound. can't cross square edge boundries.
		if(y<0) newy=HEIGHT-1;
		else if(y>=HEIGHT) newy=0;
		else newy=y;
		if(x<0) newx=WIDTH-1;
		else if(x>=WIDTH) newx=0;
		else newx=x;
	}
	else
	{ //no wrap around
		if(y<0) newy=0;
		else if(y>=HEIGHT) newy=HEIGHT-1;
		else newy=y;
		if(x<0) newx=0;
		else if(x>=WIDTH) newx=WIDTH-1;
		else newx=x;
	}
}



//
int ALife::best_direction_to_move(int y, int x)
{
	int direction=NO_DIRECTION;
	
	if(AWorld[y][x].tribe!=NO_TRIBE)
	{
		if(migration==MIGRATION_NONE)
			direction=direction_based_on_local_conditions(y,x);
		else
		{
			direction=direction_to_migration_strategy_point(y,x);
			//if(direction==NO_DIRECTION && AWorld[y][x].food==MIN_FOOD)
			//	direction=direction_based_on_local_conditions(y,x);
		}

		if(direction!=NO_DIRECTION) AWorld[y][x].direction=direction;
	}

	return direction;
}

//
int ALife::direction_based_on_local_conditions(int y, int x)
{
	int direction=NO_DIRECTION;
	
	int tribe=AWorld[y][x].tribe;
	if(tribe!=NO_TRIBE)
	{
		int d;
		int newy, newx;
		
		// look for trodden open space
		int last_d=AWorld[y][x].direction;
		for(d=0;d<=7;d++)
		{
			look_in_direction(y,x,last_d,newy,newx);
			if(AWorld[newy][newx].tribe==NO_TRIBE)
				direction=last_d;
			last_d=(last_d+1)&7;
		}
		
		// if no empty space, look for land held for other tibes: WAR
		if(direction==NO_DIRECTION)
		{
			int last_d=AWorld[y][x].direction;
			for(d=0;d<=7;d++)
			{
				look_in_direction(y,x,last_d,newy,newx);
				if(AWorld[newy][newx].tribe!=tribe)
					direction=last_d;
				last_d=(last_d+1)&7;
			}
		}
	}
	
	//
	return direction;
}


//
inline int ALife::direction_to_migration_strategy_point(int y, int x)
{
	//
	int d=NO_DIRECTION;
	
	//
	int tribe=AWorld[y][x].tribe;
	if(tribe!=NO_TRIBE)
	{
		//
		migration_decisions++;
		
		//
		int dy, dx;
		
		// outward
		switch(migration)
		{
		case MIGRATION_OUTWARD:
		{
			dy=(int)((double)(y-Tribes[tribe].migrationY)/((double)height/2.0L));
			dx=(int)((double)(x-Tribes[tribe].migrationX)/((double)width/2.0L));
			
			for(d=0;d<=7;d++)
			{
				if(dx==DXY[d].x && dy==DXY[d].y) break;
			}
		}
		break;
		
		// inward
		case MIGRATION_INWARD:
		{
			dy=-(int)((double)(y-Tribes[tribe].migrationY)/((double)height/2.0L));
			dx=-(int)((double)(x-Tribes[tribe].migrationX)/((double)width/2.0L));
			
			for(d=0;d<=7;d++)
			{	
				if(dx==DXY[d].x && dy==DXY[d].y) break;
			}	
		}
		break;
		
		// aggression
		case MIGRATION_AGGRESSION:
		{
			int t=tribe;
			
			//
			int tx=width,ty=height;
			for(int n=0;n<number_of_tribes;n++)
			{
				if(Tribes[n].population>0)
				{
					dx=abs(Tribes[tribe].migrationX-Tribes[n].migrationX);
					dy=abs(Tribes[tribe].migrationY-Tribes[n].migrationY);
					if(dx<tx && dy<ty) // && dx<(width/3) && dy<(height/3))
					{ t=n; tx=dx; ty=dy; }
				}
			}
			
			//
			dy=(int)((double)(y-Tribes[t].migrationY)/((double)height/2.0L));
			dx=(int)((double)(x-Tribes[t].migrationX)/((double)width/2.0L));
			
			for(d=0;d<=7;d++)
			{	
				if(dx==DXY[d].x && dy==DXY[d].y) break;
			}	
		}
		break;
		
		// center
		case MIGRATION_CENTER:
		{
			int topy=(height>>1)-(height>>4);
			int bottomy=(height>>1)+(height>>4);
			if(y<topy)
				dy=1;
			else if(y>bottomy)
				dy=-1;
			else
				dy=0;

			int leftx=(width>>1)-(width>>4);
			int rightx=(width>>1)+(width>>4);
			if(x<leftx)
				dx=1;
			else if(x>rightx)
				dx=-1;
			else
				dx=0;
			
			for(d=0;d<=7;d++)
			{	
				if(dx==DXY[d].x && dy==DXY[d].y) break;
			}	
		}
		break;
		
		// helter skelter
		case MIGRATION_HELTERSKELTER:
		{
			d=Tribes[tribe].direction&7;
			Tribes[tribe].direction++;
		}
		break;
		
		// off path
		case MIGRATION_OFFPATH:
		{
			d=AWorld[y][x].direction;
			AWorld[y][x].direction=(AWorld[y][x].direction+1)&7;
		}
		break;
		
		// path
		case MIGRATION_PATH:
		{
			d=AWorld[y][x].direction;
		}
		break;
		
		// food
		case MIGRATION_FOOD:
		{
			//
			dy=(int)((double)(y-foodY)/((double)height/2.0L));
			dx=(int)((double)(x-foodX)/((double)width/2.0L));
			
			for(d=0;d<=7;d++)
			{	
				if(dx==DXY[d].x && dy==DXY[d].y) break;
			}	
		}
		break;
		
		// your routine 1
		case MIGRATION_YOUR_ROUTINE_1:
		{
			d=1;
		}
		break;
		
		// your routine 2
		case MIGRATION_YOUR_ROUTINE_2:
		{
			d=5;
		}
		break;
		}
		
		//don't move where someone else already is. SIMPLE WAY
		if(d!=NO_DIRECTION)
		{
			int newy, newx;
			look_in_direction(y,x,d,newy,newx);
			if(AWorld[newy][newx].tribe!=NO_TRIBE)
			{
				d=NO_DIRECTION;
			}
		}
		
	}
	//
	return d;
}

//
int ALife::best_direction_to_give_birth(int y, int x)
{
	int newy, newx;
	int direction=NO_DIRECTION;
	int food=0;
	
	// look for trodden open space
	int last_d=AWorld[y][x].direction;
	for(int d=0;d<=7;d++)
	{
		look_in_direction(y,x,last_d,newy,newx);
		if(AWorld[newy][newx].tribe==NO_TRIBE && AWorld[newy][newx].food>food)
		{
			direction=last_d;
			food=AWorld[newy][newx].food;
		}
		last_d=(last_d+1)&7;
	}

	if(direction!=NO_DIRECTION) AWorld[newy][newx].direction=direction;

	//
	return direction;
}

// migrate from current area to a new one with more(?) food.
void ALife::move_in(int y, int x, int direction)
{
	int newy, newx;
	look_in_direction(y,x,direction,newy,newx);
	
	if(AWorld[newy][newx].tribe==NO_TRIBE)
	{
		//
		AWorld[newy][newx].tribe=AWorld[y][x].tribe;
		AWorld[y][x].tribe=NO_TRIBE;
		
		// are they allowed to carry some food with them when they migrate?
		if(allow_carrying==true)
		{
			int food=AWorld[newy][newx].food+AWorld[y][x].food;
			if(food==MIN_FOOD)
				;
			else if(food<=MAX_FOOD)
			{
				AWorld[newy][newx].food=food-1;
				AWorld[y][x].food=1;
			}
			else
			{
				AWorld[newy][newx].food=MAX_FOOD;
				AWorld[y][x].food=food-MAX_FOOD;
			}
		}
		
		//
		AWorld[newy][newx].turnover=true;
	}
}

//
void ALife::give_birth(int y, int x, int direction)
{
	int newy, newx;
	look_in_direction(y,x,direction,newy,newx);
	
	if(AWorld[newy][newx].tribe==NO_TRIBE)
	{
		//
		int tribe=AWorld[y][x].tribe;
		if(tribe!=NO_TRIBE)
		{
			
			Tribes[tribe].population++;
			AWorld[newy][newx].tribe=AWorld[y][x].tribe;
			AWorld[y][x].food-=(birth_food>>1); //BIRTH_FOOD_COST;
			
			//
			AWorld[newy][newx].turnover=true;
			
			//
			births++;
		}
	}
}

//
void ALife::starve_to_death(int y, int x)
{
	int tribe=AWorld[y][x].tribe;
	if(tribe!=NO_TRIBE)
	{
		//
		Tribes[tribe].population--;
		
		AWorld[y][x].tribe=NO_TRIBE;
		AWorld[y][x].turnover=true; //
		
		//
		deaths++;
		starvations++;
		
		if(Tribes[tribe].population==0) Tribes[tribe].death_event=true; //flag for playsound
	}
}


//
int ALife::steal_food(int y, int x)
{
	int food=0;
	int newy, newx;
	
	for(int dy=-1;dy<=1;dy++)
	{
		for(int dx=-1;dx<=1;dx++)
		{
			look_at(y+dy,x+dx,newy,newx);
			if(AWorld[newy][newx].tribe==NO_TRIBE && AWorld[newy][newx].turnover==true)
			{
				if(AWorld[newy][newx].food>MIN_FOOD)
				{
					food+=AWorld[newy][newx].food;
					AWorld[newy][newx].food--;
				}
			}
		}
	}
	
	food=food/2; //we loose some food in the raids
	
	return food;
}

// if the plot of land our alife is on is out of resources they will
// rebel against their tribe.
void ALife::rebellion(int y, int x)
{
	if(allow_rebellion==true)
	{
		int tribe=AWorld[y][x].tribe;
		if(tribe!=NO_TRIBE)
		{
			for(int t=0;t<number_of_tribes;t++)
			{
				if(t!=tribe && Tribes[t].population>0)
				{
					//no we steal food from all surrounding us...
					AWorld[y][x].food+=(short)steal_food(y,x);
					
					//
					AWorld[y][x].tribe=t;
					Tribes[t].population++;
					Tribes[tribe].population--;
					
					//
					rebellions++;
					
					if(Tribes[tribe].population==0) Tribes[tribe].death_event=true; //flag for playsound
					
					break;
				}
			}
		}
	}
}


// is 7 out of 9 of the surroudning areas have the same tribe, we call it a city.
bool ALife::is_a_city(int y, int x)
{
	int tribe=AWorld[y][x].tribe;
	
	int pop=0;
	int newy, newx;
	
	for(int dy=-1;dy<=1;dy++)
	{
		for(int dx=-1;dx<=1;dx++)
		{
			look_at(y+dy,x+dx,newy,newx);
			if(AWorld[newy][newx].tribe==tribe) pop++;
		}
	}
	
	return (pop>=7);
}


// assume is_a_city is true.
void ALife::cooperative_effects_of_city(int y, int x)
{
	if(allow_cities==true)
	{
		if(AWorld[y][x].resources<MAX_RESOURCES)
			AWorld[y][x].resources++;
		cities++;
	}
}


//
void ALife::war_on(int y, int x, int direction)
{
	if(allow_war==true)
	{
		int newy, newx;
		look_in_direction(y,x,direction,newy,newx);
		
		if(AWorld[newy][newx].tribe==NO_TRIBE)
		{
			// if no tribe there, just move in
			move_in(y,x,direction);
		}
		else
		{
			int defender=AWorld[newy][newx].tribe;
			int aggressor=AWorld[y][x].tribe;
			
			//two competing tribes -- decide who wins
			if(AWorld[newy][newx].food>=AWorld[y][x].food && aggressor!=NO_TRIBE)
			{
				//defender wins
				Tribes[aggressor].population--;
				AWorld[y][x].tribe=NO_TRIBE;
				
				if(AWorld[y][x].resources>MIN_RESOURCES)
					AWorld[y][x].resources--; //devestation to land
				
				if(Tribes[aggressor].population==0) Tribes[aggressor].death_event=true; //flag for playsound
			}
			else if(defender!=NO_TRIBE)
			{
				//aggressor wins
				Tribes[defender].population--;
				AWorld[newy][newx].tribe=NO_TRIBE;
				
				if(AWorld[newy][newx].resources>MIN_RESOURCES)
					AWorld[newy][newx].resources--; //devestation to land
				
				if(Tribes[defender].population==0) Tribes[defender].death_event=true; //flag for playsound
			}
			
			//
			short food_cost_for_war=(short)abs(AWorld[newy][newx].food-AWorld[y][x].food);
			AWorld[newy][newx].food-=food_cost_for_war;
			AWorld[y][x].food-=food_cost_for_war;
			
			if(AWorld[newy][newx].food<MIN_FOOD) AWorld[newy][newx].food=MIN_FOOD;
			if(AWorld[y][x].food<MIN_FOOD) AWorld[y][x].food=MIN_FOOD;
			
			//
			deaths++;
			fights++;
			
			//
			AWorld[newy][newx].turnover=true;
		}
	}
}

//
void ALife::Disaster()
{
	int y,x;
	
	for(y=0;y<HEIGHT;y++)
	{
		for(x=0;x<WIDTH;x++)
		{
			if(AWorld[y][x].food>MIN_FOOD) AWorld[y][x].food--;
			if(AWorld[y][x].resources>MIN_RESOURCES) AWorld[y][x].resources--;
		}
	}
}

//
void ALife::Fortune()
{
	int y,x;
	
	for(y=0;y<HEIGHT;y++)
	{
		for(x=0;x<WIDTH;x++)
		{
			if(AWorld[y][x].food<MAX_FOOD) AWorld[y][x].food++;
			if(AWorld[y][x].resources<MAX_RESOURCES) AWorld[y][x].resources++;
		}
	}
}

//
void ALife::SuperFortune()
{
	int y,x;
	
	for(y=0;y<HEIGHT;y++)
	{
		for(x=0;x<WIDTH;x++)
		{
			AWorld[y][x].food=MAX_FOOD;
			AWorld[y][x].resources=MAX_RESOURCES;
		}
	}
}

//
void ALife::NextGeneration()
{
	//
	generation++;
	
	int y,x;
	int t;
	
	// PART A
	for(t=0;t<number_of_tribes;t++)
	{
		Tribes[t].migrationX=0;
		Tribes[t].migrationY=0;
	}
	total_food=0;
	total_resources=0;
	
	int foods=0;
	foodX=0; foodY=0;
	
	
	//
	for(y=0;y<HEIGHT;y++)
	{
		for(x=0;x<WIDTH;x++)
		{
			AWorld[y][x].turnover=false;
			total_food+=AWorld[y][x].food;
			total_resources+=AWorld[y][x].resources;
			
			// food grows when no one is trappling or eating it
			if(AWorld[y][x].tribe==NO_TRIBE &&
				AWorld[y][x].food<AWorld[y][x].resources &&
				AWorld[y][x].food<MAX_FOOD)
				AWorld[y][x].food++;  //let food grow slightly ...
			
			//
			int tribe=AWorld[y][x].tribe;
			if(tribe!=NO_TRIBE)
			{
				Tribes[tribe].migrationX+=x;
				Tribes[tribe].migrationY+=y;
			}
			
			//
			if(AWorld[y][x].tribe==NO_TRIBE && AWorld[y][x].food>=birth_food)
			{
				foodX+=x;
				foodY+=y;
				foods++;
			}
		}
	}
	//
	for(t=0;t<number_of_tribes;t++)
	{
		Tribes[t].migrationX=(int)((double)Tribes[t].migrationX/(double)(Tribes[t].population));
		Tribes[t].migrationY=(int)((double)Tribes[t].migrationY/(double)(Tribes[t].population));
	}
	//
	if(foods>0)
	{
		foodX=foodX/foods;
		foodY=foodY/foods;
	}
	
	
	// PART B
	// we do scan rotation so as not to overly bias the results in any one x/y plane direction 
	switch(scan_rotation)
	{
	case 0: for(y=0;y<HEIGHT;y++) { for(x=0;x<WIDTH;x++) NextGeneration_sub(y,x); } break;
	case 1: for(y=0;y<HEIGHT;y++) { for(x=WIDTH-1;x>=0;x--) NextGeneration_sub(y,x); } break;
	case 2: for(y=HEIGHT-1;y>=0;y--) { for(x=0;x<WIDTH;x++) NextGeneration_sub(y,x); } break;
	case 3: for(y=HEIGHT-1;y>=0;y--) { for(x=WIDTH-1;x>=0;x--) NextGeneration_sub(y,x); } break;
	};
	scan_rotation=(scan_rotation+1)&3;
}


//
void ALife::NextGeneration_sub(int y, int x)
{
	if(AWorld[y][x].turnover==false)
	{
		if(AWorld[y][x].tribe!=NO_TRIBE)
		{
			int direction;
			
			// starvation. are we out of food?
			if(AWorld[y][x].food==MIN_FOOD)
			{
				// by eating everything we can get ahold of
				// we cause some permanent damage to land
				if(allow_wastelands) //allow starvation to make land uninhabitable?
				{
					if(AWorld[y][x].resources>MIN_RESOURCES)
						AWorld[y][x].resources--; //over-grazing
				}
				else
				{
					if(AWorld[y][x].resources>MIN_RESOURCES+1)
						AWorld[y][x].resources--; //over-grazing
				}
				starve_to_death(y,x); //no more food, sob.
			}
			else
				AWorld[y][x].food--; //consume food

			// is there a coooperative gathering? call it a city...
			if(allow_cities==true)
			{
				if(is_a_city(y,x)==true)
					cooperative_effects_of_city(y,x);
			}
			
			// rebellion. are we out of resources?
			if(AWorld[y][x].resources==MIN_RESOURCES)
				rebellion(y,x); //rebel!
			
			// try to migrate/leave if food running low
			if(AWorld[y][x].food<=LOW_FOOD)
			{
				direction=best_direction_to_move(y,x);
				if(direction!=NO_DIRECTION)
				{
					int newy,newx;
					look_in_direction(y,x,direction,newy,newx);
					if(AWorld[newy][newx].tribe==NO_TRIBE)
						move_in(y,x,direction);
					else if(AWorld[newy][newx].tribe!=AWorld[y][x].tribe)
						war_on(y,x,direction);
				}
			}
			
			// if lots of food, give birth to up to 2.
			int count=2;
			while(AWorld[y][x].food>=birth_food && count>0)
			{
				direction=best_direction_to_give_birth(y,x);
				if(direction==NO_DIRECTION)
					break;
				else
				{
					give_birth(y,x,direction);
					count--;
				}
			};
			if(count!=2)
				multiple_births[2-count]++;
			
			
			//
			AWorld[y][x].turnover=true;
		}
	}
}

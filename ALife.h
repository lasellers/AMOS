// ALife.h: interface for the ALife class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALIFE_H__39B4F73A_0A28_4747_AF57_064F4031B561__INCLUDED_)
#define AFX_ALIFE_H__39B4F73A_0A28_4747_AF57_064F4031B561__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class ALife  
{
public:
	ALife();
	virtual ~ALife();
	void NextGeneration();
	int Reset();
	void Disaster();
	void Fortune();
	void SuperFortune();
	
private:
	void NextGeneration_sub(int y, int x);
	int best_direction_to_move(int y, int x);
	int best_direction_to_give_birth(int y, int x);
	void move_in(int y, int x, int direction);
	void give_birth(int y, int x, int direction);
	void war_on(int y, int x, int direction);
	void starve_to_death(int y, int x);
	inline int direction_to_migration_strategy_point(int y, int x);
	int direction_based_on_local_conditions(int y, int x);
	void rebellion(int y, int x);
	int steal_food(int y, int x);
	void inline look_in_direction(int y, int x, int d, int& newy, int& newx);
	inline void look_at(int y, int x, int& newy, int& newx);
	bool is_a_city(int y, int x);
	void cooperative_effects_of_city(int y, int x);
	
public:
	
	// 0-7 direction optimization
	enum {
		//MAX_DIRECTIONS=8,
		NO_DIRECTION=8
	};
	struct _D_TO_XY_LOOKUP
	{
		short x,y;
	} DXY[8+1];
	
	
	//
	enum {
		WIDTH=170,
			HEIGHT=170
	};
	enum {
		MIN_FOOD=0,
			MAX_FOOD=15,
			STARTING_FOOD=15,
			LOW_FOOD=3, //3, //4
			BIRTH_FOOD=7, //4, //4, //4
			//	BIRTH_FOOD_COST=1 //2 //3
	};
	enum
	{
		MIN_RESOURCES=0,
			MAX_RESOURCES=15,
			STARTING_RESOURCES=15,
			LOW_RESOURCES=4
	};
	struct _AWORLD
	{
		short tribe;
		short resources; //-16 to 16.  life-giving properties of area
		short food; //0 to 16. food of current al popluation
		bool turnover;
		short direction; //used for off-path migration
	} AWorld[WIDTH][HEIGHT];
	
	//
	enum {
		MAX_DEFAULT_TRIBES=8,
			NO_TRIBE=MAX_DEFAULT_TRIBES
	};
	struct _TRIBES
	{
		int population;
		int migrationX;
		int migrationY;
		bool death_event; //to signal externals to play sound. they reset this.
		short direction;//for off-path (local round-robin) migration
	} Tribes[MAX_DEFAULT_TRIBES+1];
	
	//
	int foodX;
	int foodY;
	
	// generation count
	int generation;
	int births;
	int deaths;
	int fights;
	int starvations;
	int migration_decisions;
	int total_food;
	int total_resources;
	int multiple_births[2+1];
	int rebellions;
	int cities;
	
	//
	int width;
	int height;
	int number_of_tribes;
	int visualmultiplier256; //16 32
	
	//
	bool wraparound;
	int migration; //0=inward, 1=outward
	int birth_food;
	bool allow_war;
	bool allow_rebellion;
	bool allow_cities;
	bool allow_carrying;
	bool allow_wastelands;
	enum
	{
		MIGRATION_NONE=0,
			MIGRATION_INWARD,
			MIGRATION_OUTWARD,
			MIGRATION_AGGRESSION,
			MIGRATION_CENTER,
			MIGRATION_HELTERSKELTER,
			MIGRATION_OFFPATH,
			MIGRATION_PATH,
			MIGRATION_FOOD,
			
			MIGRATION_YOUR_ROUTINE_1,
			MIGRATION_YOUR_ROUTINE_2,
			
			MAX_MIGRATION_TYPES
	};
	
	// un-biasing vars
	// we try round-robinng all scanning and looking functions so
	// we're not always biasing the directions we choose in for loops to
	// the numerically lower directions.
	// if we don't do this the patterns tend to drift up and right.
	int scan_rotation;
};

#endif // !defined(AFX_ALIFE_H__39B4F73A_0A28_4747_AF57_064F4031B561__INCLUDED_)

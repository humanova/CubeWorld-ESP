#include <iostream>
#include <cstdint>
//#include "proc.h"
#include "Creature.h"
#include <vector>
#include <Windows.h>
class CubeX
{

public:

    struct GameVal
    {
        std::vector<Creature *>* Creatures;
		Creature* LocalPlayer;
        int num_creatures;
    }val;

    struct Offsets
    {
        unsigned int CubeBase;
        unsigned int GameController;
        unsigned int World;
        unsigned int EntityMap;
		unsigned int LocalPlayer;
    }offset;

    CubeX();
    void RefreshVal();
	std::vector<Creature*>* GetCreatures();
	
	Creature* GetPlayer();
	void GetGameOffsets();
    void PrintOffsets();

	std::vector<unsigned int>* CCreaturePtr;
	unsigned int CPlayerPtr;
private:


    HANDLE cube_handle;
    int pid;


};
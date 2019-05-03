#include "CubeX.h"

CubeX::CubeX()
{
    //GetWindowOpts();

    //GetCubeHandle();
    GetGameOffsets();
    //RefreshVal();
    //RenderESP();
}
/*
HANDLE CubeX::GetCubeHandle()
{
	pid = GetProcId(L"Cube.exe");
	HANDLE CubeHandle = 0;
	if (!pid)
	{
		printf("Couldn't get Cube PID...");
		Sleep(1000); exit(-1);
	}
	else
	{
		CubeHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	}
	cube_handle = CubeHandle;
	return CubeHandle;
}*/

void CubeX::GetGameOffsets()
{
	offset.CubeBase = (unsigned int)GetModuleHandle(NULL);
    offset.GameController = *(unsigned int*)(offset.CubeBase + CubeOffset::GameController);
    offset.World = offset.GameController + CubeOffset::World;
    offset.EntityMap = offset.World + CubeOffset::EntityMap;
	offset.LocalPlayer = offset.GameController + CubeOffset::LocalPlayer;
}

std::vector<Creature*>* CubeX::GetCreatures(){
	std::vector<Creature*>* creatures = new std::vector<Creature*>;
	unsigned int map_ptr = *(unsigned int *)offset.EntityMap;
	unsigned int node_ptr = *(unsigned int*)map_ptr;

	while (node_ptr != map_ptr) {
		unsigned int creature_ptr_ptr = node_ptr + 0x18;
		unsigned int creature_ptr = *(unsigned int*)creature_ptr_ptr;
		Creature* creature = (Creature*)creature_ptr;
		typedef void(__thiscall* mapnext_t)(unsigned int* node_ptr);
		auto mapnext = (mapnext_t)(offset.CubeBase + 0x1C3EA0);
		mapnext(&node_ptr);

		creatures->push_back(creature);
	}
	return creatures;
}

Creature* CubeX::GetPlayer()
{
	unsigned int player_ptr = *(unsigned int *)offset.LocalPlayer;
	return (Creature *)player_ptr;
}
void CubeX::RefreshVal()
{
    val.Creatures = GetCreatures();
    val.num_creatures = val.Creatures->size();
	val.LocalPlayer = GetPlayer();
	CPlayerPtr = *(unsigned int *)offset.LocalPlayer;
}

void CubeX::PrintOffsets()
{
    printf("CubeBase(imageBase) : %x\n", offset.CubeBase);
    printf("GameController : %x\n", offset.GameController);
    printf("World : %x\n", offset.World);
    printf("EntityMapPtr : %x\n", offset.EntityMap);
}

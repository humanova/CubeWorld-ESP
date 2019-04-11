#include <iostream>
#include <cstdint>
//#include "proc.h"
#include "utils.h"
#include <vector>
#include <Windows.h>
class CubeX
{

public:
    class Creature{
    public:
        unsigned int field_0;
        unsigned int field_4;
        long long int GUID;
        long long unsigned int x, y, z; //0x10 ~ 0x27
        char padding2[0x60];
        float physical_size; //0x88
        char padding1[0xD4];
        Vec3f camera_offset;
        float HP;
        float MP;
        float block_power;
        float HP_multiplier;
        float attack_speed_multiplier;
        float damage_multiplier;
        float armor_multiplier;
        float resistance_multiplier;
        char field_18C;
        char field_18D;
        char field_18E;
        char field_18F;
        int level;
        int XP;
        long long parent_GUID;

    };

    struct GameVal
    {
        std::vector<Creature *>* Creatures;
        int num_creatures;
    }val;

    struct Offsets
    {
        unsigned int CubeBase;
        unsigned int GameController;
        unsigned int World;
        unsigned int EntityMap;
    }offset;

    CubeX();
    void RefreshVal();
	std::vector<CubeX::Creature*>* GetCreatures();
    //HANDLE GetCubeHandle();
    void GetGameOffsets();
    void PrintOffsets();


private:

    HANDLE cube_handle;
    int pid;


};
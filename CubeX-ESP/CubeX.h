#include <iostream>
#include <cstdint>
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
		int64_t CursorEntityGUID;
		Vector3<int64_t> CameraPosition;
		Vector3<float> CameraAngle;
        int num_creatures;
    }val;

    struct Offsets
    {
        unsigned int CubeBase;
        unsigned int GameController;
		unsigned int CameraDistance;
		unsigned int CameraAngle;
		unsigned int CameraPosition;
		unsigned int CursorEntityGUID;
        unsigned int World;
		unsigned int WorldTime;
        unsigned int EntityMap;
		unsigned int LocalPlayer;

		//unsigned int CameraAngleCurr;
		//unsigned int CameraFocusPosition;
    }offset;

	struct Constants
	{
		double 
				PI = 3.141592653,
				RAD = 57.2957796;
	}constant;

    CubeX();
    void RefreshVal();
	std::vector<Creature*>* GetCreatures();
	int GetClosestCreatureId();
	Vector2<float> GetAimbotAngles(Vector3<int64_t> aimbotted_pos, bool from_player_pos);
	void SetCameraAngle(Vector2<float> angle);
	//void SetCameraFocusPosition(Vector3<int64_t> pos);
	
	Creature* GetPlayer();
	Creature* GetCreatureFromGUID(int64_t guid);
	void GetGameOffsets();
	const char* GetClass(char class_value);
    void PrintOffsets();

	std::vector<unsigned int>* CCreaturePtr;
	unsigned int CPlayerPtr;

	Creature* creature_temp = new Creature();
};
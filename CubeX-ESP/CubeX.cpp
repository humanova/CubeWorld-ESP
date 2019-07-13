#include "CubeX.h"

CubeX::CubeX()
{
	GetGameOffsets();
	creature_temp->GUID = -1;
}	


void CubeX::GetGameOffsets()
{
	offset.CubeBase = (unsigned int)GetModuleHandle(NULL);
    offset.GameController = *(unsigned int*)(offset.CubeBase + CubeOffset::GameController);
	offset.CameraDistance = offset.GameController + CubeOffset::CameraDistance;
	offset.CameraPosition = offset.GameController + CubeOffset::CameraPosition;
	offset.CameraAngle = offset.GameController + CubeOffset::CameraAngle;
    offset.CursorEntityGUID = offset.GameController + CubeOffset::CursorEntityGUID;
    offset.World = offset.GameController + CubeOffset::World;
	offset.WorldTime = offset.World + CubeOffset::WorldTime;
    offset.EntityMap = offset.World + CubeOffset::EntityMap;
	offset.LocalPlayer = offset.GameController + CubeOffset::LocalPlayer;

	//offset.CameraAngleCurr = offset.GameController + CubeOffset::CameraAngleCurr;
	//offset.CameraFocusPosition = offset.GameController + CubeOffset::CameraFocusPosition;
}

const char* CubeX::GetClass(char class_value)
{
	switch (int(class_value))
	{
	case 0:
		return "Rogue";
		break;
	case 1:
		return "Warrior";
		break;
	case 2:
		return "Ranger";
		break;
	case 3:
		return "Mage";
		break;
	default:
		return "Unidentified";
		break;
	}
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

		/* Don't get local player
		if(creature->GUID != 1)
			creatures->push_back(creature);
		*/
		creatures->push_back(creature);
	}
	return creatures;
}

int CubeX::GetClosestCreatureId()
{
	int closest_id = 0;
	for (int i = 0; i < val.num_creatures; i++)
	{	
		if (val.Creatures[0][i]->GUID == val.LocalPlayer->GUID)
		{
			if (i == 0)
				closest_id = 1;
			continue;
		}	
		if (i == 0)
		{
			closest_id = i;
		}
		else if (val.Creatures[0][i]->DistanceFrom(val.LocalPlayer) < val.Creatures[0][closest_id]->DistanceFrom(val.LocalPlayer))
		{
			closest_id = i;
		}
	}
	return closest_id;
}

Vector2<float> CubeX::GetAimbotAngles(Vector3<int64_t> aimbotted_pos, bool from_player_pos)
{
	Vector2<float> angle;
	Vector3<int64_t> distance;
	int64_t XYDistance;

	if (from_player_pos)
	{
		//player coordinates
		distance.X = val.LocalPlayer->entity_data.position.Y - aimbotted_pos.Y;
		distance.Y = val.LocalPlayer->entity_data.position.X - aimbotted_pos.X;
		//distance.Z = val.CameraPosition.Z - aimbotted_pos.Z;
		distance.Z = (val.LocalPlayer->entity_data.position.Z + 65535) - aimbotted_pos.Z;
	}
	else // from cam pos
	{
		distance.X = val.CameraPosition.Y - aimbotted_pos.Y;
		distance.Y = val.CameraPosition.X - aimbotted_pos.X;
		distance.Z = val.CameraPosition.Z - aimbotted_pos.Z;
	}
	

	XYDistance = sqrt(pow(fabs(distance.X), 2) + pow(fabs(distance.Y), 2));

	if (distance.X > 0 && distance.Y < 0)
		angle.X = (atan(fabs(distance.Y) / fabs(distance.X))) * constant.RAD;
	else if (distance.X < 0 && distance.Y < 0)
		angle.X = (90 - (atan(fabs(distance.Y) / fabs(distance.X))) * constant.RAD) + 90;
	else if (distance.X < 0 && distance.Y > 0)
		angle.X = (atan(fabs(distance.Y) / fabs(distance.X))) * constant.RAD + 180;
	else if (distance.X > 0 && distance.Y > 0)
		angle.X = (90 - (atan(fabs(distance.Y) / fabs(distance.X))) * constant.RAD) + 270;

	if (distance.Z < 0)
		angle.Y = 90 - constant.RAD * (atan(fabs(distance.Z) / XYDistance));
	else if (distance.Z > 0)
		angle.Y = 90 + constant.RAD * (atan(fabs(distance.Z) / XYDistance));

	return angle;
}

void CubeX::SetCameraAngle(Vector2<float> angle)
{
	Vector3<float>* last_angle = new Vector3<float>(angle.Y, val.CameraAngle.Y, angle.X);
	*(Vector3<float> *)offset.CameraAngle = *last_angle;
}

/*
void CubeX::SetCameraFocusPosition(Vector3<int64_t> pos)
{
	*(Vector3<int64_t> *)offset.CameraFocusPosition = pos;
}*/

Creature* CubeX::GetPlayer()
{
	unsigned int player_ptr = *(unsigned int *)offset.LocalPlayer;
	return (Creature *)player_ptr;
}


Creature* CubeX::GetCreatureFromGUID(int64_t guid)
{
	for (int i = 0; i < val.num_creatures; i++)
	{
		if (val.Creatures[0][i]->GUID == guid)
		{
			return val.Creatures[0][i];
		}
	}
	return creature_temp;
}

void CubeX::RefreshVal()
{
    val.Creatures = GetCreatures();
    val.num_creatures = val.Creatures->size();
	val.LocalPlayer = GetPlayer();
	val.CameraPosition = *(Vector3<int64_t> *)(offset.CameraPosition);
	val.CameraAngle = *(Vector3<float> *)(offset.CameraAngle);
	val.CursorEntityGUID = *(int64_t *)(offset.CursorEntityGUID);
	CPlayerPtr = *(unsigned int *)offset.LocalPlayer;

	//val.CameraFocusPosition = *(Vector3<int64_t> *)(offset.CameraFocusPosition);
	//val.CameraAngleCurr = *(Vector3<float> *)(offset.CameraAngleCurr);
}

void CubeX::PrintOffsets()
{
    printf("CubeBase(imageBase) : %x\n", offset.CubeBase);
    printf("GameController : %x\n", offset.GameController);
    printf("World : %x\n", offset.World);
    printf("EntityMapPtr : %x\n", offset.EntityMap);
}

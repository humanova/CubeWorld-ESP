#pragma warning(disable : 4996)
#include "CubeX.h"
#include <bitset>
//#include "Draw.h";
#include "dx9.h"
#include "Colors.h"

#include <detours.h>
#pragma comment(lib,"detours.lib")

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

using namespace std;

const char* windowName = "Cube";
WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(_stdcall* f_Reset)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(_stdcall * f_EndScene)(IDirect3DDevice9 * pDevice);
typedef HRESULT(_stdcall * f_DrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, int BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimCount);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

f_Reset oReset = nullptr;
f_EndScene oEndScene = nullptr;
f_DrawIndexedPrimitive oDrawIndexedPrimitive = nullptr;

IDirect3DDevice9 * dx9Device;
static bool imgui_init = false;
bool is_wireframe = false;
bool is_esp = false;
bool aimbot = false;
int aimbot_mode = 0;
Vector2<float> angle2;
Vector3<float> angle;
int64_t aimbotted_guid = 0;
bool aimbot_smoothness = false;
bool aimbot_from_pos = false;
int closest_creature_id;
bool debug_creatures = false;
bool show_xyz = true;
bool show_on_map = false;
bool debug_wireframe = false;
bool edit_pos = false;
bool edit_pos_init = false;
bool edit_time = false;
bool edit_player = false;
bool edit_skills = false;
bool edit_item = false;
bool edit_appearance = false;
bool edit_appearance_init = false;
bool edit_multipliers = false;
bool show_selected = false;
bool wind_spirit_effect = false;
bool ice_spirit_effect = false;
bool freeze_time = false;
bool freeze_pet_water = false;
bool freeze_hp = false, freeze_mp = false, freeze_stamina = false;
bool walk_on_air = false;

int selected_creature_num = 0;
Creature* selected_creature;
int selected_creature_distance = 0;
int wireframe_stride = 8;
int time;
int64_t last_y;
int appearance[8];

float hair_color[3];
bool rainbow_init;
bool rainbow_hair;
bool giant_mode = false;
bool giant_done = false;
bool no_stun = false;
int x = 0, y = 0, z = 0;
Vector3<int64_t> cur_pos;

bool wireframe_everything = false;
int wireframe_vertices = 10;
CubeX cube;

void DrawLine(float x1, float y1, float x2, float y2, float width, bool antialias, DWORD color)
{
	ID3DXLine *m_Line;

	D3DXCreateLine(dx9Device, &m_Line);
	D3DXVECTOR2 line[] = { D3DXVECTOR2(x1, y1), D3DXVECTOR2(x2, y2) };
	m_Line->SetWidth(width);
	if (antialias) m_Line->SetAntialias(1);
	m_Line->Begin();
	m_Line->Draw(line, 2, color);
	m_Line->End();
	m_Line->Release();

}

bool WorldToScreen(D3DXVECTOR3 pos, vec2 &screen, float matrix[16], int windowWidth, int windowHeight)
{
	//printf("WS2 === \n POS : %lld - %lld - %lld\n", pos.x, pos.y, pos.z);
	//printf("matrix : \n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n", matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);
	//printf("window : %d x %d", windowWidth, windowHeight);
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	vec4 clipCoords;
	clipCoords.x = pos.x*matrix[0] + pos.y*matrix[1] + pos.z*matrix[2] + matrix[3];
	clipCoords.y = pos.x*matrix[4] + pos.y*matrix[5] + pos.z*matrix[6] + matrix[7];
	clipCoords.z = pos.x*matrix[8] + pos.y*matrix[9] + pos.z*matrix[10] + matrix[11];
	clipCoords.w = pos.x*matrix[12] + pos.y*matrix[13] + pos.z*matrix[14] + matrix[15];

	//printf("clipCoords : %f\n", clipCoords.w);
	if (clipCoords.w < 0.1f)
		return false;
	vec3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	return true;
}

bool WorldToScreenTest(LPDIRECT3DDEVICE9 pDevice, D3DXVECTOR3* pos, D3DXVECTOR3* out)
{
	D3DVIEWPORT9 viewPort;
	D3DXMATRIX view, projection, world;

	pDevice->GetViewport(&viewPort);
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &projection);
	D3DXMatrixIdentity(&world);

	D3DXVec3Project(out, pos, &viewPort, &projection, &view, &world);
	if (out->z < 1)
		return true;
	else
		return false;
}

void DrawESP()
{
	D3DXVECTOR3 pos;
	vec2 Screen;
	D3DXMATRIX view;
	D3DVIEWPORT9 viewPort;
	D3DXVECTOR3 screenPos;
	dx9Device->GetTransform(D3DTS_VIEW, &view);
	dx9Device->GetViewport(&viewPort);
	for (int i = 0; i < cube.val.num_creatures; i++)
	{
		if (cube.val.Creatures[0][i]->GUID == cube.val.LocalPlayer->GUID)
		{
			continue;
		}
			
		pos.x = cube.val.Creatures[0][i]->entity_data.position.X;
		pos.y = cube.val.Creatures[0][i]->entity_data.position.Y;
		pos.z = cube.val.Creatures[0][i]->entity_data.position.Z;

		/*
		if (WorldToScreen(pos, Screen, view, viewPort.Width, viewPort.Height))
		{
			DrawLine(Screen.x, Screen.y, viewPort.Width / 2, viewPort.Height, 2, true, RED(255));
		}*/
		
		
		if (WorldToScreenTest(dx9Device, &pos, &screenPos))
		{
			DrawLine(screenPos.x, screenPos.y, viewPort.Width / 2, viewPort.Height, 2, true, RED(255));
		}
	}
}

HRESULT _stdcall Hooked_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, int BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimCount)
{
	if (is_wireframe)
	{
		IDirect3DVertexBuffer9* pStreamData = NULL;
		UINT pOffsetInBytes, pStride;
		dx9Device->GetStreamSource(0, &pStreamData, &pOffsetInBytes, &pStride);
		pStreamData->Release();

		if (!wireframe_everything) 
		{
			if (pStride == wireframe_stride || wireframe_vertices == NumVertices)
			{
				dx9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			}
			else
			{
				dx9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			}
			/*
			DWORD dwReEnableZB = D3DZB_TRUE;
			
			dx9Device->GetRenderState(D3DRS_ZENABLE, &dwReEnableZB); //Make sure the Buffer is enabled
			dx9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE); // Disable it
			dx9Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);// Reject the pixels (shaders)
			*/
		}
		else if (wireframe_everything)
		{
			dx9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
	}

	return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimCount);
}


HRESULT __stdcall Hooked_Reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	if (imgui_init) 
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		HRESULT result = oReset(pDevice, pPresentationParameters);
		ImGui_ImplDX9_CreateDeviceObjects();
		return result;
	}

	return oReset(pDevice, pPresentationParameters);
}

HRESULT _stdcall Hooked_EndScene(IDirect3DDevice9 * pDevice)
{
	//D3DRECT BarRect = { 100, 100, 200, 200 };
	//pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 1, 1, 1), 0.0f, 0);

	if (!imgui_init)
	{
		imgui_init = true;
		dx9Device = pDevice;
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(GetActiveWindow());
		ImGui_ImplDX9_Init(pDevice);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Checkbox("ESP", &is_esp);
	ImGui::Checkbox("Aimbot", &aimbot);
	ImGui::Checkbox("Edit Player", &edit_player);
	ImGui::Checkbox("Edit Position", &edit_pos);
	ImGui::Checkbox("Edit Time", &edit_time);
	ImGui::Checkbox("Creatures", &debug_creatures);
	ImGui::Checkbox("Wireframe", &debug_wireframe);


	if (is_esp || aimbot ||debug_creatures)
		cube.RefreshVal();

	if (is_esp)
	{
		DrawESP();
	}

	if (aimbot)
	{
		ImGui::Begin("Aimbot");
		ImGui::Checkbox("From Player Pos", &aimbot_from_pos);
		ImGui::Text("Aimbot Mode");
		ImGui::RadioButton("Off", &aimbot_mode, 0);
		ImGui::RadioButton("Closest Creature", &aimbot_mode, 1);
		ImGui::RadioButton("Selected Creature", &aimbot_mode, 2);

		ImGui::Text("Aimbotted GUID : %lld", aimbotted_guid);
		ImGui::Text("Cam Angles : %f - %f - %f", cube.val.CameraAngle.X, cube.val.CameraAngle.Y, cube.val.CameraAngle.Z);

		if (aimbot_mode == 1)
		{
			closest_creature_id = cube.GetClosestCreatureId();
			aimbotted_guid = cube.val.Creatures[0][closest_creature_id]->GUID;
			angle2 = cube.GetAimbotAngles(cube.val.Creatures[0][closest_creature_id]->entity_data.position, aimbot_from_pos);
			cube.SetCameraAngle(angle2);
		}

		else if (aimbot_mode == 2)
		{
			aimbotted_guid = cube.val.CursorEntityGUID;
			if (cube.GetCreatureFromGUID(aimbotted_guid)->GUID != -1)
				cube.SetCameraAngle(cube.GetAimbotAngles(cube.GetCreatureFromGUID(aimbotted_guid)->entity_data.position, aimbot_from_pos));
		}
		ImGui::End();
	}

	if (edit_time)
	{
		ImGui::Begin("Edit World Time");
		int curr_time = *(int *)(cube.offset.WorldTime);
		ImGui::Text("Current Time : %d", curr_time);
		ImGui::InputInt("Time", &time, 10000, 1000000);
		ImGui::Checkbox("Freeze Time", &freeze_time);

		if (freeze_time)
		{
			*(int *)(cube.offset.WorldTime) = time;
		}

		if (ImGui::SmallButton("Change Time"))
		{
			*(int *)(cube.offset.WorldTime) = time;
		}
		ImGui::End();
	}

	if (debug_wireframe)
	{
		ImGui::Begin("Wireframe Stuff");
		ImGui::Checkbox("Wireframe", &is_wireframe);
		ImGui::SameLine();
		ImGui::Checkbox("All", &wireframe_everything);
		if (!wireframe_everything)
		{
			ImGui::SliderInt("pStride", &wireframe_stride, 3, 1000);
			ImGui::SameLine();
			ImGui::InputInt("strds", &wireframe_stride, 1, 20);
			ImGui::SliderInt("numVertices", &wireframe_vertices, 3, 5000);  
			ImGui::SameLine(); 
			ImGui::InputInt("verts", &wireframe_vertices, 1, 20);
		}
		ImGui::End();
	}

	if (edit_player)
	{
		ImGui::Begin("Edit Player");
		ImGui::Text("Player : %s", cube.GetPlayer()->entity_data.name);
		ImGui::Text("Class : %s", cube.GetClass(cube.GetPlayer()->entity_data.char_class));
		ImGui::SameLine();
		ImGui::Text("Spec : %d", cube.GetPlayer()->entity_data.char_specialization);
		ImGui::SliderFloat("HP", &cube.GetPlayer()->entity_data.hp, 1, 15000);
		ImGui::SliderFloat("MP", &cube.GetPlayer()->entity_data.mp, 0, 1);
		ImGui::Checkbox("Freeze HP", &freeze_hp);
		ImGui::SameLine();
		ImGui::Checkbox("Freeze MP", &freeze_mp);
		ImGui::Checkbox("Freeze Stamina", &freeze_stamina);

		ImGui::Spacing();

		ImGui::InputInt("Level", &cube.GetPlayer()->entity_data.level, 1, 10);
		ImGui::InputInt("Money", &cube.GetPlayer()->money, 1000, 100000);
		ImGui::InputInt("Platinums", &cube.GetPlayer()->platinum_coins, 10, 100);

		ImGui::Spacing();

		ImGui::Checkbox("Edit Item", &edit_item);

		if (edit_item)
		{
			ImGui::Begin("Edit Item");

			ImGui::Text("Lvl	  : %d", cube.GetPlayer()->selected_item.level);
			ImGui::Text("Count    : %d", cube.GetPlayer()->selected_item_count);
			ImGui::Text("Category : %d", cube.GetPlayer()->selected_item.category_id);
			ImGui::Text("Id       : %d", cube.GetPlayer()->selected_item.item_id);
			ImGui::Text("Modifier : %d", cube.GetPlayer()->selected_item.modifier);
			ImGui::Text("Rarity   : %d", cube.GetPlayer()->selected_item.rarity);
			ImGui::Text("Material : %d", cube.GetPlayer()->selected_item.material);
			ImGui::Text("Adapted  : %d", cube.GetPlayer()->selected_item.adapted);
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Text("Change Modifier : UP/DOWN");
			ImGui::Text("Copy Item       : Z");
			ImGui::Text("Adapt           : LCTRL");

			
			if (GetAsyncKeyState(0x5A) & 1) 
			{
				if (cube.GetPlayer()->selected_item_count != 0)
					cube.GetPlayer()->selected_item_count += 1;
			}
			if (GetAsyncKeyState(VK_LCONTROL) & 1)
			{
				if (cube.GetPlayer()->selected_item_count != 0)
					cube.GetPlayer()->selected_item.level = (uint16_t)cube.val.LocalPlayer->entity_data.level;
			}
			if (GetAsyncKeyState(VK_UP) & 1)
			{
				if (cube.GetPlayer()->selected_item_count != 0)
					cube.GetPlayer()->selected_item.modifier += 1;
			}
			if (GetAsyncKeyState(VK_DOWN) & 1)
			{
				if (cube.GetPlayer()->selected_item_count != 0)
					cube.GetPlayer()->selected_item.modifier -= 1;
			}
			ImGui::End();
		}

		ImGui::SameLine();

		if (ImGui::SmallButton("Adapt Equipment"))
		{
			cube.GetPlayer()->entity_data.equipment.chest.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.feet.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.hands.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.shoulder.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.necklace.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.left_ring.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.right_ring.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.left_weapon.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.right_weapon.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			cube.GetPlayer()->entity_data.equipment.pet.level = (uint16_t)(cube.val.LocalPlayer->entity_data.level);
			//special item's level isn't important (because there is no such thing)
			//their rarity are important
			cube.GetPlayer()->entity_data.equipment.light.rarity = 5;
			cube.GetPlayer()->entity_data.equipment.special.rarity = 5;
		}

		ImGui::Checkbox("Edit Skills", &edit_skills);
		
		if (edit_skills)
		{
			ImGui::Begin("Edit Skills");
			if (ImGui::Button("Change Class Spec"))
				cube.GetPlayer()->entity_data.char_specialization = !int(cube.GetPlayer()->entity_data.char_specialization);

			ImGui::Text("Primitive Skills");
			ImGui::InputInt("Climbing", &cube.GetPlayer()->entity_data.skill_level_climbing, 1, 10);
			ImGui::InputInt("Gliding", &cube.GetPlayer()->entity_data.skill_level_gliding, 1, 10);
			ImGui::InputInt("Petmaster", &cube.GetPlayer()->entity_data.skill_level_petmaster, 1, 10);
			ImGui::InputInt("Riding", &cube.GetPlayer()->entity_data.skill_level_riding, 1, 10);
			ImGui::InputInt("Sailing", &cube.GetPlayer()->entity_data.skill_level_sailing, 1, 10);
			ImGui::InputInt("Swimming", &cube.GetPlayer()->entity_data.skill_level_swimming, 1, 10);
			ImGui::Text("Class Skills");
			ImGui::InputInt("Class Skill 1", &cube.GetPlayer()->entity_data.skill_level_class_1, 1, 10);
			ImGui::InputInt("Class Skill 2", &cube.GetPlayer()->entity_data.skill_level_class_2, 1, 10);
			ImGui::InputInt("Class Skill 3", &cube.GetPlayer()->entity_data.skill_level_class_3, 1, 10);
			ImGui::InputInt("Class Skill 4", &cube.GetPlayer()->entity_data.skill_level_class_4, 1, 10);
			ImGui::InputInt("Class Skill 5", &cube.GetPlayer()->entity_data.skill_level_class_5, 1, 10);

			ImGui::End();
		}
		
		ImGui::Checkbox("Edit Appearance", &edit_appearance);
		if (edit_appearance)
		{
			if (!edit_appearance_init)
			{
				appearance[0] = cube.GetPlayer()->entity_data.appearance.hair_id;
				appearance[1] = cube.GetPlayer()->entity_data.appearance.face_id;
				appearance[2] = cube.GetPlayer()->entity_data.appearance.chest_id;
				appearance[3] = cube.GetPlayer()->entity_data.appearance.shoulder_id;
				appearance[4] = cube.GetPlayer()->entity_data.appearance.hands_id;
				appearance[5] = cube.GetPlayer()->entity_data.appearance.wings_id;
				appearance[6] = cube.GetPlayer()->entity_data.appearance.tail_id;
				appearance[7] = cube.GetPlayer()->entity_data.appearance.feet_id;
				hair_color[0] = cube.GetPlayer()->entity_data.appearance.hair_color_red;
				hair_color[1] = cube.GetPlayer()->entity_data.appearance.hair_color_blue;
				hair_color[2] = cube.GetPlayer()->entity_data.appearance.hair_color_blue;
				edit_appearance_init = true;
			}
			ImGui::Begin("Edit Appearance");
			ImGui::InputInt("Hair Id", &appearance[0], 1, 1);
			ImGui::InputInt("Face Id", &appearance[1], 1, 1);
			ImGui::InputInt("Chest Id", &appearance[2], 1, 1);
			ImGui::InputInt("Shoulder Id", &appearance[3], 1, 1);
			ImGui::InputInt("Hands Id", &appearance[4], 1, 1);
			ImGui::InputInt("Wings Id", &appearance[5], 1, 1);
			ImGui::InputInt("Tail Id", &appearance[6], 1, 1);
			ImGui::InputInt("Feet Id", &appearance[7], 1, 1);
			ImGui::ColorEdit3("Hair Color", hair_color);
			if (ImGui::Button("Set Appearance"))
			{
				cube.GetPlayer()->entity_data.appearance.hair_id = (uint16_t)appearance[0];
				cube.GetPlayer()->entity_data.appearance.face_id = (uint16_t)appearance[1];
				cube.GetPlayer()->entity_data.appearance.chest_id = (uint16_t)appearance[2];
				cube.GetPlayer()->entity_data.appearance.shoulder_id = (uint16_t)appearance[3];
				cube.GetPlayer()->entity_data.appearance.hands_id = (uint16_t)appearance[4];
				cube.GetPlayer()->entity_data.appearance.wings_id = (uint16_t)appearance[5];
				cube.GetPlayer()->entity_data.appearance.tail_id = (uint16_t)appearance[6];
				cube.GetPlayer()->entity_data.appearance.feet_id = (uint16_t)appearance[7];
				cube.GetPlayer()->entity_data.appearance.hair_color_red = uint8_t(hair_color[0]);
				cube.GetPlayer()->entity_data.appearance.hair_color_green = uint8_t(hair_color[1]);
				cube.GetPlayer()->entity_data.appearance.hair_color_blue = uint8_t(hair_color[2]);
			}

			ImGui::Checkbox("Rainbow Hair", &rainbow_hair);
			ImGui::Spacing();
			ImGui::SliderFloat("Weapon Scale", &cube.GetPlayer()->entity_data.appearance.weapon_scale, 0.1, 50, "%.2f");
			ImGui::SliderFloat("Physical Size", &cube.GetPlayer()->entity_data.appearance.physical_size, 0.1, 50, "%.2f");
			ImGui::SliderFloat("Graphical Size", &cube.GetPlayer()->entity_data.appearance.graphical_size, 0.1, 50, "%.2f");

			if (rainbow_hair)
			{
				if (!rainbow_init)
				{
					hair_color[0] = 255;
					hair_color[1] = 0;
					hair_color[2] = 0;
					rainbow_init = true;
				}

				// some ugly code, but it works lol
				if (hair_color[0] == 255 && hair_color[1] != 255 && hair_color[2] == 0)
					hair_color[1]++;
				else if (hair_color[0] != 0 && hair_color[1] == 255 && hair_color[2] == 0)
					hair_color[0]--;
				else if (hair_color[0] == 0 && hair_color[1] == 255 && hair_color[2] != 255)
					hair_color[2]++;
				else if (hair_color[0] == 0 && hair_color[1] != 0 && hair_color[2] == 255)
					hair_color[1]--;
				else if (hair_color[0] != 255 && hair_color[1] == 0 && hair_color[2] == 255)
					hair_color[0]++;
				else if (hair_color[0] == 255 && hair_color[1] == 0 && hair_color[2] != 0)
					hair_color[2]--;

				cube.GetPlayer()->entity_data.appearance.hair_color_red = uint8_t(hair_color[0]);
				cube.GetPlayer()->entity_data.appearance.hair_color_green = uint8_t(hair_color[1]);
				cube.GetPlayer()->entity_data.appearance.hair_color_blue = uint8_t(hair_color[2]);
			}
			else
				rainbow_init = false;

			ImGui::End();
		}
		else
			edit_appearance_init = false;
	
		
		ImGui::Checkbox("Edit Multipliers", &edit_multipliers);
		if (edit_multipliers)
		{
			ImGui::Begin("Edit Multipliers");
			ImGui::SliderFloat("HP Multiplier", &cube.GetPlayer()->entity_data.hp_multiplier, 0.1, 50);
			ImGui::SliderInt("Damage Multiplier", &cube.GetPlayer()->entity_data.damage_multiplier, 1, 50);
			ImGui::SliderInt("Attack Speed Multiplier", &cube.GetPlayer()->entity_data.attack_speed_multiplier, 1, 50);
			ImGui::SliderInt("Resistance Multiplier", &cube.GetPlayer()->entity_data.resistance_multiplier, 1, 50);
			ImGui::SliderInt("Armor Multiplier", &cube.GetPlayer()->entity_data.armor_multiplier, 1, 50);
			ImGui::SliderInt("Unkown Multiplier", &cube.GetPlayer()->entity_data.unk_multiplier, 1, 50);
			ImGui::End();
		}

		ImGui::Checkbox("Wind Spirit Effect", &wind_spirit_effect);
		ImGui::Checkbox("Ice Spirit Effect", &ice_spirit_effect);
		ImGui::Checkbox("No stun", &no_stun);
		ImGui::Checkbox("Walk On Air", &walk_on_air);
		ImGui::Checkbox("Giant Mode", &giant_mode);
		ImGui::Checkbox("Freeze Petwater", &freeze_pet_water);

		ImGui::SliderFloat("Camera Distance", (float *)(cube.offset.CameraDistance), 1, 500);

		if (wind_spirit_effect)
			cube.GetPlayer()->entity_data.wind_spirit_effect = 5000;

		if (ice_spirit_effect)
			cube.GetPlayer()->entity_data.ice_spirit_effect = 5000;

		if (no_stun)
			cube.GetPlayer()->entity_data.stun_timer = 0;

		if (freeze_hp)
			cube.GetPlayer()->entity_data.hp = 1337;

		if (freeze_mp)
			cube.GetPlayer()->entity_data.mp = 1337;

		if (freeze_stamina)
			cube.GetPlayer()->stamina = 100;

		if (freeze_pet_water)
			cube.GetPlayer()->pet_water = 1;

		if (walk_on_air)
		{
			if (cube.GetPlayer()->entity_data.position.Z < last_y)
			{
				cube.GetPlayer()->entity_data.position.Z = last_y;
				cube.GetPlayer()->entity_data.acceleration.Z = 0;
			}
		}
		if (giant_mode && !giant_done)
		{
			*(float *)(cube.offset.CameraDistance) = 30;
			cube.GetPlayer()->entity_data.appearance.physical_size = 7.43;
			cube.GetPlayer()->entity_data.appearance.graphical_size = 3.47;
			giant_done = true;
		}
		else if (!giant_mode && giant_done)
		{
			cube.GetPlayer()->entity_data.appearance.physical_size = 2.16;
			cube.GetPlayer()->entity_data.appearance.graphical_size = 0.96;
			*(float *)(cube.offset.CameraDistance) = 14;
			giant_done = false;
		}
			
		last_y = cube.GetPlayer()->entity_data.position.Z;
		ImGui::End();
	}

	if (debug_creatures)
	{
		ImGui::Begin("Creatures");
		ImGui::Text("Number of creatures : %d", cube.val.num_creatures);
		ImGui::Checkbox("Show on map", &show_on_map);
		ImGui::SameLine();
		ImGui::Checkbox("Show XYZ", &show_xyz);
		if (ImGui::SmallButton("Teleport All Here"))
		{
			for (int i = 0; i < cube.val.num_creatures; i++)
			{
				cube.val.Creatures[0][i]->entity_data.position = cube.GetPlayer()->entity_data.position;
			}
		}
		if (!(selected_creature_num < 0))
		{
			ImGui::Checkbox("Creature Info", &show_selected);
			if (show_selected)
			{
				if (selected_creature_num >= cube.val.num_creatures)
					selected_creature_num = 0;

				selected_creature = cube.val.Creatures[0][selected_creature_num];

				ImGui::Begin("Creature Info");
				ImGui::Text("Name : %s", selected_creature->entity_data.name);
				ImGui::SameLine();
				ImGui::Text("GUID : %lld", selected_creature->GUID);
				ImGui::Text("Class : %s", cube.GetClass(selected_creature->entity_data.char_class));
				ImGui::SameLine();
				ImGui::Text("Spec : %d", selected_creature->entity_data.char_specialization);
				selected_creature_distance = (int)(selected_creature->DistanceFrom(cube.GetPlayer()) / 65535);
				ImGui::Text("Level : %d", selected_creature->entity_data.level);
				ImGui::Text("HP : %.2f", selected_creature->entity_data.hp);
				ImGui::Text("Distance : %d", selected_creature_distance);
				if (ImGui::SmallButton("Show on map"))
				{
					selected_creature->entity_data.appearance.movement_flags |= 0x2000;
				}
				ImGui::Text("Movement flags : %x", std::bitset<32>(selected_creature->entity_data.appearance.movement_flags));
				if(show_xyz)
					ImGui::Text("XYZ : %lld %lld %lld", 
						selected_creature->entity_data.position.X,
						selected_creature->entity_data.position.Y,
						selected_creature->entity_data.position.Z);
				ImGui::Text("Pyhsical size : %f", selected_creature->entity_data.appearance.physical_size); 
				ImGui::SameLine();
				ImGui::SliderFloat("Pyhsical Size", &selected_creature->entity_data.appearance.physical_size, 0.1, 50, "%.2f");
				ImGui::Text("Graphical size : %f", selected_creature->entity_data.appearance.graphical_size);
				ImGui::SameLine();
				ImGui::SliderFloat("Graphical Size", &selected_creature->entity_data.appearance.graphical_size, 0.1, 50, "%.2f");
				
				if (ImGui::SmallButton("Kill"))
				{
					selected_creature->entity_data.hp = -1;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Teleport"))
				{
					cube.GetPlayer()->entity_data.position = selected_creature->entity_data.position;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Teleport Here"))
				{
					selected_creature->entity_data.position = cube.GetPlayer()->entity_data.position;
				}
				ImGui::End();
			}
		}

		ImGui::InputInt("Select ID", &selected_creature_num, 1, 1);

		for (int i = 0; i < cube.val.num_creatures; i++)
		{
			if (show_on_map)
			{
				selected_creature->entity_data.appearance.movement_flags |= 0x2000;
			}

			if (show_xyz)
			{
				ImGui::Text("#%d - Name : %s, Distance : %d, Level : %d, HP : %f, XYZ : %lld %lld %lld",
					i,
					cube.val.Creatures[0][i]->entity_data.name,
					(int)(cube.val.Creatures[0][i]->DistanceFrom(cube.GetPlayer()) / 65535),
					cube.val.Creatures[0][i]->entity_data.level,
					cube.val.Creatures[0][i]->entity_data.hp,
					cube.val.Creatures[0][i]->entity_data.position.X,
					cube.val.Creatures[0][i]->entity_data.position.Y,
					cube.val.Creatures[0][i]->entity_data.position.Z);
			}
			
			else 
			{
				ImGui::Text("#%d - Name : %s, Distance : %d, Level : %d, HP : %f",
					i,
					cube.val.Creatures[0][i]->entity_data.name,
					(int)(cube.val.Creatures[0][i]->DistanceFrom(cube.GetPlayer()) / 65535),
					cube.val.Creatures[0][i]->entity_data.level,
					cube.val.Creatures[0][i]->entity_data.hp);
			}
		}
		
		ImGui::End();
	}

	if (edit_pos)
	{
		cur_pos = cube.GetPlayer()->entity_data.position;
		if (!edit_pos_init)
		{
			x = cur_pos.X;
			y = cur_pos.Y;
			z = cur_pos.Z;
			edit_pos_init = true;
		}
		ImGui::Begin("Edit Pos");

		ImGui::Text("Current Pos : %lld - %lld - %lld", cur_pos.X, cur_pos.Y, cur_pos.Z);

		// 1 block - 100 blocks
		ImGui::InputInt("X", &x, 65535, 6553500);
		ImGui::InputInt("Y", &y, 65535, 6553500);
		ImGui::InputInt("Z", &z, 65535, 6553500);

		if (ImGui::SmallButton("Teleport"))
		{
			((Creature*)cube.CPlayerPtr)->entity_data.position.X = x;
			((Creature*)cube.CPlayerPtr)->entity_data.position.Y = y;
			((Creature*)cube.CPlayerPtr)->entity_data.position.Z = z;
		}

		if (ImGui::SmallButton("Set Waypoint"))
		{
			x = cur_pos.X;
			y = cur_pos.Y;
			z = cur_pos.Z;
		}
		ImGui::End();
	}
	ImGui::EndFrame();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(pDevice);
}

void Hook()
{
	HWND  window = FindWindowA(NULL, windowName);
	oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
	void * d3d9Device[119];

	if (GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
	{
		std::cout << "[+] Found DirectXDevice vTable at: ";
		std::cout << std::hex << d3d9Device[0] << "\n";

		std::cout << "[+] Trying to hook functions..\n";
		//dx9Device = (IDirect3DDevice9 *)d3d9Device;
		oDrawIndexedPrimitive = (f_DrawIndexedPrimitive)DetourFunction((PBYTE)d3d9Device[82], (PBYTE)Hooked_DrawIndexedPrimitive);
		oReset = (f_Reset)DetourFunction((PBYTE)d3d9Device[16], (PBYTE)Hooked_Reset);
		oEndScene = (f_EndScene)DetourFunction((PBYTE)d3d9Device[42], (PBYTE)Hooked_EndScene);
	}
}


LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}


void main()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	Sleep(4000);	// wait for GameController to be set

	Hook();
	cube = CubeX();
	cube.PrintOffsets();
}

extern "C" __declspec(dllexport) bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)main, NULL, NULL, NULL);
		break;
	}
	return true;
}
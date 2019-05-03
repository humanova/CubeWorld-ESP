#pragma warning(disable : 4996)
#include "CubeX.h"
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
bool debug_creatures = false;
bool show_xyz = true;
bool debug_wireframe = false;
int wireframe_stride = 24;
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
bool WorldToScreen(vec3_l pos, vec2 &screen, float matrix[16], int windowWidth, int windowHeight)
{
	printf("WS2 === \n POS : %lld - %lld - %lld\n", pos.x, pos.y, pos.z);
	printf("matrix : \n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n", matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);
	printf("window : %d x %d", windowWidth, windowHeight);
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	vec4 clipCoords;
	clipCoords.x = (float)(pos.x*matrix[0]) + (float)(pos.y*matrix[1]) + (float)(pos.z*matrix[2]) + matrix[3];
	clipCoords.y = (float)(pos.x*matrix[4]) + (float)(pos.y*matrix[5]) + (float)(pos.z*matrix[6]) + matrix[7];
	clipCoords.z = (float)(pos.x*matrix[8]) + (float)(pos.y*matrix[9]) + (float)(pos.z*matrix[10]) + matrix[11];
	clipCoords.w = (float)(pos.x*matrix[12]) + (float)(pos.y*matrix[13]) + (float)(pos.z*matrix[14]) + matrix[15];

	printf("clipCoords : %f\n", clipCoords.w);
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

void DrawESP()
{
	
	vec2 Screen;
	vec3_l Pos;
	D3DVIEWPORT9 viewPort;
	D3DXMATRIX view;
	//printf("Getting viewMatrix...\n");
	dx9Device->GetTransform(D3DTS_VIEW, &view);
	//printf("Getting viewPort...\n");
	dx9Device->GetViewport(&viewPort);
	//printf("ResfreshVal() call...\n");
	//cube.RefreshVal();
	//printf("Refreshed Creatures :: count : %d\n", cube.val.Creatures->size());
	for (int i = 0; i < cube.val.num_creatures; i++)
	{
		if (cube.val.Creatures[0][i]->GUID == cube.val.LocalPlayer->GUID)
		{
			/*
			vec3_l c_pos;
			c_pos.x = cube.val.Creatures[0][i]->entity_data.position.X;
			c_pos.y = cube.val.Creatures[0][i]->entity_data.position.Y;
			c_pos.z = cube.val.Creatures[0][i]->entity_data.position.Z;
			printf("Continuing... Creature Pos: %ld - %ld - %ld\n", c_pos.x, c_pos.y, c_pos.z);
			*/
			continue;
		}
			
		Pos.x = cube.val.Creatures[0][i]->entity_data.position.X;
		Pos.y = cube.val.Creatures[0][i]->entity_data.position.Y;
		Pos.z = cube.val.Creatures[0][i]->entity_data.position.Z;

		if (WorldToScreen(Pos, Screen, view, viewPort.Width, viewPort.Height))
		{
			printf("Called W2S :: Screen Out : %f - %f\n", Screen.x, Screen.y);
			//int height = Screen.y - 5;
			//int width = height / 2;
			printf("Calling Drawline...\n", Screen.x, Screen.y);
			DrawLine(Screen.x, Screen.y, viewPort.X / 2, viewPort.Y / 2, 2, true, RED(255));
			//Draw.Box(Screen.x - (width / 2), Screen.y - height, width, height, 2, RED(255));
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
	if (imgui_init) {
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

	if(is_esp || debug_creatures)
		cube.RefreshVal();

	if (is_esp)
	{
		DrawESP();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Text("Hello Cube World Discord!");
	ImGui::Checkbox("ESP test", &is_esp);
	ImGui::Checkbox("Debug wireframe", &debug_wireframe);
	ImGui::Checkbox("Debug Creatures", &debug_creatures);

	if (debug_wireframe)
	{
		ImGui::Begin("Wireframe Stuff");
		ImGui::Checkbox("Wireframe", &is_wireframe);
		ImGui::SameLine();
		ImGui::Checkbox("All", &wireframe_everything);
		if (!wireframe_everything)
		{
			ImGui::SliderInt("pStride", &wireframe_stride, 3, 1000); ImGui::SameLine(); ImGui::InputInt("strds", &wireframe_stride, 1, 20);
			ImGui::SliderInt("numVertices", &wireframe_vertices, 3, 5000);  ImGui::SameLine(); ImGui::InputInt("verts", &wireframe_vertices, 1, 20);
		}
		ImGui::End();
	}

	if (debug_creatures)
	{
		ImGui::Begin("Creatures");
		ImGui::Text("Number of creatures : %d", cube.val.num_creatures);
		ImGui::Checkbox("Show XYZ", &show_xyz);
		
		for (int i = 0; i < cube.val.num_creatures; i++)
		{
			if (ImGui::SmallButton("Teleport"))
			{
				((Creature*)cube.CPlayerPtr)->entity_data.position = cube.val.Creatures[0][i]->entity_data.position;
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("Teleport Here"))
			{
				cube.val.Creatures[0][i]->entity_data.position = cube.GetPlayer()->entity_data.position;
			}
			ImGui::SameLine();
			if (show_xyz)
			{
				ImGui::Text("#%d - GUID : %lld, HP : %f, Level : %d, XYZ : %lld %lld %lld, Physical Size : %f",
					i,
					cube.val.Creatures[0][i]->GUID,
					cube.val.Creatures[0][i]->entity_data.hp,
					cube.val.Creatures[0][i]->entity_data.level,
					cube.val.Creatures[0][i]->entity_data.position.X,
					cube.val.Creatures[0][i]->entity_data.position.Y,
					cube.val.Creatures[0][i]->entity_data.position.Z,
					cube.val.Creatures[0][i]->entity_data.appearance.physical_size);
			}
			
			else 
			{
				ImGui::Text("#%d - GUID : %lld, HP : %f, Level : %d, Physical Size : %f",
					i,
					cube.val.Creatures[0][i]->GUID,
					cube.val.Creatures[0][i]->entity_data.hp,
					cube.val.Creatures[0][i]->entity_data.level,
					cube.val.Creatures[0][i]->entity_data.appearance.physical_size);
			}

		}
		
		ImGui::End();
	}
	ImGui::EndFrame();


	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	//delete(cube.val.Creatures);
	//delete(cube.val.LocalPlayer);
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

	switch (uMsg)
	{
		// .... your other stuff
	case WM_SETFOCUS:
	{
		if (dx9Device)
		{
			//Hook();
		}

	} break;

	default: break;
	}

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}


void main()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	Sleep(7000);

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
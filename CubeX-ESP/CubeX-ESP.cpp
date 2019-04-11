#pragma warning(disable : 4996)
#include "CubeX.h"
#include "dx9.h"

#include <detours.h>
#pragma comment(lib,"detours.lib")

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

using namespace std;


const char* windowName = "Cube";
WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(_stdcall * f_EndScene)(IDirect3DDevice9 * pDevice);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
f_EndScene oEndScene;
IDirect3DDevice9 * dx9Device;

bool is_wireframe = false;

void Wireframe()
{
	IDirect3DVertexBuffer9* pStreamData = NULL;
	UINT pOffsetInBytes, pStride;
	dx9Device->GetStreamSource(0, &pStreamData, &pOffsetInBytes, &pStride);

	if (pStride == 24 || pStride == 28)//Wireframe Example
	{
		dx9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
}

HRESULT _stdcall Hooked_EndScene(IDirect3DDevice9 * pDevice)
{
	//D3DRECT BarRect = { 100, 100, 200, 200 };
	//pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 1, 1, 1), 0.0f, 0);
	
	if (is_wireframe)
		Wireframe();

	static bool init = true;
	if (init)
	{
		init = false;
		dx9Device = pDevice;
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplWin32_Init(FindWindowA(NULL, windowName));
		ImGui_ImplDX9_Init(pDevice);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	ImGui::Text("Hello Cube World Discord!");
	//ImGui::Checkbox("Wireframe test", &is_wireframe);
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

		std::cout << "[+] Trying to hook function..\n";

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

	CubeX cube = CubeX();

	cube.PrintOffsets();
	while (true)
	{
		cube.RefreshVal();
		cout << "creatures near : " << cube.val.num_creatures << endl;
		delete(cube.val.Creatures);
		Sleep(20);
	}
	

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
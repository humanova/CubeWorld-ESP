#include "dx9.h"

#pragma comment(lib,"d3d9.lib")

bool GetD3D9Device(void ** pTable, size_t Size)
{
	std::cout << "[*] Trying to get DirectXDevice by new_creation method..\n";

	if (!pTable)
	{
		std::cout << "[-] pTable not set.\n";
		return false;
	}


	std::cout << "[*] Right before Direct3DCreate9.\n";
	IDirect3D9 * pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
	{
		std::cout << "[-] Direct3DCreate9 failed.\n";
		return false;
	}
	std::cout << "[+] Direct3DCreate9 successfull.\n";

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetForegroundWindow();
	d3dpp.Windowed = ((GetWindowLong(d3dpp.hDeviceWindow, GWL_STYLE) & WS_POPUP) != 0) ? FALSE : TRUE;;

	IDirect3DDevice9 * pDummyDevice = nullptr;
	HRESULT create_device_ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);
	std::cout << "[*] Return of CreateDevice:\n";
	std::cout << create_device_ret;
	std::cout << "\n";
	if (!pDummyDevice || FAILED(create_device_ret))
	{
		std::cout << "[-] CreateDevice failed\n";
		pD3D->Release();
		return false;
	}
	std::cout << "[+] CreateDevice successfull.\n";

	memcpy(pTable, *reinterpret_cast<void***>(pDummyDevice), Size);

	pDummyDevice->Release();
	pD3D->Release();

	std::cout << "[+] Success!\n";
	return true;
}
#include "proc.h"

DWORD GetProcId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));

		}
	}
	CloseHandle(hSnap);
	return procId;
}
 

DWORD GetModuleBase(const wchar_t * ModuleName, DWORD ProcessId) {
	// This structure contains lots of goodies about a module
	MODULEENTRY32 ModuleEntry = { 0 };
	// Grab a snapshot of all the modules in the specified process
	HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessId);

	if (!SnapShot)
		return NULL;

	// You have to initialize the size, otherwise it will not work
	ModuleEntry.dwSize = sizeof(ModuleEntry);

	// Get the first module in the process
	if (!Module32First(SnapShot, &ModuleEntry))
		return NULL;

	do {
		// Check if the module name matches the one we're looking for
		if (!wcscmp(ModuleEntry.szModule, ModuleName)) {
			// If it does, close the snapshot handle and return the base address
			CloseHandle(SnapShot);
			return (DWORD)ModuleEntry.modBaseAddr;
		}
		// Grab the next module in the snapshot
	} while (Module32Next(SnapShot, &ModuleEntry));

	// We couldn't find the specified module, so return NULL
	CloseHandle(SnapShot);
	return NULL;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}


#include <iostream>
#include "mem.h"

DWORD WINAPI HackThread(HMODULE hModule)
{
	// Create Console
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "AssaultCube Internal Cheat \n";
	std::cout << "Press F1 to toggle Godmode \n";
	std::cout << "Press F2 to toggle Armor \n";
	std::cout << "Press F3 to toggle Unlimited Ammo \n";
	std::cout << "Press F4 to toggle No Recoil \n";

	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

	// Calling it with NULL also gives you the Address of the .exe Module
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	bool bHealth = false, bArmor = false, bAmmo = false, bRecoil = false;

	while (true)
	{
		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			break;
		}

		if (GetAsyncKeyState(VK_F1) & 1)
		{
			bHealth = !bHealth;
		}

		if (GetAsyncKeyState(VK_F2) & 1)
		{
			bArmor = !bArmor;
		}

		if (GetAsyncKeyState(VK_F3) & 1)
		{
			bAmmo = !bAmmo;
		}

		// No Recoil NOP
		if (GetAsyncKeyState(VK_F4) & 1)
		{
			bRecoil = !bRecoil;

			if (bRecoil)
			{
				mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
			}

			else
			{
				// 50 8D 4C 24 1C 51 8B CE FF D2 Original Recoil Assembly Instructions
				mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
			}
		}

		// Need to use uintptr_t for pointer arithmetic later
		uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10F4F4);

		// Continuous Write/Freeze

		if (localPlayerPtr)
		{
			if (bHealth)
			{
				*(int*)(*localPlayerPtr + 0xF8) = 100;
			}

			if (bArmor)
			{
				*(int*)(*localPlayerPtr + 0xFC) = 100;
			}

			if (bAmmo)
			{
				*(int*)mem::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 }) = 1;
			}
		}
		Sleep(1);
	}

	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
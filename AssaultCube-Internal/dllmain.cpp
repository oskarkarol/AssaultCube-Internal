#include <iostream>
#include "mem.h"

uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

bool bHealth = false;
bool bArmor = false;
bool bAmmo = false;
bool bRecoil = false;
bool bRapidFire = false;
bool bFly = false;
bool bNoScope = false;
bool bMap = false;

typedef BOOL(__stdcall* twglSwapBuffers) (HDC hDc);

twglSwapBuffers owglSwapBuffers;

BOOL __stdcall hkwglSwapBuffers(HDC hDc)
{
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
			// NOP
			mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
		}

		else
		{
			// 50 8D 4C 24 1C 51 8B CE FF D2 Original Recoil Assembly Instructions
			mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
		}
	}

	// Rapid Fire NOP
	if (GetAsyncKeyState(VK_F5) & 1)
	{
		bRapidFire = !bRapidFire;

		if (bRapidFire)
		{
			// NOP Assembly
			mem::Nop((BYTE*)(moduleBase + 0x637E4), 2);
		}

		else
		{
			// Restore Original Instructions
			mem::Patch((BYTE*)(moduleBase + 0x637E4), (BYTE*)"\x89\x0A", 2);
		}
	}

	// Flyhack
	if (GetAsyncKeyState(VK_F6) & 1)
	{
		bFly = !bFly;

		if (bFly)
		{
			// Patch
			mem::Patch((BYTE*)0x45ADD8, (BYTE*)"\x01", 1);
		}

		else
		{
			// Restore
			mem::Patch((BYTE*)0x45ADD8, (BYTE*)"\x00", 1);
		}
	}

	// NoScope
	if (GetAsyncKeyState(VK_F7) & 1)
	{
		bNoScope = !bNoScope;

		if (bNoScope)
		{
			// Nop
			mem::Nop((BYTE*)0x463CEB, 5);
		}

		else
		{
			// Restore
			mem::Patch((BYTE*)0x463CEB, (BYTE*)"\xE8\x90\x43\xFA\xFF", 5);
		}
	}

	// Map Hack
	if (GetAsyncKeyState(VK_F8) & 1)
	{
		bMap = !bMap;

		if (bMap)
		{
			// Bypass gameMode checking
			mem::Nop((BYTE*)0x0040968F, 6);

			// Map show ALL
			mem::Nop((BYTE*)0x04096A1, 6);

			// Bypass gameMode checking on radar
			mem::Nop((BYTE*)0x00409FA1, 6);

			// Radar show ALL
			mem::Nop((BYTE*)0x0409FB3, 6);
		}

		else
		{
			// Rstore gameMode checking
			mem::Patch((BYTE*)0x0040968F, (BYTE*)"\x0F\x85\x65\x01\x00\x00", 6);

			// Restore Map
			mem::Patch((BYTE*)0x04096A1, (BYTE*)"\x0f\x85\x53\x01\x00\x00", 6);

			// Restore Radar gamemodeChecking?
			mem::Patch((BYTE*)0x00409FA1, (BYTE*)"\x0F\x85\x5C\x01\x00\x00", 6);

			// Restore Radar
			mem::Patch((BYTE*)0x0409FB3, (BYTE*)"\x0F\x85\x4a\x01\x00\x00", 6);
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

    return owglSwapBuffers(hDc);
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    // Hook
    owglSwapBuffers = (twglSwapBuffers)GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");
    owglSwapBuffers = (twglSwapBuffers)mem::TrampHook32((BYTE*)owglSwapBuffers, (BYTE*)hkwglSwapBuffers, 5);

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
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "Install.hpp"
#include "Utils.hpp"
#include "MinHook/minhook.hpp"

using RtlAdjustPrivilege_t = NTSTATUS ( __stdcall* ) ( ULONG, BOOLEAN, BOOLEAN, PBOOLEAN );

DWORD __stdcall UnhookListener(LPVOID)
{
	HWND hConsole = GetConsoleWindow();
	while (true)
	{
		if ( (GetAsyncKeyState(VK_SPACE) & 1 ) && GetForegroundWindow() == hConsole)
		{
			printf( "	unhooking clarity steam...\n" );
			FreeConsole();
			MH_DisableHook(MH_ALL_HOOKS);
			break;
		}
		Sleep(1000);
	}
	return 0;
}

BOOL __stdcall DllMain( HMODULE hModule, DWORD ulReason, LPVOID lpReserved )
{
	AllocConsole( );
	freopen_s( reinterpret_cast< FILE** >( stdin ), "CONIN$", "r", stdin );
	freopen_s( reinterpret_cast< FILE** >( stdout ), "CONOUT$", "w", stdout );
	SetConsoleTitleA( "clarity.tk v1 steam mapper [ft.Ex3zen]" );

	if ( ulReason != DLL_PROCESS_ATTACH )
		return 0;

	HMODULE hNtDll = GetModuleHandleA( "ntdll.dll" );
	if ( !hNtDll )
		return 1;

	RtlAdjustPrivilege_t fnRtlAdjustPrivilege = reinterpret_cast< RtlAdjustPrivilege_t >( 
		GetProcAddress( hNtDll, "RtlAdjustPrivilege" ) );
	if ( !fnRtlAdjustPrivilege )
		return 1;
	
	BOOLEAN bEnabled;
	fnRtlAdjustPrivilege( 20, 1, 0, &bEnabled );

	g_registry = CRegistry();
	HMODULE hAdvApi = LoadLibraryA( "ADVAPI32.dll" );
	if ( !hAdvApi )
		return 1;

	g_registry.m_fnRegCloseKey = reinterpret_cast< RegCloseKey_t >( GetProcAddress( hAdvApi, "RegCloseKey" ) );
	g_registry.m_fnRegOpenKeyExA = reinterpret_cast< RegOpenKeyExA_t >( GetProcAddress( hAdvApi, "RegOpenKeyExA" ) );
	g_registry.m_fnRegQueryValueExA = reinterpret_cast< RegQueryValueExA_t >( GetProcAddress( hAdvApi, "RegQueryValueExA" ) );
	g_registry.m_fnRegSetValueExA = reinterpret_cast< RegSetValueExA_t >( GetProcAddress( hAdvApi, "RegSetValueExA" ) );

	CInstall* pInstall = new CInstall( );
	pInstall->Init( );
	delete pInstall;

	printf("press SPACE unhook.\n");
	CreateThread( 0, 0, UnhookListener, 0, 0, 0 );
	return 1;
}

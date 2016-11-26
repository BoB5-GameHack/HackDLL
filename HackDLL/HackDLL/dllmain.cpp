// dllmain.cpp: DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "HackDLL.h"

#include "SendHook.h"
#include "RecvHook.h"

#include "PlainPacketHook.h"

// extern variable - used by "PlayMacro" function
HWND GameWindow = NULL;

BOOL APIENTRY DllMain (HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		//MessageBoxW(NULL, L"Attached", L"HackDLL", MB_OK);
		CreateConsoleIO();

		PatchSend(); //WS2_32.send
		PatchRecv(); //WSOCK32.recv

		GameWindow = GameWindowsInfo(); //"EnumWindows" to get Main game window
		HookPlainPacket(); // VEH handling to hook functions
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}


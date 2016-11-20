#include "stdafx.h"
#include "HackDLL.h"
#include "ChatHook.h"

#include "Packets.h"
#include "SendHook.h"
#include "RecvHook.h"


#define CHAT_DATA_INDEX 54

char filename[1024] = { 0, };

int ChatHook(unsigned char *packet) {
	WCHAR* ptr = wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), '<');
	*ptr = NULL;

	if (isPlaying) {
		if (wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), 'm')) {
			printf("[*] stopping macro\n");

			isPlaying = false;
			*ptr = '<';
		}
		else {
			int tmp = 0;
			PacketReplacer(packet, &tmp);
		}
	}
	else if (wcsstr((WCHAR *)(packet + CHAT_DATA_INDEX), L"macro ")) {
		WCHAR *ptrb = wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), ' ');
		sprintf(filename, "%ls", ptrb + 1);

		if (!access(filename, 0)) {
			printf("[*] starting macro : %s\n", filename);

			isPlaying = true;

			int miliSeconds = 5000;
			CreateRemoteThread(GetCurrentProcess(), NULL, 0, (LPTHREAD_START_ROUTINE)PlayMacro, &miliSeconds, 0, NULL);
		}

		*ptr = '<';
	}
	else if (wcsstr((WCHAR *)(packet + CHAT_DATA_INDEX), L"packets ")) {
		WCHAR *ptrb = wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), ' ');
		sprintf(filename, "%ls", ptrb + 1);

		if (!access(filename, 0)) {
			printf("[*] replacing packets : %s\n", filename);
			isHooked = true;
		}

		*ptr = '<';
	}
	else if (wcsstr((WCHAR *)(packet + CHAT_DATA_INDEX), L"modify_chat ")) {
		WCHAR *ptrb = wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), ' ');
		SIZE_T origlen = wcslen((WCHAR *)(packet + CHAT_DATA_INDEX));
		SIZE_T newlen = wcslen(ptrb + 1);

		printf("[*] chat modifying : %ls\n", ptrb + 1);

		unsigned char chat[1024] = { 0, };
		memcpy(chat, packet, CHAT_DATA_INDEX);
		memcpy(chat + CHAT_DATA_INDEX, ptrb + 1, newlen * 2);

		newlen = EntityReplacer((WCHAR *)(chat + CHAT_DATA_INDEX), L"&lt;", '<', newlen);
		newlen = EntityReplacer((WCHAR *)(chat + CHAT_DATA_INDEX), L"&gt;", '>', newlen);
		newlen = EntityReplacer((WCHAR *)(chat + CHAT_DATA_INDEX), L"&quot;", '\"', newlen);

		*ptr = '<';
		memcpy(chat + CHAT_DATA_INDEX + newlen * 2, ptr, *packet - CHAT_DATA_INDEX - origlen * 2);
		*chat = *packet - origlen * 2 + newlen * 2;

		memcpy(packet, chat, *chat);
	}
	else if (wcsstr((WCHAR *)(packet + CHAT_DATA_INDEX), L"unload")) {
		printf("[*] unloading library..\n");

		CONTEXT ctx;
		memset(&ctx, 0, sizeof(CONTEXT));
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

		ctx.Dr0 = 0;
		ctx.Dr1 = 0;
		ctx.Dr2 = 0;
		ctx.Dr3 = 0;

		ctx.Dr7 = 0;

		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetMainThreadId());

		SetThreadContext(hThread, &ctx);
		CloseHandle(hThread);

		BYTE code[] = { 0x4C, 0x8B, 0xDC, 0x48, 0x83 };
		WriteMemory(addrRecv, code, sizeof(code));

		BYTE code2[] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
		WriteMemory(addrSend, code2, sizeof(code2));

		HMODULE hCurrentLibrary = GetModuleHandleW(L"HackDLL.dll");
		FreeLibrary(hCurrentLibrary);
	}
	else {
		*ptr = '<';
	}

	return 0;
}
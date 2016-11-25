#include "stdafx.h"
#include "SendHook.h"

bool isPlainSendPacket = true;

LPVOID addrSend = NULL;
BYTE sendJmper[5] = { 0xE9, };

int WINAPI SendHook(SOCKET s, const char *buf, int len, int flags);

int PatchSend() {
	HMODULE hMod = LoadLibraryW(L"WS2_32.dll");
	addrSend = GetProcAddress(hMod, "send");

	printf("[*] send : %p\n", addrSend);

	*(LPDWORD)(sendJmper + 1) = (LPBYTE)SendHook - (LPBYTE)addrSend - 5;
	WriteMemory(addrSend, sendJmper, sizeof(sendJmper));
	
	return 0;
}

typedef int (WINAPI *pSend)(
	SOCKET s,
	const char*buf,
	int len,
	int flags
	);

int WINAPI SendHook(SOCKET s, const char *buf, int len, int flags) {
	
	/*
	if (isPlainSendPacket) {
		printf("[*] WS2_32.send : ");
		for (int i = 0; i < len; ++i) {
			printf("%02X ", (unsigned char)buf[i]);
		}
		printf("\n\n");
	}
	else {
		printf("[*] sended\n\n");
	}
	*/
	BYTE code[] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
	WriteMemory(addrSend, code, sizeof(code));
	
	int ret = ((pSend)addrSend)(s, buf, len, flags);	
	WriteMemory(addrSend, sendJmper, sizeof(sendJmper));
	
	isPlainSendPacket = true;
	return ret;
}
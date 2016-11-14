#include "stdafx.h"
#include "RecvHook.h"

bool isPlainRecvPacket = true;
bool isOmitPacket = false;

LPVOID addrRecv = NULL;
BYTE recvJmper[5] = { 0xE9, };

int WINAPI RecvHook(SOCKET s, const char *buf, int len, int flags);

int PatchRecv() {
	HMODULE hMod = LoadLibraryW(L"WSOCK32.dll");
	addrRecv = GetProcAddress(hMod, "recv");

	printf("[*] recv : %p\n", addrRecv);

	*(LPDWORD)(recvJmper + 1) = (LPBYTE)RecvHook - (LPBYTE)addrRecv - 5;
	WriteMemory(addrRecv, recvJmper, sizeof(recvJmper));

	return 0;
}

typedef int (WINAPI *pRecv)(
	SOCKET s,
	const char*buf,
	int len,
	int flags
	);

int WINAPI RecvHook(SOCKET s, const char *buf, int len, int flags) {
	BYTE code[] = { 0x4C, 0x8B, 0xDC, 0x48, 0x83 };
	WriteMemory(addrRecv, code, sizeof(code));

	int ret = ((pRecv)addrRecv)(s, buf, len, flags);
	WriteMemory(addrRecv, recvJmper, sizeof(recvJmper));

	if (isPlainRecvPacket) {
		printf("[*] WSOCK32.recv : ");
		for (int i = 0; i < ret; ++i) {
			printf("%02X ", (unsigned char)buf[i]);
		}
		printf("\n\n");
	}
	else if (!isOmitPacket) {
		printf("[*] recved\n\n");
	}

	isPlainRecvPacket = true;
	isOmitPacket = false;
	return ret;
}
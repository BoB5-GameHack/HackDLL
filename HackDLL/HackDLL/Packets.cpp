#include "stdafx.h"
#include "Packets.h"
#include "HackDLL.h"

#include "RecvHook.h"
#include "ChatHook.h"
#include "PlainPacketHook.h"

bool isHooked = false;
bool isPlaying = false;
DWORD64 PacketSendNumber = 0;
DWORD64 PacketRecvNumber = 0;

int PacketDumper(unsigned char *packetAddr, SIZE_T size, bool flag) {
	if (flag) {
		printf("[%d] plain send : ", ++PacketSendNumber);
	}
	else {
		if (!memcmp(packetAddr, "\x2D\x00\x41\x51", 4)) {
			isOmitPacket = true;
			return 0;
		}
		printf("[%d] plain recv : ", ++PacketRecvNumber);
	}

	for (int i = 0; i < size; ++i) {
		printf("%02X ", packetAddr[i]);
	}
	printf("\n[*] dump : ");
	for (int i = 0; i < size; ++i) {
		printf("%c", packetAddr[i]);
	}
	printf("\n");

	return 0;
}


BYTE **packets;
int curSet = 0;
int totalSet = 0;

int spec = 0;

int PacketReplacer(unsigned char *data, int *length) {
	if (curSet >= totalSet) {
		if (totalSet) {
			while (totalSet) {
				delete[] packets[--totalSet];
			}

			delete[] packets;
		}

		curSet = 0;
		totalSet = 0;

		FILE *fp = fopen(filename, "r");
		fscanf(fp, "%d", &totalSet);

		printf("[*] totalSet : %d..\n", totalSet);
		packets = new PBYTE[totalSet];

		if (totalSet == 1) {
			int cnt = -1;
			PBYTE pca = packets[0] = new BYTE[65535];
			
			while (!feof(fp)) {
				fscanf(fp, "%02X", &pca[++cnt]);
			}

			spec = cnt + 1;
		}
		else {
			for (int i = 0; i < totalSet; ++i) {
				BYTE _size[2] = { 0, };
				fscanf(fp, "%02X %02X", &_size[0], &_size[1]);

				WORD size = *((PWORD)_size);

				PBYTE pca = packets[i] = new BYTE[size];
				pca[0] = _size[0];
				pca[1] = _size[1];

				for (int j = 2; j < size; ++j) {
					fscanf(fp, "%02X", &pca[j]);
				}
			}
		}

		fclose(fp);
	}

	WORD len = 0;
	
	if (spec) {
		len = spec;
	}
	else {
		len = ((PWORD)(packets[curSet]))[0];
	}
	printf("[%d] CurSet (length : %d) Replacing..\n", curSet, len);

	memcpy(data, packets[curSet], len);
	*length = len;

	++curSet;
	if (curSet >= totalSet) {
		isHooked = false;
	}

	spec = 0;
	return len;
}

int EntityReplacer(WCHAR *str, WCHAR *target, WCHAR chr, int len) {
	int tarlen = wcslen(target);
	WCHAR *tmp = wcsstr((WCHAR *)str, target);

	while (tmp) {
		*tmp = chr;
		wcscpy(tmp + 1, tmp + tarlen);

		len -= tarlen - 1;

		tmp = wcsstr(tmp + 1, target);
	}

	return len;
}

int PlayMacro(int *miliSeconds) {
	int ms = *miliSeconds;
	UINT MappedKey_Return = MapVirtualKeyW(VK_RETURN, MAPVK_VK_TO_VSC);
	UINT MappedKey_A = MapVirtualKeyW(0x41, MAPVK_VK_TO_VSC);

	printf("[*] plz input 'a' to start..\n");

	while (isPlaying) {
		PostMessageW(GameWindow, WM_KEYDOWN, VK_RETURN, MappedKey_Return);
		Sleep(100);
		PostMessageW(GameWindow, WM_KEYUP, VK_RETURN, MappedKey_Return);
		Sleep(100);

		PostMessageW(GameWindow, WM_KEYDOWN, 0x41, MappedKey_A);
		Sleep(100);
		PostMessageW(GameWindow, WM_KEYUP, 0x41, MappedKey_A);
		Sleep(100);

		PostMessageW(GameWindow, WM_KEYDOWN, VK_RETURN, MappedKey_Return);
		Sleep(100);
		PostMessageW(GameWindow, WM_KEYUP, VK_RETURN, MappedKey_Return);

		Sleep(ms);
	}
	
	return 0;
}
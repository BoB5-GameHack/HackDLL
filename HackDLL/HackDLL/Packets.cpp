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

int PacketReplacer(unsigned char *data, int *length) {
	if (curSet >= totalSet) {
		curSet = 0;
		totalSet = 0;

		FILE *fp = fopen(filename, "r");
		fscanf(fp, "%d", &totalSet);

		printf("[*] totalSet : %d..\n", totalSet);
		packets = new PBYTE[totalSet];

		for (int i = 0; i < totalSet; ++i) {
			BYTE size = 0;
			fscanf(fp, "%02X", &size);

			PBYTE pca = packets[i] = new BYTE[size];
			pca[0] = size;

			for (int j = 1; j < size; ++j) {
				fscanf(fp, "%02X", &pca[j]);
			}
		}

		fclose(fp);
	}

	BYTE len = packets[curSet][0];
	printf("[%d] CurSet (length : %d) Replacing..\n", curSet, len);

	memcpy(data, packets[curSet], len);
	*length = len;

	++curSet;
	if (curSet >= totalSet) {
		isHooked = false;
	}

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

int PlayMacro() {
	HWND GameWindow = GetGameWindow();

	while (isPlaying) {
		SetForegroundWindow(GameWindow);

		keybd_event(VK_RETURN, NULL, NULL, NULL);
		keybd_event(VK_RETURN, NULL, KEYEVENTF_KEYUP, NULL);

		keybd_event(VkKeyScanW('A'), NULL, NULL, NULL);
		keybd_event(VkKeyScanW('A'), NULL, KEYEVENTF_KEYUP, NULL);

		keybd_event(VK_RETURN, NULL, NULL, NULL);
		keybd_event(VK_RETURN, NULL, KEYEVENTF_KEYUP, NULL);

		Sleep(1000);
	}
	
	return 0;
}
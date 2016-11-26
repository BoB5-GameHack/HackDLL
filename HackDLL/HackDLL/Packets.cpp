#include "stdafx.h"
#include "Packets.h"
#include "HackDLL.h"

#include "RecvHook.h"
#include "ChatHook.h"
#include "PlainPacketHook.h"

bool isHooked = false;
bool isMacroOn = false;
bool isGettingLoc = FALSE;
bool battletime = FALSE;
bool isEliminated = TRUE;
bool setTarget = FALSE;
bool isPosionIndexSet = FALSE;
bool isPosionApplied = FALSE;

MACROATTACKINFO playerattackinfo = { 0, };
MACROATTACKINFO targetinfo = { 0, };

char teleportbuf[46] = { 0x2E, 0x00, 0x0B, 0x52, 0x41, 0xA6, 0x0F, 0x00, 0, };
char attackbuf[46] = { 0x2E, 0x00, 0x0B, 0x52, 0xDD, 0x8C, 0x0F, 0x00, 0, };
char getitembuf[128] = { 0, };
char posionbuf[12] = { 0x0C, 0x00, 0xC9, 0x56, 0x80, 0x3B, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 };
char lockonbuf[13] = { 0x0D, 0x00, 0x17, 0x52, 0, };
char * ptbuf = NULL;	// plaintext buf
int szsrcbuf;

///////////////////////////////////////////////////////////////////////////////
// Print packets in hex at the allocated console

int PacketSendNumber = -1;
int PacketRecvNumber = -1;

int PrintHexData(unsigned char * data, SIZE_T datalen, bool flag) {
	int line = 0;
	int j = 0;
	int modlen = datalen % 16;
	int szline = (modlen == 0) ? (datalen / 16) : (datalen / 16 + 1);
	int szlastline = (modlen == 0) ? 16 : modlen;

	if (flag) {
		printf("[%d] Send Plaintext ====> : \n", ++PacketSendNumber);
	}
	else {
		printf("[%d] Recv Plaintext <==== : \n", ++PacketRecvNumber);
	}

	printf("Data length : %d\n", datalen);

	for (line = 0; line < szline; line++) {
		printf("	%04X	", line * 16);

		if (line == szline - 1) {	//last line
			for (j = 0; j < szlastline; j++)
				printf("%02X ", data[line * 16 + j]);

			for (j = 0; j < 16 - szlastline; j++)
				printf("   ");

			printf("	");

			for (j = 0; j < szlastline; j++) {
				if (32 <= data[line * 16 + j] && data[line * 16 + j] <= 126)
					printf("%c", data[line * 16 + j]);
				else
					printf(".");
			}
		}

		else {
			for (j = 0; j < 16; j++) {
				printf("%02X ", data[line * 16 + j]);
			}

			printf("	");

			for (j = 0; j < 16; j++) {
				if (32 <= data[line * 16 + j] && data[line * 16 + j] <= 127)
					printf("%c", data[line * 16 + j]);
				else
					printf(".");
			}
		}

		printf("\n");
	}
	printf("\n");

	return 0;
}

int ParseRecvData(char * data, SIZE_T totalsize) {
	char curbuf[1024];
	char * curptr;
	SIZE_T cursize;
	FILE * fp;

	curptr = data;

	printf("Total Size : %d\n", totalsize);

	while ((curptr - data) < totalsize) {
		cursize = *(unsigned short *)curptr;

		if (isMacroOn == TRUE && isEliminated == FALSE && !memcmp(curptr, "\x3E\x00\x0D\x52", 4) && !memcmp(curptr + 4, targetinfo.Index, 8)) {

			printf("[ Target Attack Info ]\n");

			memcpy((char *)&targetinfo.X, curptr + 16, 4);
			memcpy((char *)&targetinfo.Y, curptr + 20, 4);
			memcpy((char *)&targetinfo.Z, curptr + 24, 4);
			memcpy((char *)&targetinfo.Dir, curptr + 28, 2);

			// recalculate player's attack direction

			playerattackinfo.X = targetinfo.X - 0.3;
			playerattackinfo.Y = targetinfo.Y;
			playerattackinfo.Dir = 0;
			//playerattackinfo.Z = targetinfo.Z + 0.01;

			//playerattackinfo.Dir = targetinfo.Dir >= 0 ? (targetinfo.Dir - 32768) : (32768 + targetinfo.Dir);

			//*(attackbuf + 20) = (short)playerattackinfo.Dir;
			memcpy(attackbuf + 8, (char *)&playerattackinfo.X, 4);
			memcpy(attackbuf + 12, (char *)&playerattackinfo.Y, 4);

			memcpy(attackbuf + 20, (char *)&playerattackinfo.Dir, 2);

			battletime = TRUE;	// battle started.
			printf("Resetting Attack Direction..\n");

			PrintHexData((unsigned char *)curptr, cursize, FALSE);
		}
		else if (isMacroOn == TRUE && !memcmp(curptr, "\x0E\x00\xC2\x56", 4)) {
			printf("[ Posion Appied... ]\n");
			isPosionApplied = TRUE;
			PrintHexData((unsigned char *)curptr, cursize, FALSE);
		}

		else if (isGettingLoc == FALSE && !memcmp(curptr, "\x2D\x00\x41\x51", 4)) {
			;
		}
		else {
			PrintHexData((unsigned char *)curptr, cursize, false);
		}
		curptr += cursize;
	}


	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Replacing packets from the data file

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

int UnitPacketReplacer(unsigned char * data, int * length) {
	BYTE len = ptbuf[0];

	memcpy(data, ptbuf, len);
	isHooked = FALSE;

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

///////////////////////////////////////////////////////////////////////////////
// Thread function, It send keyboard message to game window (extern variable at HackDLL.h)

UINT MappedKey_Return = MapVirtualKeyW(VK_RETURN, MAPVK_VK_TO_VSC);
UINT MappedKey_A = MapVirtualKeyW(0x41, MAPVK_VK_TO_VSC);
UINT MappedKey_Space = MapVirtualKeyW(VK_SPACE, MAPVK_VK_TO_VSC);

int TriggerPacket() { // Trigger Building and Sending Packet.

	PostMessageW(GameWindow, WM_KEYDOWN, VK_RETURN, MappedKey_Return);
	Sleep(300);
	PostMessageW(GameWindow, WM_KEYUP, VK_RETURN, MappedKey_Return);
	Sleep(300);

	PostMessageW(GameWindow, WM_KEYDOWN, VK_SPACE, MappedKey_Space);
	Sleep(300);
	PostMessageW(GameWindow, WM_KEYUP, VK_SPACE, MappedKey_Space);
	Sleep(300);

	PostMessageW(GameWindow, WM_KEYDOWN, VK_RETURN, MappedKey_Return);
	Sleep(300);
	PostMessageW(GameWindow, WM_KEYUP, VK_RETURN, MappedKey_Return);
	Sleep(300);

	PostMessageW(GameWindow, WM_KEYDOWN, VK_SPACE, MappedKey_Space);
	Sleep(300);
	PostMessageW(GameWindow, WM_KEYUP, VK_SPACE, MappedKey_Space);

	return 0;
}

int PlayMacro() {
	FILE * fp;
	char buf[128] = { 0, };

	int i = 0;

	while (isMacroOn) {
		SetForegroundWindow(GameWindow);

		// Battle Cycle Starts...
		// Wait untill get the target
		while (isMacroOn == TRUE && setTarget == FALSE)
			Sleep(300);

		// Teleport character to location adjacent to TARGET
		// sprintf(filename, "D:\\teleport.txt");
		
		ptbuf = teleportbuf;
		TriggerPacket();

		ptbuf = lockonbuf;
		TriggerPacket();


		//printf("[ battletime : %d ]\n", battletime);
		/*
		while (isMacroOn == TRUE && battletime == FALSE) {	// Sleep untill target attack info is captured...
			Sleep(300);
		}*/

		//printf("[ break while... ]\n");

		isEliminated = FALSE;

		//memcpy(attackbuf, "\x2E\x00\x0B\x52", 4);
		//memcpy(attackbuf + 4, "\xDD\x8C\x0F\x00", 4);
		memcpy(attackbuf + 8,  (char *)&playerattackinfo.X, 4);
		memcpy(attackbuf + 12, (char *)&playerattackinfo.Y, 4);
		memcpy(attackbuf + 16, (char *)&playerattackinfo.Z, 4);
		memcpy(attackbuf + 20, (char *)&playerattackinfo.Dir, 2);
		memset(attackbuf + 22, 0, 24);

		//fp = fopen("D:\\attack.txt", "w+");
		//fprintf(fp, "1\n");
		
		//for (i = 0; i < 46; i++)
		//	fprintf(fp, "%02X ", *((unsigned char *)&buf[i]));

		//fclose(fp);

		//sprintf(filename, "D:\\attack.txt");
		
		ptbuf = attackbuf;
		while (isMacroOn == TRUE && isEliminated == FALSE) {
			Sleep(1000);
			TriggerPacket();
		}

		printf("[ Battle Finished... ]\n");


		ptbuf = posionbuf;

		printf("isPosionApplied : %d\n", isPosionApplied);
		while (isMacroOn == TRUE && isPosionApplied == FALSE) {
			TriggerPacket();
			Sleep(500);
		}

		printf("[ Posion is Applied ...]\n");
		isPosionApplied = FALSE;

		
		// Get Item

		isEliminated = TRUE;
		setTarget = FALSE;

		printf("Cycle Finished...\n");
	}

	isPosionIndexSet = FALSE;
	isMacroOn = FALSE;
	printf("[ Macro Finished... tid : %d]\n", GetCurrentThreadId());

	return 0;
}
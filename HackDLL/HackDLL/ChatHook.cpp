#include "stdafx.h"
#include "ChatHook.h"

#define CHAT_DATA_INDEX 54

int EntityReplacer(WCHAR *str, WCHAR *target, WCHAR chr, int len);

bool isHooked = false;
char filename[1024] = { 0, };

int ChatHook(unsigned char *packet) {
	WCHAR* ptr = wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), '<');
	*ptr = NULL;

	if (wcsstr((WCHAR *)(packet + CHAT_DATA_INDEX), L"packets ")) {
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

		char chat[1024] = { 0, };
		memcpy(chat, packet, CHAT_DATA_INDEX);
		memcpy(chat + CHAT_DATA_INDEX, ptrb + 1, newlen * 2);

		newlen = EntityReplacer((WCHAR *)(chat + CHAT_DATA_INDEX), L"&lt;", '<', newlen);
		newlen = EntityReplacer((WCHAR *)(chat + CHAT_DATA_INDEX), L"&gt;", '>', newlen);

		memcpy(chat + CHAT_DATA_INDEX + newlen * 2, ptr, *packet - CHAT_DATA_INDEX - origlen * 2);
		*chat = *packet - origlen * 2 + newlen * 2;

		memcpy(packet, chat, *chat);
	}
	// die : character 29 
	else if (wcsstr((WCHAR *)(packet + CHAT_DATA_INDEX), L"overflow_test")) {
		WCHAR *ptrb = wcschr((WCHAR *)(packet + CHAT_DATA_INDEX), ' ');
		SIZE_T origlen = wcslen((WCHAR *)(packet + CHAT_DATA_INDEX));
		SIZE_T newlen = _wtoi(ptrb + 1);

		char chat[8192] = { 0, };
		memcpy(chat, packet, CHAT_DATA_INDEX);

		for (int i = 0; i < newlen; ++i) {
			((WCHAR *)(chat + CHAT_DATA_INDEX))[i] = 'a';
		}

		*ptr = '<';
		memcpy(chat + CHAT_DATA_INDEX + newlen * 2, ptr, *packet - CHAT_DATA_INDEX - origlen * 2);
		*((PWORD)&chat) = *packet - origlen * 2 + newlen * 2;

		memcpy(packet, chat, *chat);
	}
	else {
		*ptr = '<';
	}

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
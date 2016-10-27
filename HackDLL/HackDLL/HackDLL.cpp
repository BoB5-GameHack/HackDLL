// HackDLL.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"

int CreateConsoleIO() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	return 0;
}

#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))
DWORD GetMainThreadId() {
	THREADENTRY32 th32;
	memset(&th32, 0, sizeof(THREADENTRY32));
	th32.dwSize = sizeof(THREADENTRY32);

	DWORD pid = GetCurrentProcessId();
	DWORD dwMainThreadID = -1;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	if (Thread32First(hSnapshot, &th32)) {
		DWORD64 ullMinCreateTime = 0xFFFFFFFFFFFFFFFF;

		do {
			if (th32.th32OwnerProcessID == pid) {
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID);

				if (hThread) {
					FILETIME afTimes[4] = { 0 };
					if (GetThreadTimes(hThread, &afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) {
						ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime,
							afTimes[0].dwHighDateTime);
						if (ullTest && ullTest < ullMinCreateTime) {
							ullMinCreateTime = ullTest;
							dwMainThreadID = th32.th32ThreadID;
						}
					}
					CloseHandle(hThread);
				}
			}
		} while (Thread32Next(hSnapshot, &th32));
	}

	CloseHandle(hSnapshot);
	return dwMainThreadID;
}

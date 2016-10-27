#pragma once

#include "stdafx.h"
#include "MemoryScanner.h"

int MemoryScan(BYTE *pattern, SIZE_T length, std::vector<LPVOID>& list) {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	LPVOID lpStartAddress = (LPVOID)sysinfo.lpMinimumApplicationAddress;
	LPVOID lpEndAddress = (LPVOID)sysinfo.lpMaximumApplicationAddress;

	std::string strPattern(pattern, pattern + length);

	while (lpStartAddress < lpEndAddress) {
		MEMORY_BASIC_INFORMATION mbi = { 0, };
		if (!VirtualQuery(lpStartAddress, &mbi, sizeof(mbi))) {
			return false;
		}

		if (mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) && mbi.Protect != PAGE_NOACCESS) {
			if ((mbi.Protect & PAGE_EXECUTE_READ) || (mbi.Protect & PAGE_EXECUTE_READWRITE)) {
				BYTE *dump = new BYTE[mbi.RegionSize];
				memcpy(dump, lpStartAddress, mbi.RegionSize);
				std::string mem(dump, dump + mbi.RegionSize);

				size_t n = -1;
				while (true) {
					n = mem.find(strPattern, n + 1);
					if (n == std::string::npos) {
						break;
					}

					list.push_back((LPVOID)((SIZE_T)lpStartAddress + n));
				}

				delete[] dump;
			}
		}

		lpStartAddress = (LPVOID)((SIZE_T)lpStartAddress + mbi.RegionSize);
	}

	return true;
}
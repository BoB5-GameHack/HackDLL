// HackDLL.cpp : DLL ���� ���α׷��� ���� ������ �Լ��� �����մϴ�.
//

#include "stdafx.h"

int CreateConsoleIO() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	return 0;
}
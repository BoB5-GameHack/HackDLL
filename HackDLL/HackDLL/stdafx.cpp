// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// HackDLL.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"

// TODO: �ʿ��� �߰� �����
// �� ������ �ƴ� STDAFX.H���� �����մϴ�.

int WriteMemory(LPVOID lpAddr, LPVOID data, SIZE_T len) {
	DWORD old;

	VirtualProtect(lpAddr, len, PAGE_EXECUTE_READWRITE, &old);
	memcpy(lpAddr, data, len);
	VirtualProtect(lpAddr, len, old, NULL);

	return 0;
}
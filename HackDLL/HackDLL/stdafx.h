// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <stdio.h>
#include <io.h>

#include <windows.h>
#include <WinSock2.h>
#include <TlHelp32.h>

#include <vector>

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
int WriteMemory(LPVOID lpAddr, LPVOID data, SIZE_T len);
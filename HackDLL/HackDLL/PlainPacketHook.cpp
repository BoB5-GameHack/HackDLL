#include "stdafx.h"

#include "HackDLL.h"
#include "MemoryScanner.h"

#include "SendHook.h"
#include "PlainPacketHook.h"

int PacketDumper(LPVOID packetAddr, SIZE_T size);
int PacketReplacer(unsigned char *data, int *len);
long WINAPI HookingHandler(PEXCEPTION_POINTERS ExceptionInfo);

LPVOID lpEncryptFunctionAddr = NULL; 
LPVOID lpMakePacketFunctionAddr = NULL;

int WINAPI HookPlainPacket() {
	std::vector<LPVOID> list;
	BYTE pattern[] = { 0x40, 0x53, 0x41, 0x54, 0x48, 0x83, 0xEC, 0x48, 0x4C, 0x8D, 0x49, 0x04 };
	MemoryScan(pattern, sizeof(pattern), list);

	lpEncryptFunctionAddr = (LPVOID)((SIZE_T)list.front() + 4);

	list.clear();
	BYTE pattern2[] = { 0x48, 0x8B, 0x74, 0x24, 0x50, 0x48, 0x8B, 0xC5, 0x48, 0x8B, 0x6C, 0x24, 0x48, 0x48, 0x83, 0xC4, 0x30, 0x5F, 0xC3, 0xCC };
	MemoryScan(pattern2, sizeof(pattern2), list);

	lpMakePacketFunctionAddr = (LPVOID)((SIZE_T)list.at(4) + 5);

	if (AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)HookingHandler) == NULL) {
		printf("[*] HookPlainText : Couldn't add vectored exception handler..\n");
		return 1;
	}

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetMainThreadId());

	ctx.Dr0 = (DWORD64)lpEncryptFunctionAddr;
	ctx.Dr1 = (DWORD64)lpMakePacketFunctionAddr;
	ctx.Dr7 |= 0x5;

	SetThreadContext(hThread, &ctx);
	CloseHandle(hThread);

	return 0;
}

long WINAPI HookingHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	PEXCEPTION_RECORD pExceptionRecord = ExceptionInfo->ExceptionRecord;
	PCONTEXT pContext = ExceptionInfo->ContextRecord;

	if (pExceptionRecord->ExceptionAddress == lpEncryptFunctionAddr) {
		isPlainPacket = false;
		
		pContext->Rip = (DWORD64)lpEncryptFunctionAddr + 4;
		pContext->Rsp -= 0x48;

		PacketDumper((LPVOID)pContext->Rdx, pContext->R8); //Rdx = PacketAddr, R8 = PacketLength
		
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExceptionRecord->ExceptionAddress == lpMakePacketFunctionAddr) {
		pContext->Rip = (DWORD64)lpMakePacketFunctionAddr + 3;
		pContext->Rax = pContext->Rbp;

		printf("test");
		if (!access("D:\\packets.dat", 0)) {
			PacketReplacer(*(unsigned char **)pContext->Rbp, (int *)&pContext->R8); //[Rbp] = PacketAddr, R8 = PacketLength
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

int PacketDumper(LPVOID packetAddr, SIZE_T size) {
	unsigned char *data = (unsigned char *)packetAddr;

	printf("[*] plain : ");
	for (int i = 0; i < size; ++i) {
		printf("%02X ", data[i]);
	}
	printf("\n[*] dump : ");
	for (int i = 0; i < size; ++i) {
		printf("%c", data[i]);
	}
	printf("\n");

	return 0;
}

int PacketReplacer(unsigned char *data, int *length) {
	printf("[*] replacing ..\n");

	FILE *fp = fopen("D:\\packets.dat", "r");

	BYTE newSize = 0;
	fscanf(fp, "%02X", &newSize);

	BYTE *newPacket = new BYTE[newSize];
	newPacket[0] = newSize;

	for (int i = 1; i < newSize; ++i) {
		fscanf(fp, "%02X", &newPacket[i]);
	}

	*length = newSize;
	memcpy(data, newPacket, newSize);
	
	delete[] newPacket;
	fclose(fp);

	return 0;
}
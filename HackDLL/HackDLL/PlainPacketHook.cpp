#include "stdafx.h"
#include "HackDLL.h"
#include "MemoryScanner.h"
#include "PlainPacketHook.h"

int PacketDumper();
long HookingHandler(PEXCEPTION_POINTERS ExceptionInfo);

LPVOID lpEncryptFunctionAddr = NULL; 

int HookPlainPacket() {
	std::vector<LPVOID> list;
	BYTE pattern[] = { 0x40, 0x53, 0x41, 0x54, 0x48, 0x83, 0xEC, 0x48, 0x4C, 0x8D, 0x49, 0x04 };
	MemoryScan(pattern, sizeof(pattern), list);

	lpEncryptFunctionAddr = list.front();

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetMainThreadId());
	printf("[*] open thread..\n");

	if (AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)HookingHandler) == NULL) {
		printf("[*] HookPlainText : Couldn't add vectored exception handler..\n");
		return 1;
	}

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_CONTROL;

	ctx.Dr0 = (DWORD64)lpEncryptFunctionAddr;
	ctx.Dr7 |= 0x1;

	SetThreadContext(hThread, &ctx);

	CloseHandle(hThread);
	return 0;
}

long HookingHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionAddress == lpEncryptFunctionAddr) {
		PacketDumper();

		ExceptionInfo->ContextRecord->Rip = (DWORD64)lpEncryptFunctionAddr + 4;
		ExceptionInfo->ContextRecord->Rsp -= 0x48;

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

int PacketDumper() {
	LPVOID packetAddr = (LPVOID)0x7FF6285E0860;
	unsigned char length = *((unsigned char *)packetAddr);

	BYTE *dump = new BYTE[length];
	memcpy(dump, packetAddr, length);

	printf("[*] plain : ");
	for (int i = 0; i < length; ++i) {
		printf("%02X ", dump[i]);
	}
	printf("\n");

	return 0;
}
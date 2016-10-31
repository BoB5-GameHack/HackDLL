#include "stdafx.h"

#include "HackDLL.h"
#include "MemoryScanner.h"

#include "SendHook.h"
#include "PlainPacketHook.h"

int PacketDumper(LPVOID packetAddr, SIZE_T size);
long WINAPI HookingHandler(PEXCEPTION_POINTERS ExceptionInfo);

LPVOID lpEncryptFunctionAddr = NULL; 

int WINAPI HookPlainPacket() {
	std::vector<LPVOID> list;
	BYTE pattern[] = { 0x40, 0x53, 0x41, 0x54, 0x48, 0x83, 0xEC, 0x48, 0x4C, 0x8D, 0x49, 0x04 };
	MemoryScan(pattern, sizeof(pattern), list);

	lpEncryptFunctionAddr = (LPVOID)((SIZE_T)list.front() + 4);

	if (AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)HookingHandler) == NULL) {
		printf("[*] HookPlainText : Couldn't add vectored exception handler..\n");
		return 1;
	}

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetMainThreadId());

	ctx.Dr0 = (DWORD64)lpEncryptFunctionAddr;
	ctx.Dr7 |= 0x1;

	SetThreadContext(hThread, &ctx);
	CloseHandle(hThread);

	return 0;
}

long WINAPI HookingHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionAddress == lpEncryptFunctionAddr) {
		PCONTEXT pContext = ExceptionInfo->ContextRecord;

		PacketDumper((LPVOID)pContext->Rdx, pContext->R8);
		isPlainPacket = false;

		pContext->Rip = (DWORD64)lpEncryptFunctionAddr + 4;
		pContext->Rsp -= 0x48;

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
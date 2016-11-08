#include "stdafx.h"

#include "HackDLL.h"
#include "MemoryScanner.h"

#include "SendHook.h"
#include "ChatHook.h"
#include "PlainPacketHook.h"

int PacketDumper(unsigned char *packetAddr, SIZE_T size);
long WINAPI HookingHandler(PEXCEPTION_POINTERS ExceptionInfo);

LPVOID lpEncryptFunctionAddr = NULL;
LPVOID lpMakePacketFunctionAddr = NULL;
LPVOID lpMakeChatFunctionAddr = NULL;

DWORD64 PacketNumber = 0;

int WINAPI HookPlainPacket() {
	std::vector<LPVOID> list;
	BYTE pattern[] = { 0x40, 0x53, 0x41, 0x54, 0x48, 0x83, 0xEC, 0x48, 0x4C, 0x8D, 0x49, 0x04 };
	MemoryScan(pattern, sizeof(pattern), list);

	lpEncryptFunctionAddr = (LPVOID)((SIZE_T)list.front() + 4);
	printf("[*] Encryption : %p\n", lpEncryptFunctionAddr);

	list.clear();
	BYTE pattern2[] = { 0x48, 0x8B, 0x74, 0x24, 0x50, 0x48, 0x8B, 0xC5, 0x48, 0x8B, 0x6C, 0x24, 0x48, 0x48, 0x83, 0xC4, 0x30, 0x5F, 0xC3, 0xCC };
	MemoryScan(pattern2, sizeof(pattern2), list);

	lpMakePacketFunctionAddr = (LPVOID)((SIZE_T)list.at(3) + 5);
	printf("[*] Packet : %p\n", lpMakePacketFunctionAddr);

	list.clear();
	BYTE pattern3[] = { 0xe8, 0x78, 0x7d, 0xd7, 0xff, 0x48, 0x8b, 0x5c, 0x24, 0x30, 0x48, 0x8b, 0x6c, 0x24, 0x38, 0x48, 0x8b, 0xc6, 0x48, 0x8b, 0x74, 0x24, 0x40, 0x48, 0x83, 0xc4, 0x20, 0x5f, 0xc3 };
	MemoryScan(pattern3, sizeof(pattern3), list);
	
	lpMakeChatFunctionAddr = (LPVOID)((SIZE_T)list.front() + 23);
	printf("[*] Chat : %p\n", lpMakeChatFunctionAddr);

	if (AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)HookingHandler) == NULL) {
		printf("[*] HookPlainText : Couldn't add vectored exception handler..\n");
		return 1;
	}

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	ctx.Dr0 = (DWORD64)lpEncryptFunctionAddr;
	ctx.Dr1 = (DWORD64)lpMakePacketFunctionAddr;
	ctx.Dr2 = (DWORD64)lpMakeChatFunctionAddr;
	ctx.Dr7 |= (0x1 | 0x4 | 0x10);

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetMainThreadId());

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

		PacketDumper((unsigned char *)pContext->Rdx, pContext->R8); //Rdx = PacketAddr, R8 = PacketLength

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExceptionRecord->ExceptionAddress == lpMakePacketFunctionAddr) {
		pContext->Rip = (DWORD64)lpMakePacketFunctionAddr + 3;
		pContext->Rax = pContext->Rbp;

		if (isHooked) {
			PacketReplacer(*(unsigned char **)pContext->Rbp, (int *)&pContext->R8);
		}
		
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExceptionRecord->ExceptionAddress == lpMakeChatFunctionAddr) {
		pContext->Rip = (DWORD64)lpMakeChatFunctionAddr + 4;
		pContext->Rsp += 0x20;
	
		ChatHook((unsigned char *)pContext->Rdx);
	
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

int PacketDumper(unsigned char *packetAddr, SIZE_T size) {
	printf("[%d] plain : ", ++PacketNumber);
	for (int i = 0; i < size; ++i) {
		printf("%02X ", packetAddr[i]);
	}
	printf("\n[*] dump : ");
	for (int i = 0; i < size; ++i) {
		printf("%c", packetAddr[i]);
	}
	printf("\n");

	return 0;
}

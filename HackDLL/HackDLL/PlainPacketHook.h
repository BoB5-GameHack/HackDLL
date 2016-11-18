#pragma once

int WINAPI HookPlainPacket();

extern LPVOID lpEncryptFunctionAddr;
extern LPVOID lpDecryptFunctionAddr;
extern LPVOID lpMakePacketFunctionAddr;
extern LPVOID lpMakeChatFunctionAddr;
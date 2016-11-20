#pragma once

extern bool isPlainRecvPacket;
extern bool isOmitPacket;

extern LPVOID addrRecv;

int PatchRecv();
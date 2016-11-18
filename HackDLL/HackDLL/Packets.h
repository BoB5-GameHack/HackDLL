#pragma once

extern bool isHooked;
extern bool isPlaying;

int PacketDumper(unsigned char *packetAddr, SIZE_T size, bool flag);
int PacketReplacer(unsigned char *data, int *length);
int EntityReplacer(WCHAR *str, WCHAR *target, WCHAR chr, int len);
int PlayMacro();
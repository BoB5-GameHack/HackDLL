#pragma once

extern bool isHooked;

int ChatHook(unsigned char *packet);
int PacketReplacer(unsigned char *packet, int *length);
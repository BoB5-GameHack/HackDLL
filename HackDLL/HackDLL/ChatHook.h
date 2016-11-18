#pragma once

extern char filename[1024];

int ChatHook(unsigned char *packet);
int PacketReplacer(unsigned char *packet, int *length);
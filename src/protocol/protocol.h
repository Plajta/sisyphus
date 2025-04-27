#pragma once

#include "lfs.h"

#define CMD_BUF_SIZE 128
#define MAX_BUF_SIZE 1024
#define CRC_BUF_SIZE 1024
#define TIMEOUT_MS 1000

#define EOT_CHAR 0x04  // EOT (End Of Transmission) character

void print_newline(char* c);
void handle_command(lfs_t *lfs, char *cmd);
void protocol_loop(lfs_t *lfs);

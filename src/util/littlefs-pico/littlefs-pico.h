#ifndef LITTLEFS_PICO_H
#define LITTLEFS_PICO_H

#include "hardware/flash.h"

#include <stdint.h>
#include <lfs.h>

// Define the flash sizes
// This is setup to read a block of the flash from the end
// ADDR_LITTLEFS is defined by the linker script
// LITTLEFS_SIZE_BYTES is defined in CMakeLists.txt
extern uint32_t ADDR_LITTLEFS[];
#define LITTLEFS_BASE_ADDR (uint32_t)(ADDR_LITTLEFS)
#define LITTLEFS_FLASH_OFFSET_ADDR (LITTLEFS_BASE_ADDR-XIP_BASE)

int pico_lfs_init(lfs_t *lfs);

#endif

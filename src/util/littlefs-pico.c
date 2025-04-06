// Based on https://dominicmaas.co.nz/pages/pico-filesystem-littlefs

#include "littlefs-pico.h"
#include <hardware/regs/addressmap.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"

#include <lfs.h>

static int pico_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr = LITTLEFS_BASE_ADDR + (block * c->block_size) + off;

    #ifdef SISYPHUS_DEBUG
    printf("[FS] READ - Addr: %p, Block: %d, Offset: %d, Size: %d\n", addr - XIP_BASE, block, off, size);
    #endif

    memcpy(buffer, (unsigned char *)addr, size);
    return LFS_ERR_OK;
}

static int pico_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr = LITTLEFS_FLASH_OFFSET_ADDR + (block * c->block_size) + off;

    #ifdef SISYPHUS_DEBUG
    printf("[FS] WRITE - Addr: %p, Block: %d, Offset: %d, Size: %d\n", addr, block, off, size);
    #endif

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr, (const uint8_t *)buffer, size);
    restore_interrupts(ints);

    return LFS_ERR_OK;
}

static int pico_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t offset = LITTLEFS_FLASH_OFFSET_ADDR + (block * c->block_size);

    #ifdef SISYPHUS_DEBUG
    printf("[FS] ERASE: Addr: %p, Block: %d\n", offset, block);
    #endif

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(offset, c->block_size);
    restore_interrupts(ints);

    return LFS_ERR_OK;
}

static int pico_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

const struct lfs_config pico_lfs_cfg = {
    // block device operations
    .read  = &pico_read,
    .prog  = &pico_prog,
    .erase = &pico_erase,
    .sync  = &pico_sync,

    // block device configuration
    .read_size = FLASH_PAGE_SIZE, // 256
    .prog_size = FLASH_PAGE_SIZE, // 256

    .block_size = FLASH_SECTOR_SIZE, // 4096
    .block_count = LITTLEFS_SIZE_BYTES / FLASH_SECTOR_SIZE,
    .block_cycles = 128,

    .cache_size = FLASH_PAGE_SIZE, // 256
    .lookahead_size = FLASH_PAGE_SIZE,   // 256
};

int pico_lfs_init(lfs_t *lfs)
{
    int err = lfs_mount(lfs, &pico_lfs_cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(lfs, &pico_lfs_cfg);
        err = lfs_mount(lfs, &pico_lfs_cfg);
    }
    return err;
}

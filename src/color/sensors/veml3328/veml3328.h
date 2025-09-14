#pragma once
#include <stdint.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "../../color.h"

#define VEML3328_I2C_ADDRESS 0x10

#define VEML3328_CMD_CODE_CFG 0x00

#define VEML3328_CMD_CODE_CLEAR 0x04
#define VEML3328_CMD_CODE_RED 0x05
#define VEML3328_CMD_CODE_GREEN 0x06
#define VEML3328_CMD_CODE_BLUE 0x07
#define VEML3328_CMD_CODE_IR 0x08
#define VEML3328_CMD_CODE_ID 0x0C

typedef union {
    uint16_t raw;
    struct {
        uint16_t SD0     : 1;  // Bit 0: Shutdown setting (default = 1)
        uint16_t RESERVED1 : 1;  // Bit 1: Reserved (Set 0)
        uint16_t TRIG    : 1;  // Bit 2: Trigger one measurement
        uint16_t AF      : 1;  // Bit 3: Auto/active force mode
        uint16_t IT      : 2;  // Bits 5:4: Integration time
        uint16_t SENS    : 1;  // Bit 6: Sensitivity
        uint16_t RESERVED2 : 1;  // Bit 7: Reserved (Set 0)
        uint16_t RESERVED3 : 2;  // Bits 9:8: Reserved (Set 0)
        uint16_t GAIN    : 2;  // Bits 11:10: Gain setting
        uint16_t DG      : 2;  // Bits 13:12: Digital gain
        uint16_t SD_ALS  : 1;  // Bit 14: ALS shutdown per channel
        uint16_t SD1     : 1;  // Bit 15: Shutdown setting
    };
} veml3328_cfg_reg;

int veml3328_setup(i2c_inst_t *i2c);
int veml3328_trigger(i2c_inst_t *i2c);
int veml3328_shutdown(i2c_inst_t *i2c, bool shutdown);
int veml3328_read_color(i2c_inst_t *i2c, color_measurement *color);

// Debug code
#ifdef SISYPHUS_DEBUG
int veml3328_read_raw_color(i2c_inst_t *i2c, raw_color_measurement *color);
#endif

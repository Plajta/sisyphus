#include "veml3328.h"
#include <stdbool.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/error.h>

int veml3328_read_registers(i2c_inst_t *i2c, uint8_t cmd, uint16_t *value) {
    int ret;

    ret = i2c_write_blocking(i2c, VEML3328_I2C_ADDRESS, &cmd, 1, true);
    if (ret < PICO_OK) {
        return ret;
    }

    uint8_t buffer[2];
    ret = i2c_read_blocking(i2c, VEML3328_I2C_ADDRESS, buffer, 2, false);
    if (ret < PICO_OK) {
        return ret;
    }
    *value = buffer[0] | (buffer[1] << 8);

    return PICO_OK;
}

int veml3328_shutdown(i2c_inst_t *i2c, bool shutdown){
    int ret;

    veml3328_cfg_reg config;
    ret = veml3328_read_registers(i2c, VEML3328_CMD_CODE_CFG, &config.raw);
    if (ret < PICO_OK) {
        return ret;
    }
    // Both need to be set for the setting to apply
    config.SD0 = shutdown;
    config.SD1 = shutdown;

    uint8_t buffer[3] = {VEML3328_CMD_CODE_CFG, config.raw&0xFF, config.raw>>8};
    ret = i2c_write_blocking(i2c, VEML3328_I2C_ADDRESS, buffer, sizeof(buffer), false);

    return ret;
}

int veml3328_read_color(i2c_inst_t *i2c, color_measurement *color) {
    uint16_t buffer;

    // Helper macro to reduce repetition
    #define READ_COLOR_COMPONENT(cmd, dest)               \
        do {                                              \
            int ret = veml3328_read_registers(i2c, cmd, &buffer); \
            if (ret < PICO_OK) return ret;                \
            dest = buffer >> 8;                           \
        } while (0)

    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_RED,   color->red);
    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_GREEN, color->green);
    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_BLUE,  color->blue);
    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_CLEAR, color->clear);

    #undef READ_COLOR_COMPONENT

    return PICO_OK;
}

#include "veml3328.h"
#include <stdbool.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/error.h>
#include "error.h"

int veml3328_read_registers(i2c_inst_t *i2c, uint8_t cmd, uint16_t *value) {
    RETURN_IF_ERROR(i2c_write_blocking(i2c, VEML3328_I2C_ADDRESS, &cmd, 1, true));

    uint8_t buffer[2];
    RETURN_IF_ERROR(i2c_read_blocking(i2c, VEML3328_I2C_ADDRESS, buffer, 2, false));
    *value = buffer[0] | (buffer[1] << 8);

    return PICO_OK;
}

/*
 * Sets up the configuration register.
 *
 * Right now setup for 4x gain, 100ms integration time and manual measuring.
 *
 * For triggering a manual measurement cycle look at `veml3328_trigger()`.
 */
int veml3328_setup(i2c_inst_t *i2c){
    veml3328_cfg_reg config;
    RETURN_IF_ERROR(veml3328_read_registers(i2c, VEML3328_CMD_CODE_CFG, &config.raw));
    // Settings good for our current application, edit settings here if you need to
    // TODO: Make it be DEFINE based, react to selected board or that kinda thing
    config.GAIN = 0b10; // 4x GAIN
    config.IT = 0b01; // 100ms INTEGRATION TIME
    config.AF = 0b1;

    uint8_t buffer[3] = {VEML3328_CMD_CODE_CFG, config.raw&0xFF, config.raw>>8};
    RETURN_IF_ERROR(i2c_write_blocking(i2c, VEML3328_I2C_ADDRESS, buffer, sizeof(buffer), false));

    return 0;
}

/*
 * Triggers one measurement cycle, that takes one integration time.
 */
int veml3328_trigger(i2c_inst_t *i2c){
    veml3328_cfg_reg config;
    RETURN_IF_ERROR(veml3328_read_registers(i2c, VEML3328_CMD_CODE_CFG, &config.raw));

    config.TRIG = 0b1;

    uint8_t buffer[3] = {VEML3328_CMD_CODE_CFG, config.raw&0xFF, config.raw>>8};
    RETURN_IF_ERROR(i2c_write_blocking(i2c, VEML3328_I2C_ADDRESS, buffer, sizeof(buffer), false));
    // TODO: Make it wait for the bit being reset, in the meantime just have a manual sleep

    return 0;
}

int veml3328_shutdown(i2c_inst_t *i2c, bool shutdown){
    veml3328_cfg_reg config;
    RETURN_IF_ERROR(veml3328_read_registers(i2c, VEML3328_CMD_CODE_CFG, &config.raw));
    // Both need to be set for the setting to apply
    config.SD0 = shutdown;
    config.SD1 = shutdown;

    uint8_t buffer[3] = {VEML3328_CMD_CODE_CFG, config.raw&0xFF, config.raw>>8};
    RETURN_IF_ERROR(i2c_write_blocking(i2c, VEML3328_I2C_ADDRESS, buffer, sizeof(buffer), false));

    return 0;
}

int veml3328_read_raw_color(i2c_inst_t *i2c, raw_color_measurement *color) {
    uint16_t buffer;

    // Helper macro to reduce repetition
    #define READ_COLOR_COMPONENT(cmd, dest)               \
        do {                                              \
            RETURN_IF_ERROR(veml3328_read_registers(i2c, cmd, &buffer));  \
            dest = buffer;                                \
        } while (0)

    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_RED,   color->red);
    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_GREEN, color->green);
    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_BLUE,  color->blue);
    READ_COLOR_COMPONENT(VEML3328_CMD_CODE_CLEAR, color->clear);

    #undef READ_COLOR_COMPONENT

    return PICO_OK;
}

int veml3328_read_color(i2c_inst_t *i2c, color_measurement *color){
    raw_color_measurement raw_color;

    RETURN_IF_ERROR(veml3328_read_raw_color(i2c, &raw_color));

    // TODO: Add a color correction matrix
    // For now it's just a bunch of coefficients
    // MAGIC NUMBERS AHEAD
    float red = (raw_color.red * 2.25) / 65536.0f;
    float green = (raw_color.green * 1.25) / 65536.0f;
    float blue = (raw_color.blue * 2.5) / 65536.0f;

    color_rgb_to_hsv(red, green, blue, color);
    color->clear = (raw_color.clear >> 8); // Make it 8 bit

    return 0;
}

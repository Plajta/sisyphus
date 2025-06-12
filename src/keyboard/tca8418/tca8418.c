#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/error.h>
#include <pico/time.h>
#include "tca8418.h"

int tca8418_init(i2c_inst_t *i2c) {
    #ifdef SISYFOSS_KEYBOARD_RESET
    gpio_init(SISYFOSS_KEYBOARD_RESET);
    gpio_set_dir(SISYFOSS_KEYBOARD_RESET, GPIO_OUT);
    gpio_put(SISYFOSS_KEYBOARD_RESET, 0); // Reset
    sleep_ms(1); // Wait a bit for reset
    gpio_put(SISYFOSS_KEYBOARD_RESET, 1); // Not reset
    sleep_ms(1); // Wait a bit for it finish
    #endif

    int ret;

    uint8_t config[2] = {TCA8418_REG_CFG, 0b10010001}; // Set up autoincrement
    ret = i2c_write_blocking(i2c, TCA8418_I2C_ADDRESS, config, sizeof(config), false);
    if (ret < PICO_OK) {
        return ret;
    }

    uint8_t event_mode[4] = {TCA8418_REG_GPI_EM1, 0xFF, 0xFF, 0xFF}; // You probably want it like this
    ret = i2c_write_blocking(i2c, TCA8418_I2C_ADDRESS, event_mode, sizeof(event_mode), false);
    if (ret < PICO_OK) {
        return ret;
    }

    return 0;
}

int tca8418_setup_keyboard(i2c_inst_t *i2c, uint8_t rows, uint16_t cols) {
    int ret;

    uint8_t data_rows[2] = {TCA8418_REG_KP_GPIO1, rows};
    ret = i2c_write_blocking(i2c, TCA8418_I2C_ADDRESS, data_rows, sizeof(data_rows), false);
    if (ret < PICO_OK) {
        return ret;
    }

    uint8_t data_cols[3] = {TCA8418_REG_KP_GPIO2, cols&0xFF, cols>>8};
    ret = i2c_write_blocking(i2c, TCA8418_I2C_ADDRESS, data_cols, sizeof(data_cols), false);
    if (ret < PICO_OK) {
        return ret;
    }

    return 0;
}

void tca8418_setup_interrupt(gpio_irq_callback_t callback) {
    gpio_init(SISYFOSS_KEYBOARD_INTERRUPT);
    gpio_set_dir(SISYFOSS_KEYBOARD_INTERRUPT, GPIO_IN);
    gpio_pull_up(SISYFOSS_KEYBOARD_INTERRUPT);
    gpio_set_irq_enabled_with_callback(SISYFOSS_KEYBOARD_INTERRUPT, GPIO_IRQ_EDGE_FALL, true, callback);
}

bool tca8418_k_int_available(i2c_inst_t *i2c) {
    uint8_t value;
    int ret;

    ret = tca8418_read_register(i2c, TCA8418_REG_INT_STAT, &value);
    if (ret < PICO_OK) {
        return false;
    }

    return (value & 1); // Check if first bit (K_INT) is 1
}

int tca8418_k_int_reset(i2c_inst_t *i2c) {
    int ret;

    uint8_t int_stat[2] = {TCA8418_REG_INT_STAT, 1}; // Writing bit 1 resets the interrupt
    ret = i2c_write_blocking(i2c, TCA8418_I2C_ADDRESS, int_stat, sizeof(int_stat), false);
    if (ret < PICO_OK) {
        return ret;
    }

    return 0;
}

uint8_t tca8418_num_events(i2c_inst_t *i2c) {
    uint8_t value;
    int ret;

    ret = tca8418_read_register(i2c, TCA8418_REG_KEY_LCK_EC, &value);
    if (ret < PICO_OK) {
        return 0;
    }

    return (value & 0b1111); // Only bottom 4 bits are relevant
}

int tca8418_get_key_from_fifo(i2c_inst_t *i2c, uint8_t *key, bool *pressed) {
    uint8_t value;
    int ret;

    ret = tca8418_read_register(i2c, TCA8418_REG_KEY_EVENT_A, &value);
    if (ret < PICO_OK) {
        return ret;
    }

    *key = value & 0b1111111; // 7 bits
    *pressed = value >> 7; // 8th bit; 1 is pressed; 0 is released

    return 0; // Only bottom 4 bits are relevant
}

int tca8418_read_register(i2c_inst_t *i2c, uint8_t reg, uint8_t *value) {
    int ret;

    ret = i2c_write_blocking(i2c, TCA8418_I2C_ADDRESS, &reg, 1, true);
    if (ret < PICO_OK) {
        return ret;
    }

    ret = i2c_read_blocking(i2c, TCA8418_I2C_ADDRESS, value, 1, false);
    if (ret < PICO_OK) {
        return ret;
    }

    return 0;
}

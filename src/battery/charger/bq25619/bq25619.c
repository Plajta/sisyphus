
#include "bq25619.h"
#include "bq25619_specific.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico.h>
#include <pico/error.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdint.h>
#include "error.h"


int bq25619_read_register(i2c_inst_t *i2c, uint8_t reg, uint8_t *value){
    int ret;
    uint8_t status[1];
    RETURN_IF_ERROR(i2c_write_blocking(i2c, BQ25619_I2C_ADDRESS, &reg, 1, false));
    RETURN_IF_ERROR(i2c_read_blocking(i2c, BQ25619_I2C_ADDRESS, status, 1, false));
    *value = status[0];
    return PICO_OK;
}

int bq25619_write_register(i2c_inst_t *i2c, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    int ret = i2c_write_blocking(i2c, BQ25619_I2C_ADDRESS, buf, 2, false);
    return (ret == 2) ? 0 : ret;
}

int bq25619_read_status(i2c_inst_t *i2c, bq25619_status *status){
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_STATUS_0, &status->raw_status0));
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_STATUS_2, &status->raw_status2));
    return PICO_OK;
}

int bq25619_set_input_current_limit(i2c_inst_t *i2c, int max_current){
    bq25619_input_settings s;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_INPUT_CURRENT_LIMIT, &s.raw_input_settings));
    s.input_current_limit = max_current / BQ25619_INPUT_CURRENT_STEP;
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_INPUT_CURRENT_LIMIT, s.raw_input_settings));
    return PICO_OK;
}

int bq25619_set_TS_ignore(i2c_inst_t *i2c, bool enable){
    bq25619_input_settings s;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_INPUT_CURRENT_LIMIT, &s.raw_input_settings));
    s.ignore_ts = enable;
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_INPUT_CURRENT_LIMIT, s.raw_input_settings));
    return PICO_OK;
}

int bq25619_set_charge_current_limit(i2c_inst_t *i2c, int max_current){
    bq25619_charge_settings s;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_CHARGE_CURRENT_LIMIT, &s.raw_charge_settings));
    s.charge_current_limit = BQ25619_ENCODE_CHARGE_CURRENT(max_current);
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_CHARGE_CURRENT_LIMIT, s.raw_charge_settings));
    return PICO_OK;
}

int bq25619_set_vbat_limit(i2c_inst_t *i2c, int max_voltage){
    bq25619_battery_settings s;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_BATTERY_SETTINGS, &s.raw_battery_settings));
    s.max_battery_voltage = BQ25619_ENCODE_CHARGE_VOLTAGE(max_voltage);
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_BATTERY_SETTINGS, s.raw_battery_settings));
    return PICO_OK;
}

int bq25619_set_precharge_current_limit(i2c_inst_t *i2c, int max_current){
    bq25619_precharge_termination_settings s;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_PRECHARGE_TERM_CURRENT_LIMIT, &s.raw_precharge_termination_settings));
    s.precharge_current_limit = max_current / BQ25619_PRECHARGE_CURRENT_STEP;
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_PRECHARGE_TERM_CURRENT_LIMIT, s.raw_precharge_termination_settings));
    return PICO_OK;
}

int bq25619_set_termination_current_limit(i2c_inst_t *i2c, int max_current){
    bq25619_precharge_termination_settings s;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_PRECHARGE_TERM_CURRENT_LIMIT, &s.raw_precharge_termination_settings));
    s.termination_current_limit = max_current / BQ25619_TERMINATION_CURRENT_STEP;
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_PRECHARGE_TERM_CURRENT_LIMIT, s.raw_precharge_termination_settings));
    return PICO_OK;
}

int bq25619_set_defaults(i2c_inst_t *i2c){
    RETURN_IF_ERROR(bq25619_set_charge_current_limit(i2c, BQ25619_DEFAULT_CHARGE_CURRENT_LIMIT));
    RETURN_IF_ERROR(bq25619_set_vbat_limit(i2c, BQ25619_DEFAULT_CHARGE_VOLTAGE_LIMIT));
    RETURN_IF_ERROR(bq25619_set_precharge_current_limit(i2c, BQ25619_DEFAULT_PRECHARGE_CURRENT_LIMIT));
    RETURN_IF_ERROR(bq25619_set_termination_current_limit(i2c, BQ25619_DEFAULT_TERMINATION_CURRENT_LIMIT));
    RETURN_IF_ERROR(bq25619_set_input_current_limit(i2c, BQ25619_DEFAULT_INPUT_CURRENT_LIMIT));
    return PICO_OK;
}

void bq25619_irq_callback(uint gpio, uint32_t event_mask){
    return;
}

int bq25619_init(i2c_inst_t *i2c){
    #ifdef SISYFOSS_PMIC_INT
    gpio_init(SISYFOSS_PMIC_INT);
    gpio_set_dir(SISYFOSS_PMIC_INT, GPIO_IN);
    gpio_set_irq_enabled_with_callback(SISYFOSS_PMIC_INT, GPIO_IRQ_EDGE_FALL, true, bq25619_irq_callback);
    #endif // SISYFOSS_PMIC_INT
    bq25619_set_defaults(i2c);
    return PICO_OK;
}

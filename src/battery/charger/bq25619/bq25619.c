
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
    uint8_t raw_status;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_STATUS_0, &raw_status));
    status->v_bus = raw_status >> 5;
    status->charge_status = (raw_status >> 3) & 0x3;
    status->power_good = raw_status & 0x4;
    status->termal_regulated = raw_status & 0x2;
    status->sys_regulation = raw_status & 0x1;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_STATUS_2, &raw_status));
    status->source_good = raw_status & 0x80;
    status->vin_dpm_mode = raw_status & 0x40;
    status->cin_dpm_mode = raw_status & 0x20;
    status->top_off_timer_active = raw_status & 0x08;
    status->adapter_overvoltage = raw_status & 0x04;
    return PICO_OK;
}

int bq25619_read_fault(i2c_inst_t *i2c, bq25619_fault_status *status){
    uint8_t raw_status;
    RETURN_IF_ERROR(bq25619_read_register(i2c, BQ25619_REG_STATUS_1, &raw_status));
    status->watchdog_expired = raw_status & 0x80;
    status->boost_fault = raw_status & 0x40;
    status->charge_fault = (raw_status >> 4) & 0x3;
    status->battery_overvoltage = raw_status & 0x08;
    status->ntc_temp = raw_status & 0x7;
    return PICO_OK;
}

int bq25619_power_from_adapter(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_TS_ignore(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}


int bq25619_set_bat_sense(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_input_cutrent_limit(i2c_inst_t *i2c, int max_current){
    return PICO_OK;
}

int bq25619_set_boost_mode(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_charging(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_sys_min(i2c_inst_t *i2c, int min_voltage){
    return PICO_OK;
}

int bq25619_set_vbat_min_in_boost(i2c_inst_t *i2c, bool mode){
    return PICO_OK;
}


int bq25619_set_q1_mode(i2c_inst_t *i2c, bool mode){
    return PICO_OK;
}

int bq25619_set_charge_cutrent_limit(i2c_inst_t *i2c, int max_current){
    return PICO_OK;
}

int bq25619_set_precharge_cutrent_limit(i2c_inst_t *i2c, int max_current){
    return PICO_OK;
}

int bq25619_set_termination_cutrent_limit(i2c_inst_t *i2c, int max_current){
    return PICO_OK;
}

int bq25619_set_vbat_limit(i2c_inst_t *i2c, int max_voltage){
    return PICO_OK;
}

int bq25619_set_topoff_timer(i2c_inst_t *i2c, int time_mode){
    return PICO_OK;
}

int bq25619_set_recharge_threshold(i2c_inst_t *i2c, bool mode){
    return PICO_OK;
}

int bq25619_set_termination_charging(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_watchdog_timer(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_charge_safety_timer(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_fastcharge_safety_timer(i2c_inst_t *i2c, bool enable){
    return PICO_OK;
}

int bq25619_set_termal_regualation_threshold(i2c_inst_t *i2c, bool threshold){
    return PICO_OK;
}

int bq25619_set_boost_voltage(i2c_inst_t *i2c, int threshold){
    return PICO_OK;
}

int bq25619_disconnect_battery(i2c_inst_t *i2c, bool immediately){
    return PICO_OK;
}


int bq25619_set_max_vin_drop(i2c_inst_t *i2c, int threshold){
    return PICO_OK;
}


int bq25619_set_defaults(i2c_inst_t *i2c){
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_INPUT_CURRENT_LIMIT, BQ25619_DEFAULT_INPUT_CURRENT_LIMIT));
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_CHARGE_CURRENT_LIMIT, BQ25619_DEFAULT_CHARGE_CURRENT_LIMIT));
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_PRECHARGE_TERM_CURRENT_LIMIT, BQ25619_DEFAULT_RECHARGE_TERM_CURRENT_LIMIT));
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_CHARGE_VOLTAGE_LIMIT, BQ25619_DEFAULT_VOLTAGE_LIMIT));
    RETURN_IF_ERROR(bq25619_write_register(i2c, BQ25619_REG_CONTROL1, BQ25619_DEFAULT_CONTROL1));
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

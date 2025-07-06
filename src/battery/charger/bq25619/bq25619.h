#pragma once
#include <hardware/i2c.h>

#define VBUS_STATUS_NONE 0
#define VBUS_STATUS_USB_HOST 1
#define VBUS_STATUS_ADAPTER 3
#define VBUS_STATUS_BOOST_MODE 7

#define CHARGING_STATUS_NOT_CHARGING 0
#define CHARGING_STATUS_PRECHARGING_TRICKLE 1
#define CHARGING_STATUS_FAST_CHARGING 2
#define CHARGING_STATUS_TERMINATED 3

#define CHARGING_FAULT_NONE 0
#define CHARGING_FAULT_INPUT 1
#define CHARGING_FAULT_TEMPERATURE 2
#define CHARGING_FAULT_TIMEOUT 3

#define NTC_TEMP_NORMAL 0
#define NTC_TEMP_WARM 2
#define NTC_TEMP_COOL 3
#define NTC_TEMP_COLD 5
#define NTC_TEMP_HOT 6

typedef struct {
    uint8_t v_bus;
    uint8_t charge_status;
    bool power_good;
    bool termal_regulated;
    bool sys_regulation;
    bool source_good;
    bool vin_dpm_mode; // Input dynamic voltage management
    bool cin_dpm_mode; // Input dynamic current management
    bool top_off_timer_active;
    bool adapter_overvoltage;
}bq25619_status;

typedef struct {
    uint8_t charge_fault;
    uint8_t ntc_temp;
    bool watchdog_expired;
    bool boost_fault;
    bool battery_overvoltage;
}bq25619_fault_status;

int bq25619_init(i2c_inst_t *i2c);
int bq25619_read_status(i2c_inst_t *i2c, bq25619_status *status);
int bq25619_read_fault_status(i2c_inst_t *i2c, bq25619_fault_status *fault_status);
uint8_t bq25619_test(i2c_inst_t *i2c);

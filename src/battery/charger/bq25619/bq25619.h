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

typedef union {
    struct {
        // STATUS_0 register
        uint8_t vbus_status       : 3; // Bits 7-5
        uint8_t charge_status     : 2; // Bits 4-3
        bool power_good           : 1; // Bit 2
        bool termal_regulated     : 1; // Bit 1
        bool sys_regulation       : 1; // Bit 0

        // STATUS_2 register
        bool vbus_source_good     : 1; // Bit 7
        bool vin_dpm_mode         : 1; // Bit 6 // Input dynamic voltage management
        bool cin_dpm_mode         : 1; // Bit 5 // Input dynamic current management
        bool bat_sense_ok         : 1; // Bit 4
        bool in_topoff            : 1; // Bit 3
        bool acov_mode            : 1; // Bit 2
        bool vin_dpm_irq          : 1; // Bit 1
        bool cin_dpm_irq          : 1; // Bit 0
    };
    struct {
        uint8_t raw_status0;
        uint8_t raw_status2;
    };
    uint16_t raw_all;
} bq25619_status;

typedef union {
    struct {
        bool watchdog_expired      : 1; // Bit 7
        bool boost_fault           : 1; // Bit 6
        uint8_t charge_fault       : 2; // Bits 4-5
        bool battery_overvoltage   : 1; // Bit 3
        uint8_t ntc_temp           : 3; // Bits 0-2
    };
    uint8_t raw_status1;
} bq25619_fault_status;

int bq25619_init(i2c_inst_t *i2c);
int bq25619_read_status(i2c_inst_t *i2c, bq25619_status *status);
int bq25619_read_fault_status(i2c_inst_t *i2c, bq25619_fault_status *fault_status);

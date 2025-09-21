#pragma once

#include <stdbool.h>
#include <stdint.h>

//  --- REGISTERS ---
#define BQ25619_I2C_ADDRESS 0x6A

#define BQ25619_REG_INPUT_CURRENT_LIMIT 0x00
#define BQ25619_REG_CHARGE_CURRENT_LIMIT 0x02
#define BQ25619_REG_PRECHARGE_TERM_CURRENT_LIMIT 0x03
#define BQ25619_REG_BATTERY_SETTINGS 0x04
#define BQ25619_REG_CONTROL1 0x05
#define BQ25619_REG_STATUS_0 0x08
#define BQ25619_REG_STATUS_1 0x09
#define BQ25619_REG_STATUS_2 0x0A

//  --- VALUES ---
#define BQ25619_INPUT_CURRENT_STEP 100
#define BQ25619_CHARGE_CURRENT_STEP 20
#define BQ25619_PRECHARGE_CURRENT_STEP 20
#define BQ25619_TERMINATION_CURRENT_STEP 20
#define BQ25619_CHARGE_VOLTAGE_STEP 10

typedef union {
    struct {
        uint8_t input_current_limit     : 5; // Bits 0-4
        bool disable_bat_sense_pin      : 1; // Bit 5
        bool ignore_ts                  : 1; // Bit 6
        bool disconnect_battery         : 1; // Bit 7
    };
    uint8_t raw_input_settings;
} bq25619_input_settings;

typedef union {
    struct {
        uint8_t charge_current_limit    : 7; // Bits 0-6
        uint8_t __reserved              : 1; // Bit 7
    };
    uint8_t raw_charge_settings;
} bq25619_charge_settings;

typedef union {
    struct {
        uint8_t termination_current_limit : 4; // Bits 0-3
        uint8_t precharge_current_limit   : 4; // Bits 4-7
    };
    uint8_t raw_precharge_termination_settings;
} bq25619_precharge_termination_settings;

typedef union {
    struct {
        bool recharge_threshold             : 1; // Bit 0
        uint8_t termination_current_limit   : 2; // Bits 1-2
        uint8_t max_battery_voltage         : 5; // Bits 3-7
    };
    uint8_t raw_battery_settings;
} bq25619_battery_settings;

typedef union {
    struct {
        bool jeita_battery_voltage      : 1; // Bit 0
        bool thermal_threshold          : 1; // Bit 1
        bool charge_timer               : 1; // Bit 2
        bool enable_safety_timer        : 1; // Bit 3
        uint8_t watchdog_timer          : 2; // Bits 4-5
        uint8_t __reserved              : 1; // Bit 6
        bool en_termination             : 1; // Bit 7
    };
    uint8_t raw_safety_settings;
} bq25619_safety_settings;

// --- MACROS ---
#define BQ25619_ENCODE_CHARGE_CURRENT(ma) (     \
    ((ma) >= 1500) ? 0b111111 :                 \
    ((ma) >= 1430) ? 0b111110 :                 \
    ((ma) >= 1360) ? 0b111101 :                 \
    ((ma) >= 1290) ? 0b111100 :                 \
    ((ma) >= 1180) ? 0b111011 :                 \
    (ma / BQ25619_CHARGE_CURRENT_STEP)          \
)

#define BQ25619_ENCODE_CHARGE_VOLTAGE(voltage_mv) (                 \
    (voltage_mv <= 3504) ? 0b00000 :                                \
    (voltage_mv <= 3600) ? 0b00001 :                                \
    (voltage_mv <= 3696) ? 0b00010 :                                \
    (voltage_mv <= 3800) ? 0b00011 :                                \
    (voltage_mv <= 3904) ? 0b00100 :                                \
    (voltage_mv <= 4000) ? 0b00101 :                                \
    (voltage_mv <= 4100) ? 0b00110 :                                \
    (voltage_mv <= 4150) ? 0b00111 :                                \
    (voltage_mv <= 4200) ? 0b01000 :                                \
    (voltage_mv <= 4300) ? 0b01001 :                                \
    (voltage_mv >= 4520) ? 0b11111 :                                \
    ((voltage_mv - 4300) / BQ25619_CHARGE_VOLTAGE_STEP + 0b01001)   \
)

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Edited to suit SisyFOSS's board - Plajta 2025 - Václav Straka
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

// pico_cmake_set PICO_PLATFORM=rp2040

#ifndef _BOARDS_PLAJTA_SISYFOSS_BETA_H
#define _BOARDS_PLAJTA_SISYFOSS_BETA_H

// For board detection
#define PLAJTA_SISYFOSS_BETA

// On some samples, the xosc can take longer to stabilize than is usual
// TODO: TEST IF ACTUALLY NEEDED
// #ifndef PICO_XOSC_STARTUP_DELAY_MULTIPLIER
// #define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64
// #endif

//------------- UART -------------//
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif

#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif

#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif

//------------- LED -------------//
#ifndef PICO_DEFAULT_WS2812_PIN
#define PICO_DEFAULT_WS2812_PIN 21
#endif

//------------- I2C -------------//
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif

#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 24
#endif

#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 25
#endif

//------------- FLASH -------------//

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2 // TEST IF STABLE
#endif
// pico_cmake_set_default PICO_FLASH_SIZE_BYTES = (16 * 1024 * 1024)
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif
// pico_cmake_set_default LITTLEFS_SIZE_BYTES_STRING = (15 * 1024 * 1024)
#ifndef LITTLEFS_SIZE_BYTES_STRING
#define LITTLEFS_SIZE_BYTES_STRING (15 * 1024 * 1024)
#endif
// All boards have B1 RP2040
#ifndef PICO_RP2040_B0_SUPPORTED
#define PICO_RP2040_B0_SUPPORTED 0
#endif

#endif

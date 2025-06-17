cmake_minimum_required(VERSION 3.13)

set(DEVICE_NAME "Sisyphus" CACHE STRING "The device name used for the \"info\" command")
option(SISYPHUS_DEBUG "Enable debug flag" OFF)
option(USE_ETERNITY "Enable usage of Eternity serial bootloader" OFF)

if(NOT DEFINED GIT_COMMIT_SHA OR GIT_COMMIT_SHA STREQUAL "")
    # Retrieve the Git commit hash
    execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Check for uncommitted changes by looking at the output of git diff
    execute_process(
    COMMAND git diff --quiet
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GIT_DIFF_RESULT
    )

    # If there are uncommitted changes, append -dirty to the commit SHA
    if(NOT GIT_DIFF_RESULT EQUAL 0)
        set(GIT_COMMIT_SHA "${GIT_COMMIT_SHA}-dirty")
    endif()
endif()

# Get the current build date and time
execute_process(
  COMMAND date "+%Y-%m-%d,%H:%M:%S"
  OUTPUT_VARIABLE BUILD_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(SISYPHUS_DEBUG)
    add_compile_definitions(SISYPHUS_DEBUG)
endif()

set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_EXTRAS_FETCH_FROM_GIT on)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

# Might come in handy later
set(PICO_BOARD plajta_sisyfoss_beta)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Required for clangd

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(sisyphus C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
  main.c
  audio/audio.c
  protocol/protocol.c
  usb/usb_descriptors.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  PICO_AUDIO_I2S_MONO_INPUT=1
  PICO_AUDIO_I2S_MONO_OUTPUT=1
  GIT_COMMIT_SHA="${GIT_COMMIT_SHA}"
  DEVICE_NAME="${DEVICE_NAME}"
  BUILD_DATE="${BUILD_DATE}"
  USE_ETERNITY=$<BOOL:${USE_ETERNITY}>
)

# If the board doesn't define this, provide a default; this will fit the majority of boards.
if(NOT DEFINED LITTLEFS_SIZE_BYTES_STRING)
    set(LITTLEFS_SIZE_BYTES_STRING "(1024 * 1024)") # Size in bytes eg. 512 * 1024 = 512kB
endif()

# Convert both strings to integers
math(EXPR LITTLEFS_SIZE_BYTES "${LITTLEFS_SIZE_BYTES_STRING}")
math(EXPR FLASH_SIZE_BYTES "${PICO_FLASH_SIZE_BYTES}")

if(LITTLEFS_SIZE_BYTES GREATER_EQUAL FLASH_SIZE_BYTES)
    message(FATAL_ERROR "LITTLEFS Partition size must be smaller than flash size of ${PICO_SIZE}!")
endif()

add_subdirectory(util)
add_subdirectory(keyboard)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib pico_audio_i2s pico_bootrom hardware_i2c
                        littlefs_pico tinyusb_device tinyusb_board keyboard_tca8418)

target_include_directories(${PROJECT_NAME} PUBLIC protocol audio usb)

pico_add_extra_outputs(${PROJECT_NAME})

# Binary info

pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_description(${PROJECT_NAME} "Basically a physical soundboard")
pico_set_program_version(${PROJECT_NAME} "0.1")
pico_set_program_url(${PROJECT_NAME} "https://plajta.eu")

# Flash setup

if(USE_ETERNITY)
    add_subdirectory(eternity)
endif()

target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)

if (USE_ETERNITY)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/memmap_eternity.ld.template ${CMAKE_CURRENT_BINARY_DIR}/memmap_custom.ld)
else()
    configure_file(${CMAKE_CURRENT_LIST_DIR}/memmap_stage2.ld.template ${CMAKE_CURRENT_BINARY_DIR}/memmap_custom.ld)
endif()

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/memmap_custom.ld)

set(RESET_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/sisyphus_reset.sh)

add_custom_target(flash
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND sh -c "${RESET_SCRIPT} 2>&1"
    COMMENT "Resetting, and flashing the RP2040"
    USES_TERMINAL
)

if(USE_ETERNITY)
    add_custom_command(TARGET flash POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/eternity/protocol/flasher.py ${PROJECT_NAME}.bin
    )
else()
    add_custom_command(TARGET flash POST_BUILD
        COMMAND picotool load -f -x ${PROJECT_NAME}.uf2
    )
endif()

if (USE_ETERNITY)
    # Convert string to integer
    math(EXPR ETERNITY_SIZE_BYTES "${ETERNITY_SIZE_BYTES_STRING}")

    set(OUTPUT_BIN "${CMAKE_BINARY_DIR}/combined.bin")

    # Run the script after building main_app
    add_custom_command(
        OUTPUT ${OUTPUT_BIN}
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/eternity/merge_binaries.py
            ${CMAKE_BINARY_DIR}/eternity/eternity.bin
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
            ${ETERNITY_SIZE_BYTES}
            ${OUTPUT_BIN}
        DEPENDS ${PROJECT_NAME} eternity
        COMMENT "Generating combined binary"
        VERBATIM
    )

    if(NOT PICO_NO_PICOTOOL)
        set(OUTPUT_UF2 "${CMAKE_BINARY_DIR}/combined.uf2")

        add_custom_command(
            OUTPUT ${OUTPUT_UF2}
            COMMAND picotool uf2 convert --quiet --family rp2040 ${OUTPUT_BIN} ${OUTPUT_UF2}
            DEPENDS ${OUTPUT_BIN}
            COMMENT "Generating combined UF2"
            VERBATIM
        )
        add_custom_target(combine_eternity ALL
            DEPENDS ${OUTPUT_BIN} ${OUTPUT_UF2}
        )
    else()
        message(STATUS "PICO_NO_PICOTOOL is set — only generating .bin")
        add_custom_target(combine_eternity ALL
            DEPENDS ${OUTPUT_BIN}
        )
    endif()
endif()

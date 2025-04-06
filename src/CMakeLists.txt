cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_EXTRAS_FETCH_FROM_GIT on)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Required for clangd

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(sisyphus C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_library(littlefs STATIC
  littlefs/lfs.c
  littlefs/lfs_util.c
)

target_include_directories(littlefs PUBLIC littlefs)

add_executable(${PROJECT_NAME}
  main.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib pico_audio_i2s
                        littlefs)

# pico_enable_stdio_usb(${PROJECT_NAME} 1)
# pico_enable_stdio_uart(${PROJECT_NAME} 0)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  PICO_AUDIO_I2S_MONO_INPUT=1
  PICO_AUDIO_I2S_MONO_OUTPUT=1
)

pico_add_extra_outputs(${PROJECT_NAME})

# Binary info

pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_description(${PROJECT_NAME} "Basically a physical soundboard")
pico_set_program_version(${PROJECT_NAME} "0.1")
pico_set_program_url(${PROJECT_NAME} "https://plajta.eu")

# Flash setup

target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)

set(LITTLEFS_SIZE_BYTES_STRING "(1024 * 1024)") # Size in bytes eg. 512 * 1024 = 512kB

# Convert both strings to integers
math(EXPR LITTLEFS_SIZE_BYTES "${LITTLEFS_SIZE_BYTES_STRING}")
math(EXPR FLASH_SIZE_BYTES "${PICO_FLASH_SIZE_BYTES}")

if(LITTLEFS_SIZE GREATER_EQUAL FLASH_SIZE_BYTES)
    message(FATAL_ERROR "LITTLEFS Partition size must be smaller than flash size of ${PICO_SIZE}!")
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/memmap_custom.ld ${CMAKE_BINARY_DIR}/memmap_custom.ld)
pico_set_linker_script(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/memmap_custom.ld)

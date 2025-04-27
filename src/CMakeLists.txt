cmake_minimum_required(VERSION 3.13)

option(SISYPHUS_DEBUG "Enable debug flag" OFF)

# Temporary workaround before making custom tusb configs
add_definitions(-DCFG_TUD_CDC_TX_BUFSIZE=1024)
add_definitions(-DCFG_TUD_CDC_RX_BUFSIZE=1024)

# Retrieve the Git commit hash
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_SHA
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check for uncommitted changes by looking at the output of git diff
execute_process(
  COMMAND git diff --quiet
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE GIT_DIFF_RESULT
)

# Get the current build date and time
execute_process(
  COMMAND date "+%Y-%m-%d,%H:%M:%S"
  OUTPUT_VARIABLE BUILD_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# If there are uncommitted changes, append -dirty to the commit SHA
if(NOT GIT_DIFF_RESULT EQUAL 0)
    set(GIT_COMMIT_SHA "${GIT_COMMIT_SHA}-dirty")
endif()

if(SISYPHUS_DEBUG)
    add_compile_definitions(SISYPHUS_DEBUG)
endif()

set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_EXTRAS_FETCH_FROM_GIT on)

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
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  PICO_AUDIO_I2S_MONO_INPUT=1
  PICO_AUDIO_I2S_MONO_OUTPUT=1
  GIT_COMMIT_SHA="${GIT_COMMIT_SHA}"
  PROJECT_NAME="${PROJECT_NAME}"
  BUILD_DATE="${BUILD_DATE}"
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

set(LITTLEFS_SIZE_BYTES_STRING "(1024 * 1024)") # Size in bytes eg. 512 * 1024 = 512kB

# Convert both strings to integers
math(EXPR LITTLEFS_SIZE_BYTES "${LITTLEFS_SIZE_BYTES_STRING}")
math(EXPR FLASH_SIZE_BYTES "${PICO_FLASH_SIZE_BYTES}")

if(LITTLEFS_SIZE_BYTES GREATER_EQUAL FLASH_SIZE_BYTES)
    message(FATAL_ERROR "LITTLEFS Partition size must be smaller than flash size of ${PICO_SIZE}!")
endif()

add_subdirectory(util)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib pico_audio_i2s
                        littlefs_pico)

target_include_directories(${PROJECT_NAME} PUBLIC protocol audio)

pico_add_extra_outputs(${PROJECT_NAME})

# Binary info

pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_description(${PROJECT_NAME} "Basically a physical soundboard")
pico_set_program_version(${PROJECT_NAME} "0.1")
pico_set_program_url(${PROJECT_NAME} "https://plajta.eu")

# Flash setup

target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)

configure_file(${CMAKE_CURRENT_LIST_DIR}/memmap_custom.ld ${CMAKE_BINARY_DIR}/memmap_custom.ld)
pico_set_linker_script(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/memmap_custom.ld)


add_custom_target(flash
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}  # Build the project
    COMMAND picotool load -f -x ${PROJECT_NAME}.uf2  # Flash the device
    DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2  # Ensure that the .uf2 file exists before flashing
    COMMENT "Building and flashing the RP2040"
)

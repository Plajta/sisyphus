add_library(littlefs STATIC
  ../littlefs/lfs.c
  ../littlefs/lfs_util.c
)

target_include_directories(littlefs PUBLIC ../littlefs)

# littlefs_pico
add_library(littlefs_pico STATIC
    littlefs-pico/littlefs-pico.c
)

target_compile_definitions(littlefs_pico PUBLIC
    LITTLEFS_SIZE_BYTES=${LITTLEFS_SIZE_BYTES}
)

target_include_directories(littlefs_pico PUBLIC littlefs-pico)

target_link_libraries(littlefs_pico PUBLIC littlefs hardware_flash hardware_sync)

add_subdirectory(error)

add_library(littlefs STATIC
  ../littlefs/lfs.c
  ../littlefs/lfs_util.c
)

target_include_directories(littlefs PUBLIC ../littlefs)

add_library(util STATIC
    littlefs-pico.c
)

target_compile_definitions(util PUBLIC
    LITTLEFS_SIZE_BYTES=${LITTLEFS_SIZE_BYTES}
)

target_include_directories(util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(util PUBLIC littlefs hardware_flash hardware_sync)

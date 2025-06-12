add_library(keyboard_tca8418 STATIC
    tca8418.c
)

target_include_directories(keyboard_tca8418 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(keyboard_tca8418 PUBLIC hardware_i2c hardware_gpio)

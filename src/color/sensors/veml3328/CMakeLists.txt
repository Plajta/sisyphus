add_library(color_sensor_veml3328 STATIC
    veml3328.c
)

target_include_directories(color_sensor_veml3328 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(color_sensor_veml3328 PUBLIC hardware_i2c)

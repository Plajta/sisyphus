add_library(charger_bq25619 STATIC bq25619.c)

target_include_directories(charger_bq25619 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/util/error)

target_link_libraries(charger_bq25619 PUBLIC hardware_i2c hardware_gpio pico_stdlib)

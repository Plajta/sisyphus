add_library(lightshow STATIC lightshow.c)

target_include_directories(lightshow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lightshow PUBLIC pico_stdlib hardware_timer)

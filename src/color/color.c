#include "lfs.h"
#include <hardware/gpio.h>
#include <pico.h>
#include <pico/time.h>
#include "color.h"

#ifdef SISYFOSS_COLOR_VEML3328
#include "sensors/veml3328/veml3328.h"
extern i2c_inst_t *sisyfoss_i2c_inst;
#endif

extern lfs_t lfs;
struct color_match_array color_lut;

int color_lut_load(){
    lfs_file_t lut_file;
    int8_t error;

    error = lfs_file_open(&lfs, &lut_file, "color_lookup_table", LFS_O_RDONLY);
    if (error != LFS_ERR_OK) return -1;

    int32_t file_size = lfs_file_size(&lfs, &lut_file);
    if (file_size % LUT_LINE_SIZE != 0) return -1;

    uint8_t entry_num = file_size / LUT_LINE_SIZE;

    struct color_entry *color_entries = malloc(entry_num * sizeof(struct color_entry));
    if (!color_entries) return -1;

    free(color_lut.data);
    color_lut.data = color_entries;
    color_lut.len = entry_num;

    char buffer[LUT_LINE_SIZE];
    for (int i; i < entry_num; i++) {
        lfs_file_read(&lfs, &lut_file, buffer, sizeof(buffer));
        char *endptr;

        uint32_t result = strtoul(buffer, &endptr, 16);

        if (endptr - buffer != 6){
            return -1;
        }

        color_lut.data[i].blue = result & 0xFF;
        color_lut.data[i].green = (result >> 8) & 0xFF;
        color_lut.data[i].red = (result >> 16) & 0xFF;

        color_lut.data[i].color_name = buffer[7];
    }

    lfs_file_close(&lfs, &lut_file);

    return 0;
}

int color_init() {
    int err;

    err = color_lut_load();
    if (err != PICO_ERROR_NONE){
        return err;
    }

    #ifdef SISYFOSS_AUX_SENSOR_LED
    gpio_init(SISYFOSS_AUX_SENSOR_LED);
    gpio_set_dir(SISYFOSS_AUX_SENSOR_LED, GPIO_OUT);
    gpio_put(SISYFOSS_AUX_SENSOR_LED, true);
    #endif

    return PICO_OK;
}

int color_read_sensor(color_measurement *color) {

    #ifdef SISYFOSS_AUX_SENSOR_LED
    gpio_put(SISYFOSS_AUX_SENSOR_LED, false);
    #endif

    #ifdef SISYFOSS_COLOR_VEML3328
    veml3328_shutdown(sisyfoss_i2c_inst, false); // Wake the sensor
    #endif

    #ifdef SISYFOSS_AUX_SENSOR_LED
    sleep_ms(100); // So the light data has time to integrate
    #endif

    #ifdef SISYFOSS_COLOR_VEML3328
    veml3328_read_color(sisyfoss_i2c_inst, color);
    #endif

    #ifdef SISYFOSS_AUX_SENSOR_LED
    gpio_put(SISYFOSS_AUX_SENSOR_LED, true);
    #endif

    #ifdef SISYFOSS_COLOR_VEML3328
    veml3328_shutdown(sisyfoss_i2c_inst, true); // Sleep the sensor
    #endif

    return PICO_OK;
}

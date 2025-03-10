#ifndef ESP32_SMARTDISPLAY_H
#define ESP32_SMARTDISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

// Use last PWM_CHANNEL for backlight
#define PWM_CHANNEL_BCKL (SOC_LEDC_CHANNEL_NUM - 1)
#ifndef PWM_FREQ_BCKL
#define PWM_FREQ_BCKL 20000
#endif
#ifndef PWM_BITS_BCKL
#define PWM_BITS_BCKL 8
#endif
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)

// Exported functions
#ifdef __cplusplus
extern "C"
{
#endif
    // Structure to store the data from the three point calibration data
    typedef struct
    {
        bool valid;
        float alphaX;
        float betaX;
        float deltaX;
        float alphaY;
        float betaY;
        float deltaY;
    } touch_calibration_data_t;

    // Initialize the display and touch
    void smartdisplay_init();
#ifdef BOARD_HAS_TOUCH
    // Touch calibration
    extern touch_calibration_data_t touch_calibration_data;
    touch_calibration_data_t smartdisplay_compute_touch_calibration(const lv_point_t screen[3], const lv_point_t touch[3]);

    typedef void (*touch_cb_t)(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
    void smartdisplay_set_touch_cb(touch_cb_t cb);
#endif
    // Set the brightness of the backlight display
    void smartdisplay_lcd_set_backlight(float duty); // [0, 1]
    // Set the adaptive brightness callback
    typedef float (*smartdisplay_lcd_adaptive_brightness_cb_t)();
    void smartdisplay_lcd_set_brightness_cb(smartdisplay_lcd_adaptive_brightness_cb_t cb, uint interval);
#ifdef BOARD_HAS_RGB_LED
    void smartdisplay_led_set_rgb(bool r, bool g, bool b);
#endif
#ifdef __cplusplus
}
#endif

#endif
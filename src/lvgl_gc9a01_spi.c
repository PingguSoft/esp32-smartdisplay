#ifdef LCD_GC9A01_SPI

#include <esp32_smartdisplay.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

#include "esp_lcd_gc9a01.h"

bool gc9a01_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

void gc9a01_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color16_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = drv->user_data;
#if LV_COLOR_16_SWAP != 1
#warning "LV_COLOR_16_SWAP should be 1 for max performance"
    ushort pixels = lv_area_get_size(area);
    lv_color16_t *p = color_map;
    while (pixels--)
        p++->full = (uint16_t)((p->full >> 8) | (p->full << 8));
#endif
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
};

void lvgl_lcd_init(lv_disp_drv_t *drv)
{
    log_d("lvgl_lcd_init");
    // Hardware rotation is supported
    drv->sw_rotate = 0;
    drv->rotated = LV_DISP_ROT_NONE;

    // Create SPI bus
    const spi_bus_config_t spi_bus_config = {
        .mosi_io_num = GC9A01_SPI_BUS_MOSI_IO_NUM,
        .miso_io_num = GC9A01_SPI_BUS_MISO_IO_NUM,
        .sclk_io_num = GC9A01_SPI_BUS_SCLK_IO_NUM,
        .quadwp_io_num = GC9A01_SPI_BUS_QUADWP_IO_NUM,
        .quadhd_io_num = GC9A01_SPI_BUS_QUADHD_IO_NUM,
        .max_transfer_sz = GC9A01_SPI_BUS_MAX_TRANSFER_SZ,
        .flags = GC9A01_SPI_BUS_FLAGS,
        .intr_flags = GC9A01_SPI_BUS_INTR_FLAGS};
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(GC9A01_SPI_HOST, &spi_bus_config, GC9A01_SPI_DMA_CHANNEL));

    // Attach the LCD controller to the SPI bus
    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = GC9A01_SPI_CONFIG_CS_GPIO_NUM,
        .dc_gpio_num = GC9A01_SPI_CONFIG_DC_GPIO_NUM,
        .spi_mode = GC9A01_SPI_CONFIG_SPI_MODE,
        .pclk_hz = GC9A01_SPI_CONFIG_PCLK_HZ,
        .trans_queue_depth = GC9A01_SPI_CONFIG_TRANS_QUEUE_DEPTH,
        .user_ctx = drv,
        .on_color_trans_done = gc9a01_color_trans_done,
        .lcd_cmd_bits = GC9A01_SPI_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = GC9A01_SPI_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_as_cmd_phase = GC9A01_SPI_CONFIG_FLAGS_DC_AS_CMD_PHASE,
            .dc_low_on_data = GC9A01_SPI_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .octal_mode = GC9A01_SPI_CONFIG_FLAGS_OCTAL_MODE,
            .lsb_first = GC9A01_SPI_CONFIG_FLAGS_LSB_FIRST}};
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)GC9A01_SPI_HOST, &io_config, &io_handle));

    // Create gc9a01 panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = GC9A01_DEV_CONFIG_RESET_GPIO_NUM,
        .color_space = GC9A01_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = GC9A01_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = GC9A01_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = GC9A01_DEV_CONFIG_VENDOR_CONFIG};
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_dev_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // Colors are inverted
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    // Turn display on
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    drv->user_data = panel_handle;
    drv->flush_cb = gc9a01_lv_flush;
}

#endif
/*
    --==ESP32 C3 GPS Speedometer for GRT Trikes==--
    This is a simple GPS Speedo that can be used in just about anything, 
    but it is designed for use in Energy Breakthrough trikes.
    
    --==Parts List==--
    TENSTAR ROBOT ESP32 C3 Supermicro
    320x170 ST7789 1.9" LCD Display Module
    NEO-6M GPS Reciever Module + GPS Antenna
    
    --==Pin Definitions==--
    Module Pin          ESP32 Pin
    GPS TX              20
    GPS RX              21
    Display MOSI        6
    Display SCK         4
    Display CS          7
    Display DC          3
    Display RST         2
    Display Backlight   1
*/

#include <Arduino.h>
#include <stdint.h>
#include <TFT_eSPI.h>
#include "lvgl/lvgl.h"
#include "ui/ui_outer.h"
#include <TinyGPSPlus.h>

/* Setup */
int8_t tx = 21;
int8_t rx = 20;
unsigned long gps_baud = 9600;

uint8_t led = 8;
uint8_t LED_ON = LOW;
uint8_t LED_OFF = HIGH;

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 170;

/* Buffers */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[5440];

/* Objects */
TFT_eSPI tft = TFT_eSPI();

TinyGPSPlus gps;

void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void setup()
{
    tft.begin();
    tft.setRotation(1);
    Serial1.begin(gps_baud, SERIAL_8N1, rx, tx);
    pinMode(led, OUTPUT);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 5440);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    ui_init();
}

void loop()
{
    if (Serial1.available())
    {
        gps.encode(Serial1.read());
    }

    if (gps.time.isValid())
    {
        lv_label_set_text_fmt(ui_time, "%i:%i:%i", gps.time.hour(), gps.time.minute(), gps.time.second());
    }
    else 
    {
        lv_label_set_text(ui_time, "No Signal");
    }

    if (gps.satellites.isValid())
    {
        lv_label_set_text_fmt(ui_sats, "%i", gps.satellites.value());
    }
    else 
    {
        lv_label_set_text(ui_sats, "-");
    }


    if (gps.speed.isValid())
    {
        lv_label_set_text_fmt(ui_speed, "%d",gps.speed.kmph());
        if (gps.speed.kmph() > 60)
        {
            lv_obj_set_style_text_color(ui_speed, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            digitalWrite(led, LOW);
        }
        else 
        {
            lv_obj_set_style_text_color(ui_speed, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            digitalWrite(led, HIGH);
        }
    }
    else
    {
        lv_label_set_text(ui_speed, "N/A");
    }
    
    lv_timer_handler();
}

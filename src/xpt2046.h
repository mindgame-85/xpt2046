#ifndef ARDUINO_H
#define ARDUINO_H
#include <Arduino.h>
#endif

#ifndef SPI_H
#define SPI_H
#include <SPI.h>
#endif


#ifndef XPT2046_h
#define XPT2046_h

class XPT2046 {
    private:
        SPIClass    *_spi_class;

        uint32_t    _spi_frequency;
        uint8_t     _spi_bit_order;
        uint8_t     _spi_mode;

        uint8_t     _pin_cs;

        uint8_t     _rotation = 0;
        uint16_t    _screen_width = 480;
        uint16_t    _screen_height = 320;

        uint16_t    _x_raw_min = 240;
        uint16_t    _x_raw_max = 3850;
        uint16_t    _y_raw_min = 180;
        uint16_t    _y_raw_max = 3850;

        uint16_t    _point_x_raw;
        uint16_t    _point_y_raw;

        uint16_t    _points_x_raw[5];
        uint16_t    _points_y_raw[5];

    public:
        uint8_t     flag_touch;

        uint16_t    point_x;
        uint16_t    point_y;
        
    private:
        uint16_t    read_raw_x();
        uint16_t    read_raw_y();
        void        apply_rotation();
        void        convert_raw_to_pixel();

    public:
        XPT2046(SPIClass *SPI_CLASS);

        void        set_pins(uint8_t PIN_CS);
        void        begin();

        void        set_rotation(uint8_t rotation);

        void        handle_touch();


};


#endif

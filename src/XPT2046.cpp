#include "xpt2046.h"

#include "debug_printer.h"




XPT2046::XPT2046(SPIClass *SPI_CLASS) {
    _spi_class = SPI_CLASS;
    _spi_frequency = 2E6;
    _spi_bit_order = MSBFIRST;
    _spi_mode = 0;
}


void XPT2046::set_pins(uint8_t PIN_CS) {
    _pin_cs = PIN_CS;
}


void XPT2046::begin() {
    pinMode(_pin_cs, OUTPUT);
    digitalWrite(_pin_cs, HIGH);
}


void XPT2046::set_rotation(uint8_t rotation) {
    if (rotation < 4) {
        _rotation = rotation;
    }

    switch (_rotation) {
        case 3:
            _x_raw_min = 240;
            _x_raw_max = 3900;
            _y_raw_min = 210;
            _y_raw_max = 3830;
            break;
    }
}


uint16_t XPT2046::read_raw_x() {
    uint8_t control_byte = 0b11010011;      // X-Position, 12 bit conversion, differential mode, ADC und REF permanent ON
    uint8_t high_byte = 0;
    uint8_t low_byte = 0;
    uint16_t raw_value = 0;

    _spi_class->beginTransaction(SPISettings(_spi_frequency, MSBFIRST, _spi_mode));
    digitalWrite(_pin_cs, LOW);
    _spi_class->transfer(control_byte);
    high_byte = _spi_class->transfer(0x00);
    low_byte = _spi_class->transfer(0x00);
    digitalWrite(_pin_cs, HIGH);
    _spi_class->endTransaction();

    //_point_x_raw = (low_byte >> 3) | (high_byte << 5);
    raw_value = ((high_byte << 8) | low_byte) >> 3;

    if (raw_value > 4095) {
        _point_x_raw = 0;
    }
    else {
        _point_x_raw = raw_value;
    }

    return _point_x_raw;
}


uint16_t XPT2046::read_raw_y() {
    uint8_t control_byte = 0b10010011;      // Y-Position, 12 bit conversion, differential mode, ADC und REF permanent ON
    uint8_t high_byte = 0;
    uint8_t low_byte = 0;
    uint16_t raw_value = 0;

    _spi_class->beginTransaction(SPISettings(_spi_frequency, MSBFIRST, _spi_mode));
    digitalWrite(_pin_cs, LOW);
    _spi_class->transfer(control_byte);
    high_byte = _spi_class->transfer(0x00);
    low_byte = _spi_class->transfer(0x00);
    digitalWrite(_pin_cs, HIGH);
    _spi_class->endTransaction();

    //raw_value = (low_byte >> 3) | (high_byte << 5);
    raw_value = ((high_byte << 8) | low_byte) >> 3;

    if (raw_value > 4095) {
        _point_y_raw = 0;
    }
    else {
        _point_y_raw = raw_value;
    }
    return _point_y_raw;
}


void XPT2046::apply_rotation() {
    uint16_t temp = 0;
    
    switch (_rotation) {
        case 0:
            _point_x_raw = 4095 - _point_x_raw;
            _point_y_raw = 4095 - _point_y_raw;
            break;
        case 1:
            temp = _point_x_raw;
            _point_x_raw = 4095 - _point_y_raw;
            _point_y_raw = temp;
            break;
        case 2:
            break;
        case 3:
            temp = _point_x_raw;
            _point_x_raw =  _point_y_raw;
            _point_y_raw = 4095 - temp;
            break;
    }

    if (_point_x_raw > 4094) {
        _point_x_raw = 0;
    }
    if (_point_y_raw > 4094) {
        _point_y_raw = 0;
    }
}


void XPT2046::convert_raw_to_pixel() {

    point_x = int((float(_screen_width) / (float(_x_raw_max) - float(_x_raw_min))) * (float(_point_x_raw) - float(_x_raw_min)));
    point_y = int((float(_screen_height) / (float(_y_raw_max) - float(_y_raw_min))) * (float(_point_y_raw) - float(_y_raw_min)));

    if (point_x > _screen_width) {
        point_x = 0;
    }
    if (point_y > _screen_height) {
        point_y = 0;
    }
}


void XPT2046::handle_touch() {
    // Read raw values
    read_raw_x();
    read_raw_y();
    if ((_point_x_raw > 0) && (_point_y_raw > 0)) {
        // Display is touched, set flag
        flag_touch = 1;
        // Clear point-arrays
        memset(_points_x_raw, 0, sizeof(_points_x_raw));
        memset(_points_y_raw, 0, sizeof(_points_x_raw));
        // First measurement is not precise, redo 5 readings
        delayMicroseconds(10);

        _points_x_raw[0] = read_raw_x();
        _points_y_raw[0] = read_raw_y();

        uint16_t min_x = _points_x_raw[0];
        uint16_t max_x = _points_x_raw[0];
        uint16_t min_y = _points_y_raw[0];
        uint16_t max_y = _points_y_raw[0];

        for (uint8_t i = 1; i < 5; i++) {
            _points_x_raw[i] = read_raw_x();
            _points_y_raw[i] = read_raw_y();

            if (_points_x_raw[i] < min_x) {
                min_x =  _points_x_raw[i];
            }
            if (_points_x_raw[i] > max_x) {
                max_x =  _points_x_raw[i];
            }
            
            if (_points_y_raw[i] < min_y) {
                min_y =  _points_y_raw[i];
            }
            if (_points_y_raw[i] > max_y) {
                max_y =  _points_y_raw[i];
            }
        }
        
        uint16_t mean_x_raw = 0;
        uint16_t mean_y_raw = 0;

        for (uint8_t k = 0; k < 5; k++) {
            mean_x_raw = mean_x_raw + _points_x_raw[k];
            mean_y_raw = mean_y_raw + _points_y_raw[k];
        }

        _point_x_raw = (mean_x_raw - max_x - min_x) / 3;
        _point_y_raw = (mean_y_raw - max_y - min_y) / 3;

        // Apply rotation to raw values
        apply_rotation();
        // Convert raw values into pixel values
        convert_raw_to_pixel();

        DEBUG_PRINTLN("DEBUG: INFO - Touched at:");
        DEBUG_PRINT("DEBUG: INFO - X = ");
        DEBUG_PRINT(point_x);
        DEBUG_PRINTLN(" pixel");
        DEBUG_PRINT("DEBUG: INFO - Y = ");
        DEBUG_PRINT(point_y);
        DEBUG_PRINTLN(" pixel");
    
    }
    else {
        point_x = 0;
        point_y = 0;
        flag_touch = 0;
    }
}

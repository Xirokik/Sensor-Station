
#ifndef BMP280_PORT_H
#define BMP280_PORT_H

#include <stdint.h>
#include <stdlib.h>

uint32_t bmp280_port_write(uint8_t reg, const uint8_t *data, uint16_t len);

uint32_t bmp280_port_read(uint8_t reg, uint8_t *data, uint16_t len);

#endif /* BMP280_PORT_H */
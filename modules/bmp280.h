
#ifndef BMP280_H_
#define BMP280_H_

#include <stdint.h>
#include <stdbool.h>

void bmp280_init(void);

void bmp280_update(void);

bool bmp280_is_init(void);

float bmp280_get_temp_C(void);

uint32_t bmp280_get_pres_Pa(void);

float bmp280_get_pres_hPa(void);

#endif /* BMP280_H */

#ifndef ENCODER_DRIVER_H_
#define ENCODER_DRIVER_H_

#include "stdint.h"
#include "stdbool.h"

#define ENCODER_FLT_INIT 0x111
#define ENCODER_POSITIONS 20U

uint16_t encoder_init(void);

uint32_t encoder_get_pos(void);

bool is_encoder_button_pressed(void);

#endif /* ENCODER_DRIVER_H_ */

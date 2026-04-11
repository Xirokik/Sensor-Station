#ifndef LOOP_H_
#define LOOP_H_

#include <stdint.h>
#include <stdbool.h>

#define LOOP_FLT_INIT 0x120

uint16_t loop_init(void);

bool is_step_ready(void);

void step_done(void);

uint32_t loop_get_all_steps(void);

uint32_t loop_get_missed_steps(void);

#endif /* LOOP_H_ */

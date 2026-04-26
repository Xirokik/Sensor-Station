#ifndef CONSOLE_CONSOLE_DRIVER_H_
#define CONSOLE_CONSOLE_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stdbool.h>

// Error codes
#define CONSOLE_BAD_ARG         0x100
#define CONSOLE_INIT_FAILED		0x101
#define CONSOLE_TX_FAIL         0x102
#define CONSOLE_TX_BUSY         0x103
#define CONSOLE_RX_BUF_EMPTY    0x105

uint16_t console_driver_init(void);
uint16_t console_driver_send(const void *tx_data, uint16_t len);
uint16_t console_driver_recv(uint8_t *rx_char);
uint32_t console_driver_lost_char(void);
bool console_driver_is_tx_ready(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* CONSOLE_CONSOLE_DRIVER_H_ */

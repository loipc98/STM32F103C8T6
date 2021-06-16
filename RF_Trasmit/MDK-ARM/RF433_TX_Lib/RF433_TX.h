/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RF433_TX_H
#define __RF433_TX_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Function ------------------------------------------------------------------*/
#ifndef VW_TX_Pin
#define VW_TX_Pin GPIO_PIN_3
#endif

#ifndef VW_TX_GPIO_Port
#define VW_TX_GPIO_Port GPIOB
#endif

#define lo8(x) ((x)&0xff) 
#define hi8(x) ((x)>>8)

#define VW_MAX_MESSAGE_LEN 80
#define VW_MAX_PAYLOAD VW_MAX_MESSAGE_LEN-3
#define VW_HEADER_LEN 8

#define vw_digitalWrite_tx(value) HAL_GPIO_WritePin(VW_TX_GPIO_Port,VW_TX_Pin,(GPIO_PinState)value)

extern void vw_wait_tx(void);

extern uint16_t _crc_ccitt_update (uint16_t crc, uint8_t data);

extern void vw_tx_start(void);

extern void vw_tx_stop(void);

extern uint8_t vw_send(uint8_t* buf, uint8_t len);

extern void transmit_Data_In_Interrupt(void);

#endif /*__RF433_H */


#include "stdint.h"
#include "mb.h"
#include "port.h"

#define FLAG_FD 0x7E // Флаг 

eMBErrorCode Res_bitstaffing(volatile uint8_t* HDLC_RXTX_buffer, volatile uint16_t Length_HDLC_RXTX_buffer, volatile uint8_t* HDLC_buffer);
uint16_t Trans_bitstaffing(volatile uint8_t* HDLC_buffer, volatile uint16_t Length_HDLC_buffer, volatile uint8_t* HDLC_RXTX_buffer);
eMBErrorCode Decoder (volatile uint8_t* input_buffer, volatile uint16_t *length_input_buffer, volatile uint8_t* output_buffer);
uint16_t Coder (volatile uint8_t* input_buffer, volatile uint16_t length_input_buffer, volatile uint8_t* output_buffer);

/*
 * protocol.h
 *
 *  Created on: Dec 1, 2020
 *      Author: vinagrero
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "stm32l1xx_hal.h"

#define TIMEOUT_TX 500
#define MAX_BUFFER_SIZE 528

typedef enum {
	Idle_State,
	Read_Header_State,
	Transport_State,
	Read_Region_State,
	Read_Sensors_State,
	Execute_Code_State,
} SystemState;

typedef enum {
	  ACK = 1,
	  PING = 2,
	  READ = 3,
	  WRITE = 4,
	  EXEC = 5,
} HeaderType;

typedef enum {
	  MEMORY = 6,
	  SENSORS = 7,
	  CODE = 8,
} BodyType;

typedef struct header_t {
        uint8_t type;
        uint8_t ttl;
        uint8_t crc;

        uint32_t bid_high;
        uint32_t bid_medium;
        uint32_t bid_low;
} __attribute__((packed)) header_t;

typedef struct body_t {
		uint8_t type;
		uint8_t crc;

		uint32_t bid_high;
		uint32_t bid_medium;
		uint32_t bid_low;

		uint16_t mem_address;
		uint8_t data[512];
} body_t;


void init_configuration(void);

uint8_t parse_body(uint8_t *buffer, body_t *body);
uint8_t parse_header(uint8_t *usart_buffer, header_t *header);
void write_mem_values(body_t *body);

void transmit_header(UART_HandleTypeDef *huart, header_t *header);
void transmit_body(UART_HandleTypeDef *huart, body_t *body);
void transmit_ACK(header_t *header);
void transmit_buffer(uint8_t *buffer, uint16_t num_bytes);

SystemState header_handler(header_t *header);
SystemState body_handler(body_t *body);



#endif /* INC_PROTOCOL_H_ */

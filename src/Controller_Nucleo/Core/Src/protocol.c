/*
 * protocol.c
 *
 *  Created on: Dec 1, 2020
 *      Author: vinagrero
 */


#include "protocol.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

uint8_t up_buffer[MAX_BUFFER_SIZE] = {0};
uint8_t down_buffer[MAX_BUFFER_SIZE] = {0};

uint16_t num_bytes_up = 0;
uint16_t num_bytes_down = 0;
uint8_t waiting_read_down = 0;

uint32_t adc_values[2];
uint8_t write_mem_en = 0;


/// Initial peripheral configuration
void init_configuration(void)
{
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&up_buffer, 15);
	HAL_UART_Receive_IT(&huart3, (uint8_t *)&down_buffer, 15);

//	__HAL_ADC_ENABLE(&hadc);
//	HAL_ADC_Start(&hadc);
}


void transmit_buffer(uint8_t *buffer, uint16_t num_bytes)
{
	for(int i = 0; i < num_bytes; ++i) {
		HAL_UART_Transmit(&huart1, (buffer + i), 1, TIMEOUT_TX);
	}
}

/// Parse a header from the received bytes
uint8_t parse_header(uint8_t *usart_buffer, header_t *header)
{
	header->type = usart_buffer[0];
	header->ttl = usart_buffer[1];
	header->crc = usart_buffer[2];

	header->bid_high = (usart_buffer[6] << 24) + (usart_buffer[5] << 16) + (usart_buffer[4] << 8) + (usart_buffer[3]);
	header->bid_medium = (usart_buffer[10] << 24) + (usart_buffer[9] << 16) + (usart_buffer[8] << 8) + (usart_buffer[7]);
	header->bid_low = (usart_buffer[14] << 24) + (usart_buffer[13] << 16) + (usart_buffer[12] << 8) + (usart_buffer[11]);

	return 1;
}

uint8_t parse_body(uint8_t *buffer, body_t *body) {

	body->type = buffer[0];
	body->crc = buffer[1];

	body->bid_high = (buffer[5] << 24) + (buffer[4] << 16) + (buffer[3] << 8) + (buffer[2]);
	body->bid_medium = (buffer[9] << 24) + (buffer[8] << 16) + (buffer[7] << 8) + (buffer[6]);
	body->bid_low = (buffer[13] << 24) + (buffer[12] << 16) + (buffer[11] << 8) + (buffer[10]);




	switch (body->type) {
	case MEMORY:
                body->mem_address = (buffer[15] << 8) + (buffer[14]);

                for(int i = 0; i < 512; ++i) {
                        body->data[i] = buffer[16 + i];
                }
		break;
	case SENSORS:
		break;
	case CODE:
		break;
	}

	return 1;
}

///
/// For the nucleo board FLASH is composed of two banks:
///		0x08000000 -> 0x0803FFFF
///     0x08040000 -> 0x08080000
///
/// Second bank is generally empty
void register_uid(void)
{
	HAL_FLASH_Unlock();

//	uint32_t *address = (uint32_t*)0x0803CCCC;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)0x0803CCCC, 0xabcdef98);

	HAL_FLASH_Lock();
}

uint32_t get_bid_high() {
	uint8_t *uid_p = (uint8_t *)0x1FF800D0;

	return ((uint8_t)*(uid_p)<< 24) + ((uint8_t)*(uid_p + 1) << 16) + ((uint8_t)*(uid_p + 2) << 8) + ((uint8_t)*(uid_p + 3));
}

uint32_t get_bid_medium() {
	uint8_t *uid_p = (uint8_t *)0x1FF800D0;

	return ((uint8_t)*(uid_p + 4)<< 24) + ((uint8_t)*(uid_p + 5) << 16) + ((uint8_t)*(uid_p + 6) << 8) + ((uint8_t)*(uid_p + 7));
}

uint32_t get_bid_low() {
	uint8_t *uid_p = (uint8_t *)0x1FF800e3;

	return ((uint8_t)*(uid_p)<< 24) + ((uint8_t)*(uid_p + 1) << 16) + ((uint8_t)*(uid_p + 2) << 8) + ((uint8_t)*(uid_p + 3));
}

void transmit_header(UART_HandleTypeDef *huart, header_t *header) {
        HAL_UART_Transmit(huart, (uint8_t *)&header->type, 1, TIMEOUT_TX);
        HAL_UART_Transmit(huart, (uint8_t *)&header->ttl, 1, TIMEOUT_TX);

        HAL_UART_Transmit(huart, (uint8_t *)&header->crc, 1, TIMEOUT_TX);

        HAL_UART_Transmit(huart, (uint8_t *)&header->bid_high, 4, TIMEOUT_TX);
        HAL_UART_Transmit(huart, (uint8_t *)&header->bid_medium, 4, TIMEOUT_TX);
        HAL_UART_Transmit(huart, (uint8_t *)&header->bid_low, 4, TIMEOUT_TX);
}

void transmit_body(UART_HandleTypeDef *huart, body_t *body) {
	// Read body to check which regions of memory to read
	// Send header to inform master of the read data
	HAL_UART_Transmit(huart, (uint8_t *)&body->type, 1, TIMEOUT_TX);
	HAL_UART_Transmit(huart, (uint8_t *)&body->crc, 1, TIMEOUT_TX);

	HAL_UART_Transmit(huart, (uint8_t *)&body->bid_high, 4, TIMEOUT_TX);
	HAL_UART_Transmit(huart, (uint8_t *)&body->bid_medium, 4, TIMEOUT_TX);
	HAL_UART_Transmit(huart, (uint8_t *)&body->bid_low, 4, TIMEOUT_TX);

	HAL_UART_Transmit(huart, (uint8_t *)&body->mem_address, 2, TIMEOUT_TX);

	uint8_t *mem = (uint8_t *)0x20000000;

	switch(body->type) {
		case MEMORY:
			for(int i = 0; i < 512; i++) {
				HAL_UART_Transmit(huart, (uint8_t *)(mem + body->mem_address + i), 1, TIMEOUT_TX);
			}
			break;
		case SENSORS:
			break;
		case CODE:
			break;
	}

	// TODO: Implement sensors readout
//	uint16_t *vdd_cal = (uint16_t *)0x1FF800F8;
//	uint16_t *temp30_cal = (uint16_t *)0x1FF800FA;
//	uint16_t *temp110_cal = (uint16_t *)0x1FF800FE;
//
//	uint32_t temp = adc_values[0];
//	uint32_t vdd = adc_values[1];

//	HAL_ADC_Stop_DMA(&hadc);
}

/// Write the values to memory
void write_mem_values(body_t *body) {

        uint8_t *mem = (uint8_t *)0x20000000;

        for(int i = 0; i < 512; i++) {
                *(mem + body->mem_address + i) = body->data[i];
        }
}


void transmit_ACK(header_t *header) {
	header->type = ACK;
	transmit_header(&huart1, header);
}

///
/// Choose appropriate state depending on header type
///
/// If there is an error, return to Idle_State
SystemState header_handler(header_t *header) {

	switch(header->type){
	case ACK:
		// ACKs can only be sent up the chain
		return Idle_State;
		break;

	case PING:
		header->ttl += 1;

		if (header->bid_high == 0 && header->bid_medium == 0 && header->bid_low == 0) {

	    	// Send ACK up the chain
			header->bid_high = get_bid_high();
			header->bid_medium = get_bid_medium();
			header->bid_low = get_bid_low();
	    	transmit_ACK(header);

	    	HAL_Delay(800);

	    	header->type = PING;
	    	header->bid_high = 0;
	    	header->bid_medium = 0;
	    	header->bid_low = 0;

	    	// Transport PING packet down the chain
	    	transmit_header(&huart3, header);

		} else if(header->bid_high == get_bid_high() && header->bid_medium == get_bid_medium() && header->bid_low == get_bid_low() ) {
//			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET);
	    	transmit_ACK(header);
	    } else {
	    	// Transport PING packet
	    	transmit_header(&huart3, header);
	    }

		num_bytes_up = 0;
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&up_buffer, 15);

		num_bytes_down = 0;
		HAL_UART_Receive_IT(&huart3, (uint8_t *)&down_buffer, 15);

		return Idle_State;

		break;

	case READ:
		// Wait to receive body
		write_mem_en = 0;
		num_bytes_up = 0;
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&up_buffer, MAX_BUFFER_SIZE);

		// Reply to packet
		if(header->bid_high == get_bid_high() && header->bid_medium == get_bid_medium() && header->bid_low == get_bid_low() ) {
			transmit_ACK(header);
			HAL_Delay(800);

			waiting_read_down = 0;
			return Read_Region_State;
		} else {
			// Send packet down to next board in the chain
			transmit_header(&huart3, header);

			// Wait for ACK of the board
			num_bytes_down = 0;
			HAL_UART_Receive_IT(&huart3, (uint8_t *)&down_buffer, 15);

			waiting_read_down = 1;
			return Idle_State;
		}
		break;

	case WRITE:
		// Wait to receive body
		num_bytes_up = 0;
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&up_buffer, MAX_BUFFER_SIZE);
		write_mem_en = 1;

		// Reply to packet
		if(header->bid_high == get_bid_high() && header->bid_medium == get_bid_medium() && header->bid_low == get_bid_low() ) {
			transmit_ACK(header);
			HAL_Delay(800);

			waiting_read_down = 0;
			return Read_Region_State;
		} else {
			// Send packet down to next board in the chain
			transmit_header(&huart3, header);

			// Wait for ACK of the board
			num_bytes_down = 0;
			HAL_UART_Receive_IT(&huart3, (uint8_t *)&down_buffer, 15);

			waiting_read_down = 1;
			return Idle_State;
		}
		break;

	case EXEC:
		return Idle_State;
		break;
	default:
		return Idle_State;
		break;
	}
}

SystemState body_handler(body_t *body)
{
	switch(body->type) {
	case MEMORY:

		if(write_mem_en == 1) {
			write_mem_values(body);
		} else {
			transmit_body(&huart1, body);
		}

		// Prepare to receive headers
		num_bytes_down = 0;
		HAL_UART_Receive_IT(&huart3, (uint8_t *)&down_buffer, 15);

		num_bytes_up = 0;
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&up_buffer, 15);

		return Idle_State;

		break;
	case SENSORS:
	case CODE:
		return Idle_State;
		break;
	default:
		return Idle_State;
	}
}

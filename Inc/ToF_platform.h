
#ifndef INC_TOF_PLATFORM_H_
#define INC_TOF_PLATFORM_H_

//#include "vl53l5cx_api.h"
#include "stm32h7xx.h"
#include "stdint.h"
#include "string.h"

#define 	VL53L5CX_NB_TARGET_PER_ZONE		1U

typedef struct
{
	int16_t distance_matrix[64];
	int16_t distance_vector[8];
	int16_t point_x[8];

} Point_buffer;


typedef struct
{
	float rotation_matrix_0_0;
	float rotation_matrix_0_1;
	float rotation_matrix_1_0;
	float rotation_matrix_1_1;
	int16_t translate_vector_0;
	int16_t translate_vector_1;

} Linear_transform;

typedef struct
{

    uint16_t  			          address;
    uint8_t                       device_num;
    I2C_HandleTypeDef             i2c_handler;
    uint8_t                       initialized;
    Linear_transform              linear_transform;
    Point_buffer                  Point_buffer;

} VL53L5CX_Platform;


uint8_t RdByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_value);


uint8_t WrByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t value);


uint8_t RdMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size);


uint8_t WrMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size);


uint8_t Reset_Sensor(
		VL53L5CX_Platform *p_platform);


void SwapBuffer(
		uint8_t 		*buffer,
		uint16_t 	 	 size);


uint8_t WaitMs(
		VL53L5CX_Platform *p_platform,
		uint32_t TimeMs);

#endif /* INC_TOF_PLATFORM_H_ */


#ifndef _TOF_CONFIG_H_
#define _TOF_CONFIG_H_
#pragma once
//
#include "vl53l5cx_api.h"

int ToF_init
(
		VL53L5CX_Configuration *,
		I2C_HandleTypeDef,
		uint8_t,
		uint8_t,
		uint8_t,
		float,
		int16_t,
		int16_t
);

int matrix_2_vector
(
		VL53L5CX_Configuration *,
		VL53L5CX_ResultsData *,
		uint8_t
);

int dist_2_point
(
		VL53L5CX_Configuration *,
		uint8_t
);

int linear_transform
(
		VL53L5CX_Configuration *
);

int linear_transform_mapping
(
		VL53L5CX_Configuration *,
		float,
		int16_t,
		int16_t,
		uint8_t
);

int16_t calculate_slope
(
		VL53L5CX_Configuration *,
		VL53L5CX_Configuration *
);


int ToF_print_matrix_data
(
		VL53L5CX_Configuration *,
		uint8_t
);

int ToF_print_point_data
(
		VL53L5CX_Configuration *,
		uint8_t
);

int ToF_print_targetnum_data
(
		VL53L5CX_ResultsData *,
		uint8_t
);

int ToF_print_status_data
(
		VL53L5CX_ResultsData *,
		uint8_t
);
/*
int ToF_exec
(
		VL53L5CX_Configuration *,
		VL53L5CX_Configuration *,
		VL53L5CX_Configuration *,
		VL53L5CX_Configuration *,
		uint16_t,
		uint8_t,
		uint8_t,
		uint8_t
);
*/
int ToF_sensor_check
(
		VL53L5CX_Configuration *,
		uint16_t
);

#endif	// _PLATFORM_H_

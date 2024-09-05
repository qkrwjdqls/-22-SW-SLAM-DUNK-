#include "vl53l5cx_api.h"
#include "ToF.h"
#include "string.h"
#include "stdio.h"
#include "math.h"

extern UART_HandleTypeDef huart2;

//------------------------------------------------------------------------------------------------------------------------

int ToF_init (
		VL53L5CX_Configuration *Dev,
		I2C_HandleTypeDef i2c_handler,
		uint8_t address,
		uint8_t resolution,
		uint8_t frequency,
		float degree,
		int16_t x_to_set_point,
		int16_t y_to_set_point
		)
{

	uint8_t 				status, isAlive;
	uint8_t                 change_address = 0;

	char buffer[200];

	Dev->platform.i2c_handler = i2c_handler;
	Dev->platform.address = address;

	status = vl53l5cx_is_alive(Dev, &isAlive);
	if(!isAlive || status)
	{
		sprintf(buffer, "ToF %u not detected at requested address, try to access in default address\n", Dev->platform.device_num);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

		Dev->platform.address = 0x52; //default address
		status = vl53l5cx_is_alive(Dev, &isAlive);
		if(!isAlive || status)
		{
			sprintf(buffer, "ToF %u not detected at default address, need to reset sensor\n", Dev->platform.device_num);
		    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		    return status;
		}
		change_address = 1;
	}

	status = vl53l5cx_init(Dev);
	if(status)
	{
		sprintf(buffer, "ToF %u ULD Loading failed\n", Dev->platform.device_num);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		return status;
	}

	if (change_address)
	{
		status = vl53l5cx_set_i2c_address(Dev, address);
		if(status)
		{
			sprintf(buffer, "ToF %u I2C address set failed\n", Dev->platform.device_num);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
			return status;
		}
		sprintf(buffer, "ToF %u I2C address changed, 0x52 -> 0x%x\n", Dev->platform.device_num, Dev->platform.address);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	}

	Dev->platform.initialized = 1;
	sprintf(buffer, "ToF %u ready ! (I2C address : 0x%x)\n", Dev->platform.device_num, Dev->platform.address);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	status = vl53l5cx_set_resolution(Dev, resolution);
	if(status)
	{
		sprintf(buffer, "Sensor %u set resolution failed\n", Dev->platform.device_num);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		return status;
	}

	status = vl53l5cx_set_ranging_frequency_hz(Dev, frequency);
	if(status)
	{
		sprintf(buffer, "ToF %u set ranging frequency failed\n", Dev->platform.device_num);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		return status;
	}

	sprintf(buffer, "ToF %u (resolution : %u) (ranging frequency : %u)\n", Dev->platform.device_num, resolution, frequency);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	/* counterclockwise
	   0_0 0_1
	   1_0 1_1 */

	Dev->platform.linear_transform.rotation_matrix_0_0 = cos(degree * M_PI / 180);
	Dev->platform.linear_transform.rotation_matrix_0_1 = -sin(degree * M_PI / 180);
	Dev->platform.linear_transform.rotation_matrix_1_0 = sin(degree * M_PI / 180);
	Dev->platform.linear_transform.rotation_matrix_1_1 = cos(degree * M_PI / 180);
	Dev->platform.linear_transform.translate_vector_0 = x_to_set_point;
	Dev->platform.linear_transform.translate_vector_1 = y_to_set_point;

	int8_t rounded_degree = round(degree);

	sprintf(buffer, "ToF %u transform parameter set, (x point: %i, y point: %i, degree: %i)\n\n",
			Dev->platform.device_num, x_to_set_point, y_to_set_point, rounded_degree);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	return status;
}

//------------------------------------------------------------------------------------------------------------------------

int matrix_2_vector (VL53L5CX_Configuration *Dev, VL53L5CX_ResultsData *Results, uint8_t accuracy) {

	uint8_t i, j, count;
	uint16_t sum;

	if (accuracy == 0) {
		for (i = 0; i < 8; i++) {
			Dev->platform.Point_buffer.distance_vector[i]
			    = (Dev->platform.Point_buffer.distance_matrix[i + 24] + Dev->platform.Point_buffer.distance_matrix[i + 32]) / 2;
		}
	} else if (accuracy == 1) {
		for (i = 0; i < 8; i++) {
			count = 0;
			sum = 0;
			for (j = 0; j < 4; j++) {
				if (Results->nb_target_detected[i + 8 * j + 16] == 1 && Dev->platform.Point_buffer.distance_matrix[i + 8 * j + 16] > 0) {
					count++;
					sum += Dev->platform.Point_buffer.distance_matrix[i + 8 * j + 16];
				}
			}
			if (count == 0) {Dev->platform.Point_buffer.distance_vector[i] = 0;}
			else Dev->platform.Point_buffer.distance_vector[i] = sum / count;
		}
	} else if (accuracy == 2) {
		for (i = 0; i < 8; i++) {
			count = 0;
			sum = 0;
			for (j = 0; j < 4; j++) {
				if (Results->nb_target_detected[i + 8 * j + 16] == 1
				        && (Results->target_status[i + 8 * j + 16] == 5
								|| Results->target_status[i + 8 * j + 16] == 6
								        || Results->target_status[i + 8 * j + 16] == 9)
								                && Dev->platform.Point_buffer.distance_matrix[i + 8 * j + 16] > 0) {
					count++;
					sum += Dev->platform.Point_buffer.distance_matrix[i + 8 * j + 16];
				}
			}
			if (count == 0) {Dev->platform.Point_buffer.distance_vector[i] = 0;}
			else Dev->platform.Point_buffer.distance_vector[i] = sum / count;
		}
	} else if (accuracy == 3) {
		for (i = 0; i < 8; i++) {
			count = 0;
			sum = 0;
			for (j = 0; j < 4; j++) {
				if (Results->nb_target_detected[i + 8 * j + 16] == 1
						&& Results->target_status[i + 8 * j + 16] == 5
						        && Dev->platform.Point_buffer.distance_matrix[i + 8 * j + 16] > 0) {
					count++;
					sum += Dev->platform.Point_buffer.distance_matrix[i + 8 * j + 16];
				}
			}
			if (count == 0) {Dev->platform.Point_buffer.distance_vector[i] = 0;}
			else Dev->platform.Point_buffer.distance_vector[i] = sum / count;
		}
	}

	return 0;
}

int dist_2_point (VL53L5CX_Configuration *Dev, uint8_t degree) {

	float tan_degree_2 = tan(degree * M_PI / 360);
	uint8_t i;
	int8_t compute[8] = {-7, -5, -3, -1, 1, 3, 5 , 7};

	for (i = 0; i < 8; i++) {
		Dev->platform.Point_buffer.point_x[i] =
		   compute[i] * Dev->platform.Point_buffer.distance_vector[i] * tan_degree_2 / 8;
	}

	return 0;
}

int linear_transform (VL53L5CX_Configuration *Dev) {

    uint8_t i;
    int16_t point_x_buffer[8], point_y_buffer[8];

    for (i = 0; i < 8; i++) {

    	point_x_buffer[i] = Dev->platform.Point_buffer.point_x[i];
    	point_y_buffer[i] = Dev->platform.Point_buffer.distance_vector[i];

    	if (Dev->platform.Point_buffer.distance_vector[i] == 0 && Dev->platform.Point_buffer.point_x[i] == 0) {
    		Dev->platform.Point_buffer.point_x[i] = 0;
    		Dev->platform.Point_buffer.distance_vector[i] = 0;
    	} else {
            Dev->platform.Point_buffer.point_x[i] =
            		(Dev->platform.linear_transform.rotation_matrix_0_0) * point_x_buffer[i]
        			+ (Dev->platform.linear_transform.rotation_matrix_0_1) * point_y_buffer[i]
        			+ Dev->platform.linear_transform.translate_vector_0;

            Dev->platform.Point_buffer.distance_vector[i] =
                		(Dev->platform.linear_transform.rotation_matrix_1_0) * point_x_buffer[i]
            			+ (Dev->platform.linear_transform.rotation_matrix_1_1) * point_y_buffer[i]
            			+ Dev->platform.linear_transform.translate_vector_1;
    	}
    }

	return 0;
}

int linear_transform_mapping (VL53L5CX_Configuration *Dev, float degree, int16_t x_to_set_point, int16_t y_to_set_point, uint8_t endline) {

	float rotation_matrix_0_0 = cos(degree * M_PI / 180);
	float rotation_matrix_0_1 = -sin(degree * M_PI / 180);
	float rotation_matrix_1_0 = sin(degree * M_PI / 180);
	float rotation_matrix_1_1 = cos(degree * M_PI / 180);
	int16_t translate_vector_0 = x_to_set_point;
	int16_t translate_vector_1 = y_to_set_point;
	uint8_t i;
	int16_t point_x_buffer[8];
	int16_t point_y_buffer[8];

	char buffer[200];

    for (i = 0; i < 8; i++) {

    	if (Dev->platform.Point_buffer.distance_vector[i] == 0 && Dev->platform.Point_buffer.point_x[i] == 0) {
    		point_x_buffer[i] = 0;
    		point_y_buffer[i] = 0;
    	} else {
            point_x_buffer[i] =
            		rotation_matrix_0_0 * Dev->platform.Point_buffer.point_x[i]
        			+ rotation_matrix_0_1 * Dev->platform.Point_buffer.distance_vector[i]
        			+ translate_vector_0;

            point_y_buffer[i] =
                		rotation_matrix_1_0 * Dev->platform.Point_buffer.point_x[i]
            			+ rotation_matrix_1_1 * Dev->platform.Point_buffer.distance_vector[i]
            			+ translate_vector_1;
    	}
    }
	if (endline) {

		for (i = 0; i < 8; i++) {
	        sprintf(buffer, "%d,", point_x_buffer[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		for (i = 0; i < 7; i++) {
	        sprintf(buffer, "%d,", point_y_buffer[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		sprintf(buffer, "%d\n", point_y_buffer[7]);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	}
	else {

		for (i = 0; i < 8; i++) {
	        sprintf(buffer, "%d,", point_x_buffer[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		for (i = 0; i < 8; i++) {
	        sprintf(buffer, "%d,", point_y_buffer[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }

	}


	return 0;
}

int16_t calculate_slope (VL53L5CX_Configuration *Dev_1, VL53L5CX_Configuration *Dev_2) {

	int16_t point_x[16];
	int16_t point_y[16];
	int16_t sum_x = 0;
	int16_t sum_y = 0;
	int16_t sum_xy = 0;
	int16_t sum_x2 = 0;
	uint8_t i;
	//double slope;

	for (i = 0; i < 8; i++) {
		point_x[i] = Dev_1->platform.Point_buffer.point_x[i];
		point_x[i + 8] = Dev_2->platform.Point_buffer.point_x[i];
		point_y[i] = Dev_1->platform.Point_buffer.distance_vector[i];
		point_y[i + 8] = Dev_2->platform.Point_buffer.distance_vector[i];
	}

	for (i = 0; i < 16; i++) {
		sum_x += point_x[i];
		sum_y += point_y[i];
		sum_xy += point_x[i] * point_y[i];
		sum_x2 += point_x[i] * point_x[i];
	}

	//int16_t a = 16 * sum_xy - sum_x * sum_y;
	//int16_t b = 16 * sum_x2 - sum_x * sum_x;
    float slope = (16 * sum_xy - sum_x * sum_y) / (16 * sum_x2 - sum_x * sum_x);
	float atan_slope = atan(slope);
	int16_t slope_degree = (int16_t)(atan_slope * 180 / M_PI);

	//float slope = (16 * sum_xy - sum_x * sum_y) / (16 * sum_x2 - sum_x * sum_x);
	//float atan_slope = atan(slope);
	//int16_t slope_degree = (int16_t)round(atan_slope * 180 / M_PI);

	return slope_degree;
}

//------------------------------------------------------------------------------------------------------------------------

int ToF_print_matrix_data (VL53L5CX_Configuration *Dev, uint8_t endline) {

	char buffer[200];
	uint8_t i = 0;

	if (endline) {

		for (i = 0; i < 63; i++) {
	        sprintf(buffer, "%d,", Dev->platform.Point_buffer.distance_matrix[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		sprintf(buffer, "%d\n", Dev->platform.Point_buffer.distance_matrix[63]);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	}
	else {

		for (i = 0; i < 64; i++) {
			        sprintf(buffer, "%d,", Dev->platform.Point_buffer.distance_matrix[i]);
			        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
			    }
	}

	return 0;
}

int ToF_print_point_data (VL53L5CX_Configuration *Dev, uint8_t endline) {

	char buffer[200];
	uint8_t i = 0;

	if (endline) {

		for (i = 0; i < 8; i++) {
	        sprintf(buffer, "%d,", Dev->platform.Point_buffer.point_x[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		for (i = 0; i < 7; i++) {
	        sprintf(buffer, "%d,", Dev->platform.Point_buffer.distance_vector[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		sprintf(buffer, "%d\n", Dev->platform.Point_buffer.distance_vector[7]);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	}
	else {

		for (i = 0; i < 8; i++) {
	        sprintf(buffer, "%d,", Dev->platform.Point_buffer.point_x[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		for (i = 0; i < 8; i++) {
	        sprintf(buffer, "%d,", Dev->platform.Point_buffer.distance_vector[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }

	}

	return 0;
}

int ToF_print_targetnum_data (VL53L5CX_ResultsData *Results, uint8_t endline) {

	char buffer[200];
	uint8_t i = 0;

	if (endline) {

		for (i = 0; i < 63; i++) {
	        sprintf(buffer, "%d,", Results->nb_target_detected[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		sprintf(buffer, "%d\n", Results->nb_target_detected[63]);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	}
	else {

		for (i = 0; i < 64; i++) {
			        sprintf(buffer, "%d,", Results->nb_target_detected[i]);
			        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
			    }
	}

	return 0;
}

int ToF_print_status_data (VL53L5CX_ResultsData *Results, uint8_t endline) {

	char buffer[200];
	uint8_t i = 0;

	if (endline) {

		for (i = 0; i < 63; i++) {
	        sprintf(buffer, "%d,", Results->target_status[i]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	    }
		sprintf(buffer, "%d\n", Results->target_status[63]);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	}
	else {

		for (i = 0; i < 64; i++) {
			        sprintf(buffer, "%d,", Results->target_status[i]);
			        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
			    }
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------

int ToF_sensor_check
(
		VL53L5CX_Configuration *Dev,
		uint16_t measure_time
)
{
	uint8_t 				isReady, i, loop, status;
	VL53L5CX_ResultsData 	Results;

	char buffer[200];


	status = vl53l5cx_start_ranging(Dev);

	loop = 0;

	while(loop < measure_time)
	{
		status = vl53l5cx_check_data_ready(Dev, &isReady);

		if(isReady)
		{
			status = vl53l5cx_get_ranging_data(Dev, &Results);
			for(i = 0; i < 64; i++)
			{
				if (Results.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE * i] % 10 >= 5) {
					Dev->platform.Point_buffer.distance_matrix[i]
															   = (Results.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE * i] / 10) + 1;
				}      else {
					Dev->platform.Point_buffer.distance_matrix[i]
															   = Results.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE * i] / 10;
				}
			}
			status = ToF_print_matrix_data(Dev, 1);
			loop++;
		}
		HAL_Delay(5);
	}
	status = vl53l5cx_stop_ranging(Dev);

	sprintf(buffer, "Stop ranging\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	return status;
}

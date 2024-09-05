/*
 * mpu6050.h
 *
 *  Created on: Nov 13, 2019
 *      Author: Bulanov Konstantin
 */

#ifndef INC_GY521_H_
#define INC_GY521_H_

#endif /* INC_GY521_H_ */

#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

#include "stdint.h"
#include "math.h"

// MPU6050 structure
typedef struct
{

    long Cal_AcX, Cal_AcY, Cal_AcZ, Cal_GyX, Cal_GyY, Cal_GyZ;
    int Cal_Count;
    long Offset_AcX, Offset_AcY, Offset_AcZ, Offset_GyX, Offset_GyY, Offset_GyZ;
    int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
    float axg, ayg, azg, gxrs, gyrs, gzrs;
    float norm_s;
    float halfvx_s, halfvy_s, halfvz_s;
    float halfex_s, halfey_s, halfez_s;
    float qa_s, qb_s, qc_s;
    float sampleFreq;
    uint32_t lastUpdate, firstUpdate, Now;
    float twoKp_s, twoKi_s;
    float q0_s, q1_s, q2_s, q3_s;
    float integralFBx_s, integralFBy_s, integralFBz_s;
    float roll, pitch, yaw;

} MPU6050_t;

void MPU6050_Init(I2C_HandleTypeDef *I2Cx);

void MPU6050_Calibration(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_re_Calibration(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

//void MPU6050_fast_Calibration(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_GetData(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_updateQuaternion(MPU6050_t *DataStruct);

void MPU6050_SamplingFrequency(MPU6050_t *DataStruct);

void MPU6050_SamplingFrequency_Initialize(MPU6050_t *DataStruct);

void MahonyAHRSupdateIMU(MPU6050_t *DataStruct);

void MPU6050_getRollPitchYaw(MPU6050_t *DataStruct);

void MPU6050_RUN(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_initial_RUN(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_Start(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_set(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

int MPU6050_y(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

void MPU6050_DataReset(MPU6050_t *DataStruct);

//void MPU6050SelfTest(I2C_HandleTypeDef *I2Cx, float * destination);

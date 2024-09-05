#include "mpu6050.h"

#define RAD_TO_DEG 57.295779513082320876798154814105

#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define SMPLRT_DIV_REG 0x19
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_CONFIG_REG 0x1B
#define GYRO_XOUT_H_REG 0x43

#define CONFIG_REG 0x1A
#define ACCEL_CONFIG_REG 0x1C
#define GYRO_CONFIGE_REG 0x1B
#define SELF_TEST_X_REG 0x0D
#define SELF_TEST_Y_REG 0x0E
#define SELF_TEST_Z_REG 0x0F
#define SELF_TEST_A_REG 0x10

// Transform raw data of accelerometer & gyroscope
//#define MPU6050_AXOFFSET 395
//#define MPU6050_AYOFFSET -135
//#define MPU6050_AZOFFSET 345
//#define MPU6050_AXOFFSET 0
//#define MPU6050_AYOFFSET 0
//#define MPU6050_AZOFFSET 0
#define MPU6050_AXGAIN 16384.0 // AFS_SEL = 0, +/-2g, MPU6050_ACCEL_FS_2
#define MPU6050_AYGAIN 16384.0 // AFS_SEL = 0, +/-2g, MPU6050_ACCEL_FS_2
#define MPU6050_AZGAIN 16384.0 // AFS_SEL = 0, +/-2g, MPU6050_ACCEL_FS_2
//#define MPU6050_AXGAIN 8192.0 // AFS_SEL = 1, +/-4g, MPU6050_ACCEL_FS_4
//#define MPU6050_AYGAIN 8192.0 // AFS_SEL = 1, +/-4g, MPU6050_ACCEL_FS_4
//#define MPU6050_AZGAIN 8192.0 // AFS_SEL = 1, +/-4g, MPU6050_ACCEL_FS_4
//#define MPU6050_AXGAIN 4096.0 // AFS_SEL = 2, +/-8g, MPU6050_ACCEL_FS_8
//#define MPU6050_AYGAIN 4096.0 // AFS_SEL = 2, +/-8g, MPU6050_ACCEL_FS_8
//#define MPU6050_AZGAIN 4096.0 // AFS_SEL = 2, +/-8g, MPU6050_ACCEL_FS_8
//#define MPU6050_AXGAIN 2048.0 // AFS_SEL = 3, +/-16g, MPU6050_ACCEL_FS_16
//#define MPU6050_AYGAIN 2048.0 // AFS_SEL = 3, +/-16g, MPU6050_ACCEL_FS_16
//#define MPU6050_AZGAIN 2048.0 // AFS_SEL = 3, +/-16g, MPU6050_ACCEL_FS_16
//#define MPU6050_GXOFFSET -9
//#define MPU6050_GYOFFSET -19
//#define MPU6050_GZOFFSET -20
//#define MPU6050_GXOFFSET 0
//#define MPU6050_GYOFFSET 0
//#define MPU6050_GZOFFSET 0
#define MPU6050_GXGAIN 131.072 // FS_SEL = 0, +/-250degree/s, MPU6050_GYRO_FS_250
#define MPU6050_GYGAIN 131.072 // FS_SEL = 0, +/-250degree/s, MPU6050_GYRO_FS_250
#define MPU6050_GZGAIN 131.072 // FS_SEL = 0, +/-250degree/s, MPU6050_GYRO_FS_250
//#define MPU6050_GXGAIN 65.536 // FS_SEL = 1, +/-500degree/s, MPU6050_GYRO_FS_500
//#define MPU6050_GYGAIN 65.536 // FS_SEL = 1, +/-500degree/s, MPU6050_GYRO_FS_500
//#define MPU6050_GZGAIN 65.536 // FS_SEL = 1, +/-500degree/s, MPU6050_GYRO_FS_500
//#define MPU6050_GXGAIN 32.768 // FS_SEL = 2, +/-1000degree/s, MPU6050_GYRO_FS_1000
//#define MPU6050_GYGAIN 32.768 // FS_SEL = 2, +/-1000degree/s, MPU6050_GYRO_FS_1000
//#define MPU6050_GZGAIN 32.768 // FS_SEL = 2, +/-1000degree/s, MPU6050_GYRO_FS_1000
//#define MPU6050_GXGAIN 16.384 // FS_SEL = 3, +/-2000degree/s, MPU6050_GYRO_FS_2000
//#define MPU6050_GYGAIN 16.384 // FS_SEL = 3, +/-2000degree/s, MPU6050_GYRO_FS_2000
//#define MPU6050_GZGAIN 16.384 // FS_SEL = 3, +/-2000degree/s, MPU6050_GYRO_FS_2000

// Degree to Radians, Pi/180
#define DEG_TO_RAD 0.01745329251994329576923690768489

// For Quaternion function
#define twoKpDef  (2.0f * 0.5f) // 2 * proportional gain
#define twoKiDef  (2.0f * 0.0f) // 2 * integral gain

/*
volatile float twoKp = twoKpDef;                      // 2 * proportional gain (Kp)
volatile float twoKi = twoKiDef;                      // 2 * integral gain (Ki)
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;          // quaternion of sensor frame relative to auxiliary frame
volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f; // integral error terms scaled by Ki
 */

//#define MPU6050_ADDR 0x68

// Setup MPU6050
#define MPU6050_ADDR 0xD0
const uint16_t i2c_timeout = 10;
/*const int i2c_timeout = HAL_MAX_DELAY;*/


void MPU6050_Init(I2C_HandleTypeDef *I2Cx)
{
	uint8_t check = 0;
	uint8_t Data = 0;

	// check device ID WHO_AM_I
	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, i2c_timeout);

	if (check == 104) // 0x68 will be returned by the sensor if everything goes well
	{
		// power management register 0X6B we should write all 0's to wake the sensor up
		Data = 0;
		HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1, i2c_timeout);

		// Selection Clock 'PLL with X axis gyroscope reference'
		Data = 0x01;
		HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1, i2c_timeout);

		/*        // Set DATA RATE of 1KHz by writing SMPLRT_DIV register
        Data = 0x07;
        HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1, i2c_timeout);*/

		//Accel BW 260Hz, Delay 0ms / Gyro BW 256Hz, Delay 0.98ms, Fs 8KHz
		Data = 0x00;
		HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, CONFIG_REG, 1, &Data, 1, i2c_timeout);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// AFS_SEL = 3, +/-16g, MPU6050_ACCEL_FS_16
		Data = 0x00;
		HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1, i2c_timeout);

		// Set Gyroscopic configuration in GYRO_CONFIG Register
		// FS_SEL = 3, +/-2000degree/s, MPU6050_GYRO_FS_2000
		Data = 0x00;
		HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &Data, 1, i2c_timeout);
	}
}


void MPU6050_Calibration(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	uint8_t Rec_Data[14];
	int repeat = 100;

	/*	DataStruct->Offset_AcX = 0;
	DataStruct->Offset_AcY = 0;
	DataStruct->Offset_AcZ = 0;
	DataStruct->Offset_GyX = 0;
	DataStruct->Offset_GyY = 0;
	DataStruct->Offset_GyZ = 0;*/

	for(int i = 0; i < repeat; i++){
		HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 14, i2c_timeout);

		DataStruct->AcX = (int16_t)(Rec_Data[0] << 8 | Rec_Data[1]);
		DataStruct->AcY = (int16_t)(Rec_Data[2] << 8 | Rec_Data[3]);
		DataStruct->AcZ = (int16_t)((Rec_Data[4] << 8 | Rec_Data[5]) - MPU6050_AZGAIN);
		DataStruct->GyX = (int16_t)(Rec_Data[8] << 8 | Rec_Data[9]);
		DataStruct->GyY = (int16_t)(Rec_Data[10] << 8 | Rec_Data[11]);
		DataStruct->GyZ = (int16_t)(Rec_Data[12] << 8 | Rec_Data[13]);
		HAL_Delay(10);

		DataStruct->Cal_AcX += DataStruct->AcX;
		DataStruct->Cal_AcY += DataStruct->AcY;
		DataStruct->Cal_AcZ += DataStruct->AcZ;
		DataStruct->Cal_GyX += DataStruct->GyX;
		DataStruct->Cal_GyY += DataStruct->GyY;
		DataStruct->Cal_GyZ += DataStruct->GyZ;
	}

	DataStruct->Offset_AcX = DataStruct->Cal_AcX / repeat;
	DataStruct->Offset_AcY = DataStruct->Cal_AcY / repeat;
	DataStruct->Offset_AcZ = DataStruct->Cal_AcZ / repeat;
	DataStruct->Offset_GyX = DataStruct->Cal_GyX / repeat;
	DataStruct->Offset_GyY = DataStruct->Cal_GyY / repeat;
	DataStruct->Offset_GyZ = DataStruct->Cal_GyZ / repeat;

	DataStruct->Cal_AcX = 0;
	DataStruct->Cal_AcY = 0;
	DataStruct->Cal_AcZ = 0;
	DataStruct->Cal_GyX = 0;
	DataStruct->Cal_GyY = 0;
	DataStruct->Cal_GyZ = 0;
}

void MPU6050_re_Calibration(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	MPU6050_Calibration(I2Cx, DataStruct);
	MPU6050_initial_RUN(I2Cx, DataStruct);
	HAL_Delay(10);

	while(DataStruct->yaw > 0.5 || DataStruct->yaw < -0.5
			|| DataStruct->pitch > 1.0 || DataStruct->pitch < -1.0
			|| DataStruct->roll > 1.0 || DataStruct->roll < -1.0
			|| isnan(DataStruct->yaw) || isnan(DataStruct->pitch) || isnan(DataStruct->roll)
/*			|| DataStruct->yaw == 0.0 || DataStruct->pitch == 0.0 || DataStruct->roll == 0.0*/){
		DataStruct->Cal_Count += 1;

		if(DataStruct->Cal_Count % 5 == 0)
		{
			HAL_Delay(10);
			/*MPU6050_Init(I2Cx);*/
			MPU6050_DataReset(DataStruct);
			HAL_Delay(10);
			MPU6050_Calibration(I2Cx, DataStruct);
			HAL_Delay(10);
			MPU6050_initial_RUN(I2Cx, DataStruct);
			HAL_Delay(10);
		}
		else{
			MPU6050_Calibration(I2Cx, DataStruct);
			HAL_Delay(10);
			MPU6050_initial_RUN(I2Cx, DataStruct);
			HAL_Delay(10);
		}
	}
}

void MPU6050_GetData(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	uint8_t data_org[14];
	/*	DataStruct->AcX = 0;
	DataStruct->AcY = 0;
	DataStruct->AcZ = 0;
	DataStruct->GyX = 0;
	DataStruct->GyY = 0;
	DataStruct->GyZ = 0;*/

	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, data_org, 14, i2c_timeout);

	DataStruct->AcX = (int16_t)(data_org[0] << 8 | data_org[1]);
	DataStruct->AcY = (int16_t)(data_org[2] << 8 | data_org[3]);
	DataStruct->AcZ = (int16_t)(data_org[4] << 8 | data_org[5]);
	DataStruct->GyX = (int16_t)(data_org[8] << 8 | data_org[9]);
	DataStruct->GyY = (int16_t)(data_org[10] << 8 | data_org[11]);
	DataStruct->GyZ = (int16_t)(data_org[12] << 8 | data_org[13]);
}

void MPU6050_updateQuaternion(MPU6050_t *DataStruct)
{
	/*	DataStruct->axg = 0.0f;
	DataStruct->ayg = 0.0f;
	DataStruct->azg = 0.0f;
	DataStruct->gxrs = 0.0f;
	DataStruct->gyrs = 0.0f;
	DataStruct->gzrs = 0.0f;*/

	DataStruct->axg = (float)((DataStruct->AcX) - (DataStruct->Offset_AcX)) / MPU6050_AXGAIN;
	DataStruct->ayg = (float)((DataStruct->AcY) - (DataStruct->Offset_AcY)) / MPU6050_AYGAIN;
	DataStruct->azg = (float)((DataStruct->AcZ) - (DataStruct->Offset_AcZ)) / MPU6050_AZGAIN;
	DataStruct->gxrs = (float)((DataStruct->GyX) - (DataStruct->Offset_GyX)) / MPU6050_GXGAIN * DEG_TO_RAD;
	DataStruct->gyrs = (float)((DataStruct->GyY) - (DataStruct->Offset_GyY)) / MPU6050_GYGAIN * DEG_TO_RAD;
	DataStruct->gzrs = (float)((DataStruct->GyZ) - (DataStruct->Offset_GyZ)) / MPU6050_GZGAIN * DEG_TO_RAD;
}

void MPU6050_SamplingFrequency(MPU6050_t *DataStruct)
{
	DataStruct->Now = HAL_GetTick();
	DataStruct->sampleFreq = (float)(1000.0f / ((DataStruct->Now) - (DataStruct->lastUpdate)));
	DataStruct->lastUpdate = DataStruct->Now;
}

void MPU6050_SamplingFrequency_Initialize(MPU6050_t *DataStruct)
{
	DataStruct->sampleFreq = 0.0f;
	DataStruct->lastUpdate = 0;
	DataStruct->firstUpdate = 0;
	DataStruct->Now = 0;
}

void MahonyAHRSupdateIMU(MPU6050_t *DataStruct)
{
	float norm = DataStruct->norm_s;
	float halfvx = DataStruct->halfvx_s;
	float halfvy = DataStruct->halfvy_s;
	float halfvz = DataStruct->halfvz_s;
	float halfex = DataStruct->halfex_s;
	float halfey = DataStruct->halfey_s;
	float halfez = DataStruct->halfez_s;
	float qa = DataStruct->qa_s;
	float qb = DataStruct->qb_s;
	float qc = DataStruct->qc_s;
	float ax = DataStruct->axg;
	float ay = DataStruct->ayg;
	float az = DataStruct->azg;
	float gx = DataStruct->gxrs;
	float gy = DataStruct->gyrs;
	float gz = DataStruct->gzrs;
	float sampleFreq = DataStruct->sampleFreq;
	/*
	float twoKp = twoKpDef;                      // 2 * proportional gain (Kp)
	float twoKi = twoKiDef;                      // 2 * integral gain (Ki)
	float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;          // quaternion of sensor frame relative to auxiliary frame
	float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f; // integral error terms scaled by Ki
	 */

	float twoKp = DataStruct->twoKp_s;
	float twoKi = DataStruct->twoKi_s;
	float q0 = DataStruct->q0_s;
	float q1 = DataStruct->q1_s;
	float q2 = DataStruct->q2_s;
	float q3 = DataStruct->q3_s;
	float integralFBx = DataStruct->integralFBx_s;
	float integralFBy = DataStruct->integralFBy_s;
	float integralFBz = DataStruct->integralFBz_s;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))){
		// Normalise accelerometer measurement
		norm = sqrt(ax * ax + ay * ay + az * az);
		ax /= norm;
		ay /= norm;
		az /= norm;

		// Estimated direction of gravity and vector perpendicular to magnetic flux
		halfvx = q1 * q3 - q0 * q2;
		halfvy = q0 * q1 + q2 * q3;
		halfvz = q0 * q0 - 0.5f + q3 * q3;

		// Error is sum of cross product between estimated and measured direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			integralFBx += twoKi * halfex * (1.0f / sampleFreq);  // integral error scaled by Ki
			integralFBy += twoKi * halfey * (1.0f / sampleFreq);
			integralFBz += twoKi * halfez * (1.0f / sampleFreq);
			gx += integralFBx;  // apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		}
		else {
			integralFBx = 0.0f; // prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / sampleFreq));   // pre-multiply common factors
	gy *= (0.5f * (1.0f / sampleFreq));
	gz *= (0.5f * (1.0f / sampleFreq));

	qa = q0;
	qb = q1;
	qc = q2;

	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx);

	DataStruct->q0_s = q0;
	DataStruct->q1_s = q1;
	DataStruct->q2_s = q2;
	DataStruct->q3_s = q3;
}

void MPU6050_getRollPitchYaw(MPU6050_t *DataStruct)
{
	//  yaw = atan2(2*q1*q2 - 2*q0*q3, 2*q0*q0 + 2*q1*q1 - 1) * 57.29577951;
	//  pitch = -asin(2*q1*q3 + 2*q0*q2) * 57.29577951;
	//  roll = atan2(2*q2*q3 - 2*q0*q1, 2*q0*q0 + 2*q3*q3 - 1) * 57.29577951;
	//  roll = atan2(2*q0*q1 + 2*q2*q3, 1 - 2*q1*q1 - 2*q2*q2) * 57.29577951;
	//  pitch = asin(2*q0*q2 - 2*q3*q1) * 57.29577951;
	//  yaw = atan2(2*q0*q3 + 2*q1*q2, 1 - 2*q2*q2 - 2*q3*q3) * 57.29577951;
	float q0 = DataStruct->q0_s;
	float q1 = DataStruct->q1_s;
	float q2 = DataStruct->q2_s;
	float q3 = DataStruct->q3_s;

	DataStruct->yaw   = -atan2(2.0f * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3)* RAD_TO_DEG;
	DataStruct->pitch = asin(2.0f * (q1 * q3 - q0 * q2))* RAD_TO_DEG;
	DataStruct->roll  = atan2(2.0f * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3)* RAD_TO_DEG;

	if(isnan(DataStruct->yaw) || isnan(DataStruct->pitch) || isnan(DataStruct->roll)){
		DataStruct->yaw = 0.0f;
		DataStruct->pitch= 0.0f;
		DataStruct->roll = 0.0f;
	}
}

void MPU6050_RUN(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	MPU6050_GetData(I2Cx, DataStruct);
	MPU6050_updateQuaternion(DataStruct);
	MPU6050_SamplingFrequency(DataStruct);
	MahonyAHRSupdateIMU(DataStruct);
	MPU6050_getRollPitchYaw(DataStruct);

	if(DataStruct->yaw == 0 || DataStruct->pitch == 0 || DataStruct->roll == 0){
		MPU6050_Init(I2Cx);
		MPU6050_DataReset(DataStruct);
		MPU6050_re_Calibration(I2Cx, DataStruct);
	}

	HAL_Delay(0);
}

void MPU6050_initial_RUN(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	MPU6050_GetData(I2Cx, DataStruct);
	MPU6050_updateQuaternion(DataStruct);
	MPU6050_SamplingFrequency(DataStruct);
	MahonyAHRSupdateIMU(DataStruct);
	MPU6050_getRollPitchYaw(DataStruct);

	HAL_Delay(0);
}

void MPU6050_Start(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
//   MPU6050_Init(I2Cx);
//   MPU6050_DataReset(DataStruct);
//   MPU6050_re_Calibration(I2Cx, DataStruct);

   while(DataStruct->yaw < 90 && DataStruct->yaw > -90){
      MPU6050_RUN(I2Cx, DataStruct);
   }

//   HAL_Delay(5000);
}

void MPU6050_set(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	MPU6050_Init(I2Cx);
	MPU6050_DataReset(DataStruct);
	MPU6050_re_Calibration(I2Cx, DataStruct);
}

int MPU6050_y(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
	MPU6050_RUN(I2Cx, DataStruct);
	return (int)(DataStruct->yaw * DEG_TO_RAD);
}

void MPU6050_DataReset(MPU6050_t *DataStruct){
	DataStruct->Cal_AcX = 0;
	DataStruct->Cal_AcY = 0;
	DataStruct->Cal_AcZ = 0;
	DataStruct->Cal_GyX = 0;
	DataStruct->Cal_GyY = 0;
	DataStruct->Cal_GyZ = 0;
	DataStruct->Cal_Count = 0;
	DataStruct->Offset_AcX = 0;
	DataStruct->Offset_AcY = 0;
	DataStruct->Offset_AcZ = 0;
	DataStruct->Offset_GyX = 0;
	DataStruct->Offset_GyY = 0;
	DataStruct->Offset_GyZ = 0;
	DataStruct->AcX = 0;
	DataStruct->AcY = 0;
	DataStruct->AcZ = 0;
	DataStruct->GyX = 0;
	DataStruct->GyY = 0;
	DataStruct->GyZ = 0;
	DataStruct->norm_s = 0;
	DataStruct->halfvx_s = 0;
	DataStruct->halfvy_s = 0;
	DataStruct->halfvz_s = 0;
	DataStruct->halfex_s = 0;
	DataStruct->halfey_s = 0;
	DataStruct->halfez_s = 0;
	DataStruct->qa_s = 0;
	DataStruct->qb_s = 0;
	DataStruct->qc_s = 0;
	DataStruct->axg = 0.0;
	DataStruct->ayg = 0.0;
	DataStruct->azg = 0.0;
	DataStruct->gxrs = 0.0;
	DataStruct->gyrs = 0.0;
	DataStruct->gzrs = 0.0;
	DataStruct->twoKp_s = twoKpDef;
	DataStruct->twoKi_s = twoKiDef;
	DataStruct->q0_s = 1.0;
	DataStruct->q1_s = 0.0;
	DataStruct->q2_s = 0.0;
	DataStruct->q3_s = 0.0;
	DataStruct->integralFBx_s = 0.0;
	DataStruct->integralFBy_s = 0.0;
	DataStruct->integralFBz_s = 0.0;
	DataStruct->sampleFreq = 0.0;
	/*	DataStruct->lastUpdate = 0;
	DataStruct->firstUpdate = 0;
	DataStruct->Now = 0;*/
	DataStruct->roll = 0.0;
	DataStruct->pitch = 0.0;
	DataStruct->yaw = 0.0;
}

/*// Accelerometer and gyroscope self test; check calibration wrt factory settings
void MPU6050SelfTest(I2C_HandleTypeDef *I2Cx, float * destination) // Should return percent deviation from factory trim values, +/- 14 or less deviation is a pass
{
   uint8_t rawData[4];
   uint8_t selfTest[6];
   float factoryTrim[6];
   uint8_t w_Data;

   // Configure the accelerometer for self-test

   // Enable self test on all three axes and set accelerometer range to +/- 8 g
   w_Data = 0xF0;
   HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &w_Data, 1, i2c_timeout);

   // Enable self test on all three axes and set gyro range to +/- 250 degrees/s
   w_Data = 0xE0;
   HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, GYRO_CONFIGE_REG, 1, &w_Data, 1, i2c_timeout);

   HAL_Delay(250);

   HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, SELF_TEST_X_REG, 1, &rawData[0], 1, i2c_timeout); // X-axis self-test results
   HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, SELF_TEST_Y_REG, 1, &rawData[1], 1, i2c_timeout); // Y-axis self-test results
   HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, SELF_TEST_Z_REG, 1, &rawData[2], 1, i2c_timeout);	// Z-axis self-test results
   HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, SELF_TEST_A_REG, 1, &rawData[3], 1, i2c_timeout);	// Mixed-axis self-test results

   // Extract the acceleration test results first
   selfTest[0] = (rawData[0] >> 3) | (rawData[3] & 0x30) >> 4 ; // XA_TEST result is a five-bit unsigned integer
   selfTest[1] = (rawData[1] >> 3) | (rawData[3] & 0x0C) >> 2 ; // YA_TEST result is a five-bit unsigned integer
   selfTest[2] = (rawData[2] >> 3) | (rawData[3] & 0x03) ; // ZA_TEST result is a five-bit unsigned integer
   // Extract the gyration test results first
   selfTest[3] = rawData[0]  & 0x1F ; // XG_TEST result is a five-bit unsigned integer
   selfTest[4] = rawData[1]  & 0x1F ; // YG_TEST result is a five-bit unsigned integer
   selfTest[5] = rawData[2]  & 0x1F ; // ZG_TEST result is a five-bit unsigned integer
   // Process results to allow final comparison with factory set values
   factoryTrim[0] = (4096.0*0.34)*(pow( (0.92/0.34) , (((float)selfTest[0] - 1.0)/30.0))); // FT[Xa] factory trim calculation
   factoryTrim[1] = (4096.0*0.34)*(pow( (0.92/0.34) , (((float)selfTest[1] - 1.0)/30.0))); // FT[Ya] factory trim calculation
   factoryTrim[2] = (4096.0*0.34)*(pow( (0.92/0.34) , (((float)selfTest[2] - 1.0)/30.0))); // FT[Za] factory trim calculation
   factoryTrim[3] =  ( 25.0*131.0)*(pow( 1.046 , ((float)selfTest[3] - 1.0) ));             // FT[Xg] factory trim calculation
   factoryTrim[4] =  (-25.0*131.0)*(pow( 1.046 , ((float)selfTest[4] - 1.0) ));             // FT[Yg] factory trim calculation
   factoryTrim[5] =  ( 25.0*131.0)*(pow( 1.046 , ((float)selfTest[5] - 1.0) ));             // FT[Zg] factory trim calculation

 //  Output self-test results and factory trim calculation if desired
 //  Serial.println(selfTest[0]); Serial.println(selfTest[1]); Serial.println(selfTest[2]);
 //  Serial.println(selfTest[3]); Serial.println(selfTest[4]); Serial.println(selfTest[5]);
 //  Serial.println(factoryTrim[0]); Serial.println(factoryTrim[1]); Serial.println(factoryTrim[2]);
 //  Serial.println(factoryTrim[3]); Serial.println(factoryTrim[4]); Serial.println(factoryTrim[5]);

 // Report results as a ratio of (STR - FT)/FT; the change from Factory Trim of the Self-Test Response
 // To get to percent, must multiply by 100 and subtract result from 100
   for (int i = 0; i < 6; i++) {
     destination[i] = 100.0 + 100.0*((float)selfTest[i] - factoryTrim[i])/factoryTrim[i]; // Report percent differences
   }
}*/

/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "eth.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
#include "vl53l5cx_api.h"
#include "ToF_platform.h"
#include "ToF.h"
#include "mpu6050.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VL53L5CX_SENSOR_DEFAULT_I2C_ADDRESS   ((uint16_t)0x52)
#define VL53L5CX_SENSOR_0_I2C_ADDRESS         ((uint16_t)0x52)
#define VL53L5CX_SENSOR_1_I2C_ADDRESS         ((uint16_t)0x54)
#define VL53L5CX_SENSOR_2_I2C_ADDRESS         ((uint16_t)0x56)
#define VL53L5CX_SENSOR_3_I2C_ADDRESS         ((uint16_t)0x58)
#define VL53L5CX_SENSOR_4_I2C_ADDRESS         ((uint16_t)0x60)

#define VL53L5CX_RESOLUTION_4X4               ((uint8_t) 16U)
#define VL53L5CX_RESOLUTION_8X8               ((uint8_t) 64U)

#define VL53L5CX_FREQUENCY_1HZ                ((uint8_t) 1U)
#define VL53L5CX_FREQUENCY_2HZ                ((uint8_t) 2U)
#define VL53L5CX_FREQUENCY_3HZ                ((uint8_t) 3U)
#define VL53L5CX_FREQUENCY_4HZ                ((uint8_t) 4U)
#define VL53L5CX_FREQUENCY_8HZ                ((uint8_t) 8U)

#define VL53L5CX_PRINT_MODE_DEFAULT           ((uint8_t) 0U)
#define VL53L5CX_PRINT_MODE_VAILD_TARGET      ((uint8_t) 1U)
#define VL53L5CX_PRINT_MODE_STATUS            ((uint8_t) 2U)
#define VL53L5CX_PRINT_MODE_SLOPE             ((uint8_t) 3U)

#define VL53L5CX_ACCURACY_LEVEL_0             ((uint8_t) 0U)
#define VL53L5CX_ACCURACY_LEVEL_1             ((uint8_t) 1U)
#define VL53L5CX_ACCURACY_LEVEL_2             ((uint8_t) 2U)
#define VL53L5CX_ACCURACY_LEVEL_3             ((uint8_t) 3U)

#define VL53L5CX_MAX_DISTANCE                 ((uint8_t) 200U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char buffer[200];
VL53L5CX_Configuration Dev[5];
int ToF_status;
uint8_t dmode=0;
volatile uint32_t captured_value1 = 0;
volatile uint32_t captured_value2 = 0;
volatile uint32_t captured_value3 = 0;
volatile uint32_t captured_value4 = 0;
#define MAX_COORDINATES 1000
MPU6050_t MPU6050;
int32_t dis_left;
int32_t dis_right;
int32_t dis_front=60;
uint32_t move; //moving distance
uint32_t pre_move;
int angle=0;
int32_t x_coor=0;
int32_t y_coor=0;
uint32_t cnt=0;
int32_t xx;
int32_t yy;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int ToF_multi_bootloading(void);


int ToF_exec
(
      VL53L5CX_Configuration *,
      VL53L5CX_Configuration *,
      VL53L5CX_Configuration *,
      VL53L5CX_Configuration *,
      VL53L5CX_Configuration *,
      uint16_t,
      uint8_t,
      uint8_t,
      uint8_t
);
void Drive();
void Rotate_Right();
void Rotate_Left();
void Brake();
typedef struct {
   int x;
   int y;
} Point;
int mycos(int);
int mysin(int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int mycos(int angle) {
	if ((angle + 12) % 4 == 0) return 1;
	else if ((angle + 12) % 4 == 1) return 0;
	else if ((angle + 12) % 4 == 2) return -1;
	else if ((angle + 12) % 4 == 3) return 0;
}

int mysin(int angle) {
	if ((angle + 12) % 4 == 0) return 0;
	else if ((angle + 12) % 4 == 1) return 1;
	else if ((angle + 12) % 4 == 2) return 0;
	else if ((angle + 12) % 4 == 3) return -1;
}


void saveCoordinate(Point array[], int* num_coordinates, int x, int y, int angle) {
	for (int i = 1; i < 5; ++i) {
		// Loop over points (j from 0 to 7)
		for (int k = 0; k < 8; ++k) {
			// Access the specific point's x and y coordinates

			xx=Dev[i].platform.Point_buffer.point_x[k];
			yy=Dev[i].platform.Point_buffer.distance_vector[k];

			array[*num_coordinates].x = x+xx*mycos(angle)-yy*mysin(angle);
			array[*num_coordinates].y = y+xx*mysin(angle)+yy*mycos(angle);
			(*num_coordinates)++;

		}
	}


	for (int i = 0; i < 32;i++) {
		sprintf(buffer, "%d,", array[i].x);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	}
	for (int i = 0; i < 31;i++) {
		sprintf(buffer, "%d,", array[i].y);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
	}
	sprintf(buffer, "%d\n", array[31].y);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

	*num_coordinates=0;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_HS_USB_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_I2C5_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
   HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
   HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
   HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
   HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);

   //Drive();


   sprintf(buffer, "Board Initialization Success\n\n");
   HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

   ToF_status = ToF_multi_bootloading();

   if (ToF_status)
   {
      sprintf(buffer, "ToF Bootstrapping process failed, 2nd try..\n\n");
      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

      ToF_status = ToF_multi_bootloading();
   }

   if (ToF_status)
   {
      sprintf(buffer, "ToF Bootstrapping process failed, last try...\n\n");
      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

      ToF_status = ToF_multi_bootloading();
   }

   if (ToF_status)
   {
      sprintf(buffer, "ToF Bootstrapping process failed, may need hardware reset\n\n");
      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

      return 0;
   }

   ToF_status = ToF_exec (
         &Dev[0],
         &Dev[1],
         &Dev[2],
         &Dev[3],
         &Dev[4],
         6000,
         0,
         VL53L5CX_ACCURACY_LEVEL_3,
         VL53L5CX_MAX_DISTANCE
   );
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1)
   {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
   }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int ToF_multi_bootloading(void) {

   uint8_t status = 0;
   uint8_t i = 0;

   Dev[0].platform.device_num = 0;
   Dev[1].platform.device_num = 1;
   Dev[2].platform.device_num = 2;
   Dev[3].platform.device_num = 3;
   Dev[4].platform.device_num = 4;

   uint16_t LPn_buffer[5] = {LPn_0_Pin, LPn_1_Pin, LPn_2_Pin, LPn_3_Pin, LPn_4_Pin};
   uint16_t address_buffer[5] = {VL53L5CX_SENSOR_0_I2C_ADDRESS, VL53L5CX_SENSOR_1_I2C_ADDRESS, VL53L5CX_SENSOR_2_I2C_ADDRESS, VL53L5CX_SENSOR_3_I2C_ADDRESS, VL53L5CX_SENSOR_4_I2C_ADDRESS};
   float degree_buffer[5] = {0, -67.5, -22.5, 22.5, 67.5};
   int8_t x_point_buffer[5] = {0, 5, 2, -2, -5};
   int8_t y_point_buffer[5] = {0, 2, 5, 5, 2};

   for (i = 0; i < 5; i++) {
      HAL_GPIO_WritePin(GPIOF, LPn_0_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOF, LPn_1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOF, LPn_2_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOF, LPn_3_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOF, LPn_4_Pin, GPIO_PIN_RESET);

      HAL_GPIO_WritePin(GPIOF, LPn_buffer[i], GPIO_PIN_SET);

      HAL_Delay(1000);

      status += ToF_init
            (
                  &Dev[i],
                  hi2c1,
                  address_buffer[i],
                  VL53L5CX_RESOLUTION_8X8,
                  VL53L5CX_FREQUENCY_2HZ,
                  degree_buffer[i],
                  x_point_buffer[i],
                  y_point_buffer[i]
            );

      HAL_Delay(100);
   }

   HAL_GPIO_WritePin(GPIOF, LPn_0_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOF, LPn_1_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOF, LPn_2_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOF, LPn_3_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOF, LPn_4_Pin, GPIO_PIN_SET);

   return status;
}


int ToF_exec (
      VL53L5CX_Configuration *Dev_0,
      VL53L5CX_Configuration *Dev_1,
      VL53L5CX_Configuration *Dev_2,
      VL53L5CX_Configuration *Dev_3,
      VL53L5CX_Configuration *Dev_4,
      uint16_t measure_time,
      uint8_t mode,
      uint8_t accuracy,
      uint8_t max_distance
)
{

   uint8_t i, j, loop, status, isReady;
   VL53L5CX_Configuration *Dev[5] = {Dev_0, Dev_1, Dev_2, Dev_3, Dev_4};
   VL53L5CX_ResultsData    Results[5];
   Point array[MAX_COORDINATES];
   int num_coordinates = 0;
   uint8_t init = 0;
   uint8_t drive_count = 0;
   int16_t x_offset = 0;
   int16_t y_offset = 0;
   //uint8_t rotate_buffer[10] = {1,1,1,1,1,1,1,1,1,1};
   uint8_t rotate = 0;

   for (i = 0; i < 5; i++) {
      init += Dev[i]->platform.initialized;
   }

   for (i = 0; i < init; i++) {
      status = vl53l5cx_start_ranging(Dev[i]);
      if (status)
      {
         sprintf(buffer, "ToF %u start ranging failed\n\n", Dev[i]->platform.device_num);
         HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
      }
   }

   loop = 0;

   while(loop < measure_time)
   {
      status = vl53l5cx_check_data_ready(Dev[0], &isReady);

      if(isReady)
      {
         for (i = 0; i < init; i++) {

            status = vl53l5cx_get_ranging_data(Dev[i], &Results[i]);
            if (status)
            {
               sprintf(buffer, "ToF %u get ranging data failed\n", Dev[i]->platform.device_num);
               HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
               for (j = 0; j < 64; j++) {Dev[i]->platform.Point_buffer.distance_matrix[j] = 0;}
            }
            else if (!status) {
               for(j = 0; j < 64; j++)
               {
                  if (Results[i].distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*j] < (max_distance * 10)) {
                     if (Results[i].distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*j] % 10 >= 5) {
                        Dev[i]->platform.Point_buffer.distance_matrix[j]
                                                        = (Results[i].distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*j] / 10) + 1;
                     }      else {
                        Dev[i]->platform.Point_buffer.distance_matrix[j]
                                                        = Results[i].distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*j] / 10;
                     }
                  } else {
                     Dev[i]->platform.Point_buffer.distance_matrix[j] = 0;
                  }
               }
            }

            status = matrix_2_vector(Dev[i], &Results[i], accuracy);
            status = dist_2_point(Dev[i], 45);
            status = linear_transform(Dev[i]);

            if (i < (init - 1) && i > 0) {
               if (mode == 0) {
                  status = ToF_print_point_data(Dev[i], 0);
               }
            } else if (i == (init - 1)) {
               if (mode == 0) {
                 status = ToF_print_point_data(Dev[i], 1);
               }
            }
         }

         //angle = MPU6050_y(&hi2c5, &MPU6050);
         move=(captured_value1-pre_move)*195/2000;
         pre_move=captured_value1;

         //dis_front = Dev[2]->platform.Point_buffer.distance_vector[1];

         int16_t dis_front_buffer[4] = {Dev[2]->platform.Point_buffer.distance_vector[6], Dev[2]->platform.Point_buffer.distance_vector[7], Dev[3]->platform.Point_buffer.distance_vector[0], Dev[3]->platform.Point_buffer.distance_vector[1]};
         uint8_t g;
         int16_t dis_left_buffer[4] = {Dev[4]->platform.Point_buffer.distance_vector[7], Dev[4]->platform.Point_buffer.distance_vector[6], Dev[4]->platform.Point_buffer.distance_vector[5], Dev[4]->platform.Point_buffer.distance_vector[4]};
         int16_t dis_right_buffer[4] = {Dev[1]->platform.Point_buffer.distance_vector[0], Dev[1]->platform.Point_buffer.distance_vector[1], Dev[1]->platform.Point_buffer.distance_vector[2], Dev[1]->platform.Point_buffer.distance_vector[3]};


         uint8_t count_front = 0;

         int32_t sum_front = 0;
         uint8_t count_left = 0;

         int32_t sum_left = 0;
         uint8_t count_right = 0;

         int32_t sum_right = 0;

         for (g = 0; g < 4; g++) {
            if (dis_front_buffer[g] > 0) {
               count_front++;
               sum_front += dis_front_buffer[g];
            }
         }


         dis_front = sum_front / count_front;

         for (g = 0; g < 4; g++) {
            if (dis_left_buffer[g] > 0) {
               count_left++;
               sum_left += dis_left_buffer[g];
            }
         }


         dis_left = - sum_left / count_left;
         for (g = 0; g < 4; g++) {
            if (dis_right_buffer[g] > 0) {
               count_right++;
               sum_right += dis_right_buffer[g];
            }
         }


         dis_left = - sum_left / count_left;
         dis_right = sum_right / sum_left;

         //current coordinate
         x_coor= x_coor+cos(angle)*move;
         y_coor= y_coor+sin(angle)*move;
         makeCoordinate(array, &num_coordinates,x_coor,y_coor,dis_right,angle);
         makeCoordinate(array, &num_coordinates,x_coor,y_coor,dis_left,angle);


/*
         x_coor= x_coor-sin(angle)*move;
         y_coor= y_coor+cos(angle)*move;


         // Loop over devices (i from 0 to 4)
         for (int i = 0; i < 5; ++i) {
            // Loop over points (j from 0 to 7)
            for (int k = 0; k < 8; ++k) {
               // Access the specific point's x and y coordinates
               xx=Dev[i]->platform.Point_buffer.point_x[k];
               yy=Dev[i]->platform.Point_buffer.distance_vector[k];

               // Call the saveCoordinate function with the required parameters
               saveCoordinate(array, &num_coordinates, x_coor, y_coor, xx, yy, angle);
            }
         }
*/
         x_coor= x_coor-mysin(angle)*move;
         y_coor= y_coor+mycos(angle)*move;

         saveCoordinate(array, &num_coordinates, x_coor, y_coor, angle);

         if (drive_count > 2 && dis_front <40 && dis_front > 20) {

        	sprintf(buffer, "Rotate\n");
        	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

            Brake();
            //sprintf(buffer, "Rotatemode\n");
            //HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
             Decide direction based on left and right distances after rotation
            if (dis_left > 0 && dis_right > 0 && dis_left > dis_right) {
            //if (rotate_buffer[rotate] == 0) {

           // sprintf(buffer, "%u %u %u %u\n", Dev[2]->platform.Point_buffer.distance_vector[6], Dev[2]->platform.Point_buffer.distance_vector[7], Dev[3]->platform.Point_buffer.distance_vector[0], Dev[3]->platform.Point_buffer.distance_vector[1]);
           // HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
               MPU6050_set(&hi2c5, &MPU6050);
               Rotate_Left();
               MPU6050_Start(&hi2c5, &MPU6050);
   //            angle = angle+MPUangle;
               Brake();
               //angle++;
               angle = angle + M_PI/2;
               drive_count = 0;
               rotate++;
               HAL_Delay(1000);

            } else if (dis_left > 0 && dis_right > 0 && dis_left < dis_right){
            //} else if (rotate_buffer[rotate] == 1){
              //  sprintf(buffer, "%u %u %u %u\n", Dev[2]->platform.Point_buffer.distance_vector[6], Dev[2]->platform.Point_buffer.distance_vector[7], Dev[3]->platform.Point_buffer.distance_vector[0], Dev[3]->platform.Point_buffer.distance_vector[1]);
              //  HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
               MPU6050_set(&hi2c5, &MPU6050);
               Rotate_Right();
               MPU6050_Start(&hi2c5, &MPU6050);
      //         angle = angle-MPUangle;
               Brake();
               //angle--;
               angle = angle - M_PI/2;
               drive_count = 0;
               rotate++;
               HAL_Delay(3700);
            //} else if (rotate_buffer[rotate] == 2) {
            //	Brake();
            //}

            //dis_front=50;

         }
         else {
        	 Drive();
        	 //sprintf(buffer, "Dis_front:%u\n", dis_front);
        	// HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
            //sprintf(buffer, "Drivemode\n\n");
            //HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
            if(captured_value1>captured_value3){
               Driveright(); // Drive forward if the front is clear
          }
            else{
               Driveleft();
           }
         }
         /* Other vehicle logic can be added here */

         loop++;
         drive_count++;
      }
      HAL_Delay(5);
   }
   for (i = 0; i < init; i++) {
      status = vl53l5cx_stop_ranging(Dev[i]);
   }

   sprintf(buffer, "Stop ranging\n");
   HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

   return status;
}
void Drive(){
   dmode=0;
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);//right-drive
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//left-drive
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period *65) / 100);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (htim3.Init.Period *60) / 100);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period *63) / 100);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (htim3.Init.Period *70) / 100);
}
void Driveleft(){
	dmode=0;
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);//right-drive
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//left-drive
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period *67) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (htim3.Init.Period *67) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period *61) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (htim3.Init.Period *65) / 100);

}
void Driveright(){
	dmode=0;
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);//right-drive
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//left-drive
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period *61) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (htim3.Init.Period *61) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period *63) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (htim3.Init.Period *68) / 100);
}
void Rotate_Right(){
   dmode=1;
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);//right-drive
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);//left-drive
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period *85) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (htim3.Init.Period *85) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period *85) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (htim3.Init.Period *85) / 100);
}
void Rotate_Left(){
   dmode=2;
   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);//right-drive
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);//left-drive
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period *77) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (htim3.Init.Period *77) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period *77) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (htim3.Init.Period *77) / 100);
}
void Brake(){
   dmode=3;
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (htim3.Init.Period *0) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (htim3.Init.Period *0) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (htim3.Init.Period *0) / 100);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (htim3.Init.Period *0) / 100);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
   if(dmode==0){
      if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
      {
         captured_value1 ++;
      }
      if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
      {

         captured_value2 ++;
      }
      if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
      {
         captured_value3 ++;
      }
      if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
      {

         captured_value4 ++;
      }
   }

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
   /* User can add his own implementation to report the HAL error return state */
   __disable_irq();
   while (1)
   {
   }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
   /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

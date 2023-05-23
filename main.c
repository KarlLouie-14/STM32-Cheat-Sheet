// I2C BUS SCAN - WORKING!
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t Buffer[25] = {0};
uint8_t Space[] = " - ";
uint8_t StartMSG[] = "Starting I2C Scanning: \r\n";
uint8_t EndMSG[] = "Done! \r\n\r\n";

//main()
uint8_t i = 0, ret;

/*-[ I2C Bus Scanning ]-*/
HAL_UART_Transmit(&hlpuart1, StartMSG, sizeof(StartMSG), 10000);
for(i=1; i<128; i++)
{
    ret = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i<<1), 3, 5);
    if (ret != HAL_OK) /* No ACK Received At That Address */
    {
        HAL_UART_Transmit(&hlpuart1, Space, sizeof(Space), 10000);
    }
    else if(ret == HAL_OK)
    {
        sprintf(Buffer, "0x%X", i);
        HAL_UART_Transmit(&hlpuart1, Buffer, sizeof(Buffer), 10000);
    }
}
HAL_UART_Transmit(&hlpuart1, EndMSG, sizeof(EndMSG), 10000);
/*--[ Scanning Done ]--*/

//while()

--------------------------------------------------------------
//I2C Check Particular Device - WORKING!
static const uint8_t GPIO_ADDR = 0x20 << 1;	// 0x40
static const uint8_t ADC1_ADDR = 0x48 << 1;	// 0x90
static const uint8_t ADC2_ADDR = 0x4A << 1;	// 0x94
static const uint8_t REG_CUR = 0x00;

//main()
HAL_StatusTypeDef ret;
uint8_t buf[12];
int16_t val;
float cur;

//while()
buf[0] = REG_CUR;
ret = HAL_I2C_Master_Transmit(&hi2c1, ADC2_ADDR, buf, 1, HAL_MAX_DELAY);
if (ret != HAL_OK) {
  strcpy((char*)buf, "Error Tx\r\n");
} else {
  ret = HAL_I2C_Master_Receive(&hi2c1, ADC2_ADDR, buf, 2, HAL_MAX_DELAY);
  if (ret != HAL_OK) {
          strcpy((char*)buf, "Error Rx\r\n");
      } else {
          val = ((int16_t)buf[0] << 4) | (buf[1] >> 4); // Combine the bytes
          // CALCULATION HERE
          sprintf((char*)buf, "%u Amps\r\n", ((unsigned int)val));
      }
}

//strcpy((char*)buf, "Hello!\r\n");
HAL_UART_Transmit(&hlpuart1, buf, strlen((char*)buf), HAL_MAX_DELAY);
HAL_Delay(500);

------------------------------------------------------------
// Data Structure - WORKING!
union RGB_DEF {
	uint32_t bits;
	struct {
		uint8_t led_number;
		uint8_t green;
		uint8_t red;
		uint8_t blue;
	};
};

struct PINDEF {
	GPIO_TypeDef *port; //first 4 bytes
	uint32_t pin; //second 4 bytes
};

struct HDD {
	struct PINDEF enable;
	struct PINDEF detect;
	union RGB_DEF led;
} Hdd[HDD_SIZE];

void initialize_Hdd (){
	Hdd[ 0].enable.port = GPIOA; Hdd[ 0].enable.pin = GPIO_PIN_15; Hdd[ 0].detect.port = GPIOA; Hdd[ 0].detect.pin = GPIO_PIN_0;  Hdd[ 0].led.led_number = 0;  Hdd[ 0].led.red = 0;   Hdd[ 0].led.green = 0;   Hdd[ 0].led.blue = 0;
	Hdd[ 1].enable.port = GPIOB; Hdd[ 1].enable.pin = GPIO_PIN_3;  Hdd[ 1].detect.port = GPIOD; Hdd[ 1].detect.pin = GPIO_PIN_2;  Hdd[ 1].led.led_number = 1;  Hdd[ 1].led.red = 0;   Hdd[ 1].led.green = 0;   Hdd[ 1].led.blue = 0;
    //...
    //Send
}

//main()
initialize_Hdd();
--------------------------------------------------------------------------
// Misc Functions - WORKING!
#define MAX_LED 40
#define USE_BRIGHTNESS 1
#define PI 3.14159265
#define HDD_SIZE 40

uint8_t LED_Data[MAX_LED][4];
uint8_t LED_Mod[MAX_LED][4];  // for brightness
uint16_t pwmData[(24*MAX_LED)+50];
int datasentflag=0;

void Set_LED (int LEDnum, int Red, int Green, int Blue){
	LED_Data[LEDnum][0] = LEDnum;
	LED_Data[LEDnum][1] = Green;
	LED_Data[LEDnum][2] = Red;
	LED_Data[LEDnum][3] = Blue;
}

void WS2812_Send (void){
	uint32_t indx=0;
	uint32_t color;
	Set_Brightness(5);

	for (int i= 0; i<MAX_LED; i++){
		#if USE_BRIGHTNESS
			color = ((LED_Mod[i][1]<<16) | (LED_Mod[i][2]<<8) | (LED_Mod[i][3]));
		#else
			color = ((LED_Data[i][1]<<16) | (LED_Data[i][2]<<8) | (LED_Data[i][3]));
		#endif

		for (int i=23; i>=0; i--){
			if (color&(1<<i)){			
				pwmData[indx] = 26;  // 2/3 of 40
			} else pwmData[indx] = 14;  // 1/3 of 40
			indx++;
		}
	}

	for (int i=0; i<50; i++){	{
		pwmData[indx] = 0;
		indx++;
	}

	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t *)pwmData, indx);
	while (!datasentflag){};
	datasentflag = 0;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_2);
	datasentflag=1;
}
	
//main()
Set_LED(0, 255, 0, 0);
WS2812_Send();
	
--------------------------------------------------------------------------
// Input Scan using above data structure - WORKING!
//main()
volatile int diskPresent = 0;

//while()
diskPresent = 0;

// Scan through the ports and see if there's any disk present
for(int num = 0; num<MAX_LED; num++){
  if(HAL_GPIO_ReadPin(Hdd[num].detect.port, Hdd[num].detect.pin) == GPIO_PIN_SET){
      diskPresent = 1;
  }
}

// If there are no disks, run the test cycle lighting all LEDs
if (diskPresent == 0){
  for(int num = 0; num<MAX_LED; num++){
      Set_LED(num, 255, 255, 120);
      WS2812_Send();
     // HAL_Delay(1);
  }
  for(int num = 0; num<MAX_LED; num++){
      Set_LED(num, 0, 0, 0);
      WS2812_Send();
     // HAL_Delay(1);
  }
}
// Else if there is a disk present, start the main sequence which:
else {
  for(int num = 0; num<MAX_LED; num++){
      if(HAL_GPIO_ReadPin(Hdd[num].detect.port, Hdd[num].detect.pin) == GPIO_PIN_SET)
          Set_LED(num, 0, 255, 0);	// Lights up an LED to green in the disk's location
      else
          Set_LED(num, 255, 0, 0);	// or Lights up an LED to red where no disks are present
  }
  WS2812_Send();
}
--------------------------------------------------------------------------
// Basic UART RX & TX - DMA (non blocking) - RX NOT WORKING PROPERLY!
unsigned char Rx_data[7];  //  creating a buffer of 7 bytes
uint8_t count = 0;
char str[10];

void HAL_UARTEx_RxHalfCpltCallback(UART_HandleTypeDef *huart, uint16_t Size){
	HAL_UART_Transmit_DMA(&hlpuart1, Rx_data, strlen (Rx_data));	// Transmit received data (not working as of right now)
	HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, Rx_data, sizeof(Rx_data));
	__HAL_DMA_DISABLE_IT(&hdma_lpuart1_rx, DMA_IT_HT);
	memset(Rx_data, 0, strlen (Rx_data));
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	//	__HAL_DMA_DISABLE_IT(&hdma_usart2_tx, DMA_IT_HT);
	datasentflag=1;
}

//main()
 HAL_UARTEx_ReceiveToIdle_DMA (&hlpuart1, Rx_data, sizeof(Rx_data));  // Receive 6 Bytes of data
 __HAL_DMA_DISABLE_IT(&hdma_lpuart1_rx, DMA_IT_HT);
-------------------------------------------------------------------------------




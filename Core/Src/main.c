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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define BUFFER_SIZE 10
#define BUFFER_SIZE_STAPTIME 11

char timebuffer[30];
char date[30];

UART_HandleTypeDef huart2;
char received_data[BUFFER_SIZE];
uint16_t data_index = 0;



/* -------------StapTime Degiskenleri------------ */

char stapTimeBuffer[BUFFER_SIZE_STAPTIME];
int hexToDecimal;
int stapTime_mi =0;
int giris =0;

int sure =0;
int deltasure;

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR   (60 * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY    (24 * SECONDS_PER_HOUR)
#define SECONDS_PER_YEAR   (365 * SECONDS_PER_DAY)
#define SECONDS_PER_MONTH  (30 * SECONDS_PER_DAY)

const int days_in_months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // Şubat için 28 gün (artık yıl dikkate alınmamıştır)

unsigned long seconds_remaining;
int artik_yil =0;

/*unsigned long stapYil;*/
uint32_t stapYil;


/* -------------StapTime End --------------------- */


int date_Time = 2;
int reset =0;
bool date2kereyazma =1;
bool butondan2kereyazma = 0;
bool sadeceButondaUyar =1;
int hataliTarih = 0;

bool kalvyedenGiris=0;


int hata =0;
int deger=0;
int flashayazkontrol=0;



uint8_t yil;
uint8_t Ay;
uint8_t Gun;
uint8_t saat;
uint8_t dakika;
uint8_t saniye;


uint32_t FlashAdress = 0x08008000;
#define FLAG_ADDRESS 0x0800FC00  // Flash bellekte kullanılmayan bir adres

int birDakika = 0;
char *ptr;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
RTC_TimeTypeDef sTime ;
RTC_DateTypeDef sDate ;

FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t PageError;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
int debug;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim15;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM15_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void process_received_data(char *data);
void HataliDateGirisi(int Hangisi);
void process_stap_data(char *data);

void Clear_UART_Buffer(UART_HandleTypeDef *huart) {
    __HAL_UART_FLUSH_DRREGISTER(huart);  // Data register temizlenir
    __HAL_UART_CLEAR_OREFLAG(huart);     // Overrun bayrağı temizlenir
    __HAL_UART_CLEAR_PEFLAG(huart);      // Parity bayrağı temizlenir
    __HAL_UART_CLEAR_FEFLAG(huart);      // Framing bayrağı temizlenir
    __HAL_UART_CLEAR_NEFLAG(huart);      // Noise bayrağı temizlenir
}

int add_leap_year(int year) {
    int arttir = 0;
    for (int i = 1970; i < year; i++) {
        if (i % 4 == 0 && (i % 100 != 0 || i % 400 == 0))
            arttir--;
    }
    return arttir;
}


void check_reset_cause(void) {

	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        // Güç butonuyla reset (NRST pinine basıldığında)
    	HAL_UART_Transmit(&huart2, (uint8_t*)"Resetlendi.", sizeof("Resetlendi.") - 1, 1000);
    	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
    }
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

void flashaYaz()
{

	if (HAL_FLASH_Unlock()!= HAL_OK) {
	    hata=1;
	}

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FlashAdress;
    EraseInitStruct.NbPages = 8;
    deger++;
	//HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FlashAdress, sDate.Year);
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
           hata = 2;
       }
       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAdress, (uint16_t)sTime.Seconds) != HAL_OK) {
           hata = 3;
       }

       FlashAdress += 2;
       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAdress, (uint16_t)sTime.Minutes) != HAL_OK) {
        }
       FlashAdress += 2;

       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAdress, (uint16_t)sTime.Hours) != HAL_OK) {
       }

       FlashAdress += 2;

       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAdress, (uint16_t)sDate.Year) != HAL_OK) {
           hata = 4;
       }
       FlashAdress += 2;
       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAdress, (uint16_t)sDate.Month) != HAL_OK) {
           hata = 4;
       }

       FlashAdress += 2;

       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAdress, (uint16_t)sDate.Date) != HAL_OK) {
           hata = 5;
       }
       FlashAdress -= 10;
       HAL_FLASH_Lock();
}


uint16_t yazilanSaniye;
uint16_t yazilanDakika;
uint16_t yazilanSaat;
uint16_t yazilanYil;
uint16_t yazilanAy;
uint16_t yazilanGun;

void flashOkumaIslemi(void)
{
	yazilanSaniye = *(uint16_t*)(FlashAdress);
	yazilanDakika = *(uint16_t*)(FlashAdress+=2);
	yazilanSaat = *(uint16_t*)(FlashAdress+=2);
	yazilanYil = *(uint16_t*)(FlashAdress+=2);
	yazilanAy = *(uint16_t*)(FlashAdress+=2);
	yazilanGun = *(uint16_t*)(FlashAdress+=2);
	FlashAdress-=10;
}

void ResettenDonunceGuncelle()
{
	sTime.Seconds = *(uint16_t*)(FlashAdress);
	sTime.Minutes = *(uint16_t*)(FlashAdress+=2);
	sTime.Hours = *(uint16_t*)(FlashAdress+=2);
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	sDate.Year = *(uint16_t*)(FlashAdress+=2);
	sDate.Month = *(uint16_t*)(FlashAdress+=2);
	sDate.Date = *(uint16_t*)(FlashAdress+=2);
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	FlashAdress-=10;
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
  MX_RTC_Init();
  MX_USART2_UART_Init();
  MX_TIM15_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
  __HAL_UART_FLUSH_DRREGISTER(&huart2);
  flashOkumaIslemi();

  check_reset_cause();
  HAL_TIM_Base_Start_IT(&htim15);
  HAL_TIM_Base_Start_IT(&htim14);
  ResettenDonunceGuncelle();
  HAL_UART_Receive_IT(&huart2, (uint8_t *)received_data, 1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(1 == flashayazkontrol)
	  {
		  flashaYaz();
		  flashayazkontrol =0;
	  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 48000;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 19;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 4800;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	 HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	 HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	 if (htim->Instance == TIM14)
	 {
		 if(2 == date_Time)
		 	  {
			 deltasure = (HAL_GetTick() -sure) ;
			 sure = HAL_GetTick();


		 	  snprintf(date,sizeof(date),"Date: %02d.%02d.%02d\t",sDate.Date,sDate.Month,sDate.Year);
		 	  snprintf(timebuffer,sizeof(date),"Time: %02d.%02d.%02d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);

		 	  HAL_UART_Transmit(&huart2, (uint8_t *)date,sizeof(date), 300);
		 	  HAL_UART_Transmit(&huart2, (uint8_t *)timebuffer,sizeof(date), 300);
		 	  }
		 birDakika++;
		 if(10 == birDakika)
		 {
			 birDakika = 0;
			 flashayazkontrol=1;
		 }
	 }

	 if (htim->Instance == TIM15){

	 if (received_data[reset++] == 'r')
	         NVIC_SystemReset();
	 if(10 == reset)
		 reset = 0;

	 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) || kalvyedenGiris == 1 )
	 {
		 while((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) );
		 stapTime_mi =0;
		 giris = 0;
		 date_Time=0;
		 /*HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"Aralara '.' Koyarak tarih ve zaman giriniz:  ", sizeof("Aralara '.' Koyarak tarih ve zaman giriniz:  ") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"Ornek Format 19.20.24", sizeof("Ornek Format 19.20.24") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"Date: ", sizeof("Date: ") - 1, 1000);*/

		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"---------------------------", sizeof("---------------------------") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"Aralara '.' Koyarak giriş yapiniz.", sizeof("Aralara '.' Koyarak giriş yapiniz.") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"Ornek Format : Date: 01.01.70", sizeof("Ornek Format : Date: 01.01.70") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"StapTime girmek için 's' karakteri giniz.", sizeof("StapTime girmek için 's' karakteri giniz.") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"---------------------------", sizeof("---------------------------") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
		 HAL_UART_Transmit(&huart2, (uint8_t*)"Date: ", sizeof("Date: ") - 1, 1000);

		 butondan2kereyazma = 1;
		 kalvyedenGiris = 0;
	 }
	 }

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        if (received_data[data_index] == '\r') { // \r

            received_data[data_index] = '\0';  // \0
            if(0 == stapTime_mi)
            	process_received_data(received_data);
            else if(1 == stapTime_mi)
            {
            	if(0 == giris)
            	{
           		 HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
           		 HAL_UART_Transmit(&huart2, (uint8_t*)"Decimal: ", sizeof("Decimal: ") - 1, 1000);
           		 giris++;
            	}
            	else if(1 == giris)
            	{
            		giris++;
            		process_stap_data(received_data);
            		//HAL_UART_Transmit(&huart2, (uint8_t*)" \t", sizeof(" \t") - 1, 1000);
            		HAL_UART_Transmit(&huart2, (uint8_t*)received_data, sizeof(received_data) - 1, 1000);
            		HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
            		HAL_UART_Transmit(&huart2, (uint8_t*)"Binary: ", sizeof("Binary: ") - 1, 1000);
            		HAL_UART_Transmit(&huart2, (uint8_t*)stapTimeBuffer, sizeof(stapTimeBuffer) - 1, 1000);
            		HAL_UART_Transmit(&huart2, (uint8_t*)"\n\r", sizeof("\n\r") - 1, 1000);
            	}
            }
            data_index = 0;

        }
        else if ((received_data[data_index] >= '0' && received_data[data_index] <= '9') || received_data[data_index] == '.'  )
        {
        	data_index++;

        }
        else if (received_data[data_index] == 'r' )
        {
        	data_index++;
        }
        else if (received_data[data_index] == 'u' )
        {
        	kalvyedenGiris = 1;
        	received_data[data_index] = 'z';
        	data_index=0;

        }
        else if (received_data[0] == 's')
        {
        	stapTime_mi = 1;
        }
        else if(1 == stapTime_mi)
        {
        	if((received_data[data_index] >= '0' && received_data[data_index] <= '9') ||
        			(received_data[data_index] >= 'A' && received_data[data_index] <= 'F') )
        	{
            	data_index++;
        	}
        	else
        	{
        		data_index =0;
        		HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
        		HAL_UART_Transmit(&huart2, (uint8_t*)"!!!Lutfen Sadece 0-F Karakterleri Giriniz!!! ", sizeof("!!!Lutfen Sadece 0-F Karakterleri Giriniz!!! ") - 1, 1000);
        		HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
        		HAL_UART_Transmit(&huart2, (uint8_t*)"Decimal: ", sizeof("Decimal: ") - 1, 1000);
        	}

        }
        else{
        	data_index =0;

        	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
        	HAL_UART_Transmit(&huart2, (uint8_t*)"!!!Lutfen Sadece Sayi Giriniz!!! ", sizeof("!!!Lutfen Sadece Sayi Giriniz!!! ") - 1, 1000);
        	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);

        	if(0 == date_Time)
        		HAL_UART_Transmit(&huart2, (uint8_t*)"Date: ", sizeof("Date: ") - 1, 1000);
        	if(1 == date_Time && date2kereyazma == 1)
        		HAL_UART_Transmit(&huart2, (uint8_t*)"Time: ", sizeof("Time: ") - 1, 1000);

        	date2kereyazma=1;

        }
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&received_data[data_index], 1); //
    }
}

void process_stap_data(char *data)
{
	hexToDecimal = strtoul(received_data, NULL, 16);
	snprintf(stapTimeBuffer, sizeof(stapTimeBuffer), "%u", hexToDecimal);

	sDate.Year = ((hexToDecimal / SECONDS_PER_YEAR)); // + 1970);

	stapYil = ((hexToDecimal / SECONDS_PER_YEAR) + 1970);
    seconds_remaining = hexToDecimal % SECONDS_PER_YEAR;

    sDate.Month = ((seconds_remaining / SECONDS_PER_MONTH) % 12)+1;
    sDate.Date = (seconds_remaining / SECONDS_PER_DAY);

    seconds_remaining %= SECONDS_PER_MONTH;
    seconds_remaining %= SECONDS_PER_DAY;

    sTime.Hours = seconds_remaining / SECONDS_PER_HOUR;
    seconds_remaining %= SECONDS_PER_HOUR;

    sTime.Minutes = seconds_remaining / SECONDS_PER_MINUTE;
    sTime.Seconds = seconds_remaining % SECONDS_PER_MINUTE;


    artik_yil = add_leap_year(stapYil);
    sDate.Date += artik_yil;

    for(int i = 0;i<12;i++)
        {
        if(sDate.Date <32)
        break;
    	sDate.Date -= days_in_months[i];
        }

    if((stapYil % 4 == 0 && (stapYil % 100 != 0 || stapYil % 400 == 0)) != 1)
    	sDate.Date++;


    sDate.Year = stapYil % 100;



    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    date_Time=2;
    flashaYaz();
    flashOkumaIslemi();

}

void process_received_data(char *data) {

    if(0 == date_Time)
    {
    	if(0 == date2kereyazma && 0 == butondan2kereyazma && hataliTarih != 1)
    	HAL_UART_Transmit(&huart2, (uint8_t*)"Date: ", sizeof("Date: ") - 1, 1000);
    	else{
    		date2kereyazma=0;
    		butondan2kereyazma=0;
    	}
    	HAL_UART_Transmit(&huart2, (uint8_t *)data, strlen(data), 1000);
    	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);


    	/*
    	 *  strtok() fonksiyonu ikinci parametreye göre girilen
    	 *  stringi parçalar ve ilk karakterin işsaretcisini döndürür
    	 *  burda dönen dğere ptr adında bir pointer a atandı
    	 *  ve sonraki şlemlerde kaldığı yerden devam etmesi için
    	 *  NULL girildi.
    	 * */

    	ptr = strtok(received_data, ".");
    	    if (ptr != NULL) {
    	        Gun = atoi(ptr);
    	    }
    	    ptr = strtok(NULL, ".");
    	    if (ptr != NULL) {
    	        Ay = atoi(ptr);
    	    }
    	    ptr = strtok(NULL, ".");
    	    if (ptr != NULL) {
    	        yil = atoi(ptr);
    	    }

    	hataliTarih = 0;

    	sDate.Year = yil;
    	sDate.Month = Ay;
    	sDate.Date = Gun;
		if(yil > 99)
			HataliDateGirisi(0);
		else if(Ay > 12)
			HataliDateGirisi(1);
		else if(Gun> 30)
			HataliDateGirisi(2);
		else
		{
			date_Time = 1;

		}
    	if( 0 == hataliTarih )
    	HAL_UART_Transmit(&huart2, (uint8_t*)"Time: ", sizeof("Time: ") - 1, 1000);
  	  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    }
    else if ((1 == date_Time)){
    	HAL_UART_Transmit(&huart2, (uint8_t *)data, strlen(data), 1000);
    	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
			ptr = strtok(received_data, ".");
			if (ptr != NULL) {
				saat = atoi(ptr);
			}
			ptr = strtok(NULL, ".");
			if (ptr != NULL) {
				dakika = atoi(ptr);
			}
			ptr = strtok(NULL, ".");
			if (ptr != NULL) {
				saniye = atoi(ptr);
			}

		hataliTarih =0;

		sTime.Hours = saat;
		sTime.Minutes = dakika;
		sTime.Seconds = saniye;

		if(saat > 23)
			HataliDateGirisi(3);
		else if(dakika > 59)
			HataliDateGirisi(4);
		else if(saniye > 59)
			HataliDateGirisi(5);
		else
		{
			date_Time = 2;
			 flashaYaz();
			 flashOkumaIslemi();
		}
		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
   }

}

void HataliDateGirisi(int Hangisi)
{
	data_index =0;

	/* Eğer tarihten hata girilirse
	 * date_time = 0 döndürülüp
	 * tekrardan tarih girişi istenir
	 * eğer 3 ten büyükse zaman hatalıdır.
	 * bu durumdada date_time = 1 yapılarak
	 * zaman girşi tekrardan istenir.
	 * */


	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
	switch(Hangisi)
	{
	case 0:
		HAL_UART_Transmit(&huart2, (uint8_t*)"HATALI YIL GIRDINIZ !!!", sizeof("HATALI YIL GIRDINIZ !!!") - 1, 1000);
		break;
	case 1:
		HAL_UART_Transmit(&huart2, (uint8_t*)"HATALI AY GIRDINIZ !!!", sizeof("HATALI AY GIRDINIZ !!!") - 1, 1000);
		break;
	case 2:
		HAL_UART_Transmit(&huart2, (uint8_t*)"HATALI GUN GIRDINIZ !!!", sizeof("HATALI GUN GIRDINIZ !!!") - 1, 1000);
		break;
	case 3:
		HAL_UART_Transmit(&huart2, (uint8_t*)"HATALI SAAT GIRDINIZ !!!", sizeof("HATALI SAAT GIRDINIZ !!!") - 1, 1000);
		break;
	case 4:
		HAL_UART_Transmit(&huart2, (uint8_t*)"HATALI DAKIKA GIRDINIZ !!!", sizeof("HATALI DAKIKA GIRDINIZ !!!") - 1, 1000);
		break;
	case 5:
		HAL_UART_Transmit(&huart2, (uint8_t*)"HATALI SANIYE GIRDINIZ !!!", sizeof("HATALI SANIYE GIRDINIZ !!!") - 1, 1000);
		break;
	}
	HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
	hataliTarih =1;

	if(Hangisi < 3)
	{
		date_Time = 0;
		HAL_UART_Transmit(&huart2, (uint8_t*)"Date: ", sizeof("Date: ") - 1, 1000);
	}
	else
	{
		date_Time = 1;
		HAL_UART_Transmit(&huart2, (uint8_t*)"Time: ", sizeof("Time: ") - 1, 1000);
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

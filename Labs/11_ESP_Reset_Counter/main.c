/*
 * 11_ESP_Reset_Counter
 */

/* ******************* Private Includes ******************* */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"


/* ******************* Private Macros ******************* */

/* ******************* Public Functions Prototype ******************* */

void ESP_Reset(void);

/* ******************* Helper Functions Prototype ******************* */


/* ******************* Private Global Variables ******************* */

static uint8_t resetCounter = 0;
static nvs_handle_t nvsHandle;

/* ******************* Application Start Function ******************* */

void app_main(void)
{
	nvs_flash_init();

	esp_err_t err = nvs_open("myStore",NVS_READWRITE,&nvsHandle);

	if(ESP_OK == err)
	{
		printf("OK ! \n");
	}
	else
	{
		printf("Error ! \n");
	}

	err = nvs_get_u8(nvsHandle, "RstCnt", &resetCounter);


	if(ESP_OK == err)
	{
		printf("OK ! \n");
	}
	else
	{
		printf("Error ! \n");
	}

	printf("RstCnt = %d \n",resetCounter);

	vTaskDelay(5000/portTICK_PERIOD_MS);

	ESP_Reset();

	while(1)
	{

		vTaskDelay(100/portTICK_PERIOD_MS);
	}

}

/* ******************* Public Functions Implementation ******************* */

void ESP_Reset(void)
{
	resetCounter++;
	nvs_set_u8(nvsHandle, "RstCnt", resetCounter);
	esp_restart();
}

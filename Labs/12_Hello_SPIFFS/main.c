/*
 * 12_Hello_SPIFFS
 */

/* ******************* Private Includes ******************* */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"

/* ******************* Private Macros ******************* */

/* ******************* Public Functions Prototype ******************* */


/* ******************* Helper Functions Prototype ******************* */


/* ******************* Private Global Variables ******************* */


/* ******************* Application Start Function ******************* */

void app_main(void)
{
	char line[100];
	esp_vfs_spiffs_conf_t config;

	config.base_path = "/mydir";
	config.partition_label = "storage";
	config.max_files = 2;
	config.format_if_mount_failed = true;

	esp_vfs_spiffs_register(&config);

	FILE * story1 =  fopen("/mydir/sub/story1.txt","r");

	if(story1 == NULL)
	{
		printf("Can't open Story 1 \n");
	}
	else
	{

		while(fgets(line,sizeof(line),story1) != NULL)
		{
			printf(line);
		}
		fclose(story1);
	}

	printf("\n");

	FILE * story2 =  fopen("/mydir/story2.txt","r");

	if(story2 == NULL)
	{
		printf("Can't open Story 2 \n");
	}
	else
	{
		while(fgets(line,sizeof(line),story2) != NULL)
		{
			printf(line);
		}
		fclose(story2);
	}

	printf("\n");
	esp_vfs_spiffs_unregister("storage");

	while(1)
	{

		vTaskDelay(100/portTICK_PERIOD_MS);
	}

}

/* ******************* Public Functions Implementation ******************* */


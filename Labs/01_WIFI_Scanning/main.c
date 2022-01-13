/*
	* LAP 01
	* Wifi_Scanning
*/

/* ******************* Private Includes ******************* */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

/* ******************* Private Macros ******************* */

#define MAX_SCAN_RESULT 20

/* ******************* Public Functions Prototype ******************* */

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event);
void Wifi_Init(wifi_mode_t mode);
void Wifi_Start(void);
void Wifi_Scan(bool block);
void Wifi_GetScanResult(void);
void Wifi_Process(uint8_t eventFlags[]);

/* ******************* Helper Functions Prototype ******************* */

void Wifi_PrintEventName(system_event_id_t event);
char * Wifi_GetAuthModeName(wifi_auth_mode_t authMode);
void Wifi_PrintScanResult(wifi_ap_record_t * ap_records ,uint16_t count);


/* ******************* Private Global Variables ******************* */

static uint8_t Wifi_EventFlags[SYSTEM_EVENT_MAX] = {0};


/* ******************* Application Start Function ******************* */

void app_main(void)
{
	Wifi_Init(WIFI_MODE_STA);

	Wifi_Start();

	Wifi_Scan(false);

	while(1)
	{

		Wifi_Process(Wifi_EventFlags);

		/* Delay 1 Second */
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}

}

/* ******************* Public Functions Implementation ******************* */

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event)
{

	Wifi_PrintEventName(event->event_id);

	Wifi_EventFlags[event->event_id] = 1;

	return ESP_OK;
}

void Wifi_Init(wifi_mode_t mode)
{
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

	/* Initialize NVS Flash */
	nvs_flash_init();

	/* Initialize TCP/IP Stack */
	tcpip_adapter_init();

	/* Register Wi-fi Event Handler */
	esp_event_loop_init(Wifi_EventHandler,NULL);

	/* Initialize WI-FI Module */
	esp_wifi_init(&wifi_init_config);

	/* Set Wi-fi Mode */
	esp_wifi_set_mode(mode);

}

void Wifi_Start(void)
{
	/* Start Wi-fi Feature */
	esp_wifi_start();
}

void Wifi_Scan(bool block)
{

	wifi_scan_config_t scan_config = {0};

	/* Set Scanning Configuration */
	scan_config.ssid = 0;
	scan_config.bssid = 0;
	scan_config.channel = 0;
	scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
	scan_config.show_hidden = true;
	scan_config.scan_time.active.min = 0;
	scan_config.scan_time.active.max = 1000;

	/* Start Scanning  */
	esp_wifi_scan_start(&scan_config,block);

}

void Wifi_GetScanResult(void)
{
	/* Read Scanning Result */

	uint16_t APs_Count = MAX_SCAN_RESULT;
	wifi_ap_record_t ap_records[MAX_SCAN_RESULT];

	esp_wifi_scan_get_ap_records(&APs_Count, ap_records);

	/* Print Scanning Result */
	Wifi_PrintScanResult(ap_records,APs_Count);
}

void Wifi_Process(uint8_t eventFlags[])
{
	if (eventFlags[SYSTEM_EVENT_SCAN_DONE] == 1)
	{
		eventFlags[SYSTEM_EVENT_SCAN_DONE] = 0;
		Wifi_GetScanResult();
	}
	else
	{
		/* Do Nothing */
	}
}
/* ******************* Helper Functions Implementation ******************* */

void Wifi_PrintEventName(system_event_id_t event)
{
	char * eventNameList[] =
	{
		    "SYSTEM_EVENT_WIFI_READY",
		    "SYSTEM_EVENT_SCAN_DONE",
		    "SYSTEM_EVENT_STA_START",
		    "SYSTEM_EVENT_STA_STOP",
		    "SYSTEM_EVENT_STA_CONNECTED",
		    "SYSTEM_EVENT_STA_DISCONNECTED",
		    "SYSTEM_EVENT_STA_AUTHMODE_CHANGE",
		    "SYSTEM_EVENT_STA_GOT_IP",
		    "SYSTEM_EVENT_STA_LOST_IP",
		    "SYSTEM_EVENT_STA_BSS_RSSI_LOW",
		    "SYSTEM_EVENT_STA_WPS_ER_SUCCESS",
		    "SYSTEM_EVENT_STA_WPS_ER_FAILED",
		    "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT",
		    "SYSTEM_EVENT_STA_WPS_ER_PIN",
		    "SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP",
		    "SYSTEM_EVENT_AP_START",
		    "SYSTEM_EVENT_AP_STOP",
		    "SYSTEM_EVENT_AP_STACONNECTED",
		    "SYSTEM_EVENT_AP_STADISCONNECTED",
		    "SYSTEM_EVENT_AP_STAIPASSIGNED",
		    "SYSTEM_EVENT_AP_PROBEREQRECVED",
		    "SYSTEM_EVENT_ACTION_TX_STATUS",
		    "SYSTEM_EVENT_ROC_DONE",
		    "SYSTEM_EVENT_STA_BEACON_TIMEOUT",
		    "SYSTEM_EVENT_FTM_REPORT",
		    "SYSTEM_EVENT_GOT_IP6",
		    "SYSTEM_EVENT_ETH_START",
		    "SYSTEM_EVENT_ETH_STOP",
		    "SYSTEM_EVENT_ETH_CONNECTED",
		    "SYSTEM_EVENT_ETH_DISCONNECTED",
		    "SYSTEM_EVENT_ETH_GOT_IP",
		    "SYSTEM_EVENT_MAX"
	};

	printf("%s\n",eventNameList[event]);
}

char * Wifi_GetAuthModeName(wifi_auth_mode_t authMode)
{
	char * AuthModeNameList[] =
	{
			"open",
			"WEP",
			"WPA_PSK",
			"WPA2_PSK",
			"WPA_WPA2_PSK",
			"WPA2_ENTERPRISE",
			"WPA3_PSK",
			"WPA2_WPA3_PSK"
	};

	return AuthModeNameList[authMode];
}

void Wifi_PrintScanResult(wifi_ap_record_t * ap_records ,uint16_t count)
{

	printf("Scanning Found : %d Access Points \n",count);
	printf("             SSID                |         MAC       | Channel |  Auth Mode      |  RSSI \n");
	printf("*****************************************************************************************\n");
	for(uint16_t index = 0; index < count; index++)
	{
		printf("%32s | %02X:%02X:%02X:%02X:%02X:%02X | %7d | %15s | %4d ",
				(char *)ap_records[index].ssid,
				ap_records[index].bssid[0] , ap_records[index].bssid[1] , ap_records[index].bssid[2],
				ap_records[index].bssid[3] , ap_records[index].bssid[4] , ap_records[index].bssid[5],
				ap_records[index].primary,
				Wifi_GetAuthModeName(ap_records[index].authmode),ap_records[index].rssi);

		printf("\n");
	}
	printf("*****************************************************************************************\n");

}

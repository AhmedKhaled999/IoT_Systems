/*
 * LAP 03
 * WIFI_Being_AP
 */

/* ******************* Private Includes ******************* */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

/* ******************* Private Macros ******************* */

#define WIFI_AP_CONFIG_SSID   			 "AG_ESP"
#define WIFI_AP_CONFIG_PASSWORD  		 "AG_ESP32"

/* ******************* Public Functions Prototype ******************* */

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event);
void Wifi_Init(wifi_mode_t mode);
void Wifi_Start(void);
void Wifi_Process(uint8_t eventFlags[]);
void Wifi_Init_Access_Point(char * ssid, char * password);
void Wifi_AP_Show_STA_Connected_Info(system_event_info_t info);
void Wifi_AP_Show_STA_IP_Assigned_Info(system_event_info_t info);
void Wifi_AP_Show_STA_Disconnected_Info(system_event_info_t info);

/* ******************* Helper Functions Prototype ******************* */

void Wifi_PrintEventName(system_event_id_t event);
char * Wifi_GetAuthModeName(wifi_auth_mode_t authMode);
void Wifi_PrintIPString(uint32_t ip);
char * Wifi_ConvertMacToString(uint8_t Mac[]);

/* ******************* Private Global Variables ******************* */

static uint8_t Wifi_EventFlags[SYSTEM_EVENT_MAX] = {0};
static system_event_info_t Wifi_EventInfo[SYSTEM_EVENT_MAX];


/* ******************* Application Start Function ******************* */

void app_main(void)
{
	Wifi_Init(WIFI_MODE_AP);

	Wifi_Init_Access_Point(WIFI_AP_CONFIG_SSID,WIFI_AP_CONFIG_PASSWORD);

	Wifi_Start();

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
	memcpy(&(Wifi_EventInfo[event->event_id]),&(event->event_info),sizeof(system_event_info_t));

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

void Wifi_Init_Access_Point(char * ssid, char * password)
{
	wifi_config_t AP_Config = {0};

	strcpy((char *)AP_Config.ap.ssid, ssid);
	strcpy((char *)AP_Config.ap.password, password);
	AP_Config.ap.ssid_len = 6;
	AP_Config.ap.authmode = WIFI_AUTH_WPA2_PSK;
	AP_Config.ap.ssid_hidden = 0;
	AP_Config.ap.max_connection=5;


	esp_wifi_set_config(ESP_IF_WIFI_AP, &AP_Config);
}

void Wifi_AP_Show_STA_Connected_Info(system_event_info_t info)
{
	printf("New Station Connected : \n");
	printf("Association ID : %d \n",info.sta_connected.aid);
	printf("MAC : %s \n",Wifi_ConvertMacToString(info.sta_connected.mac));
	printf("***********************************\n");
}

void Wifi_AP_Show_STA_IP_Assigned_Info(system_event_info_t info)
{
	printf("Assigned IP For New Station : \n");
	printf("IP : ");
	Wifi_PrintIPString(info.ap_staipassigned.ip.addr);
	printf("\n");
	printf("***********************************\n");
}

void Wifi_AP_Show_STA_Disconnected_Info(system_event_info_t info)
{
	printf("A Station Disconnected : \n");
	printf("Association ID : %d \n",info.sta_disconnected.aid);
	printf("MAC : %s \n",Wifi_ConvertMacToString(info.sta_disconnected.mac));
	printf("***********************************\n");
}

void Wifi_Process(uint8_t eventFlags[])
{
	if(1 == eventFlags[SYSTEM_EVENT_AP_START])
	{
		eventFlags[SYSTEM_EVENT_AP_START] = 0;

	}
	else if(1 == eventFlags[SYSTEM_EVENT_AP_STACONNECTED])
	{
		eventFlags[SYSTEM_EVENT_AP_STACONNECTED] = 0;
		Wifi_AP_Show_STA_Connected_Info(Wifi_EventInfo[SYSTEM_EVENT_AP_STACONNECTED]);
	}
	else if(1 == eventFlags[SYSTEM_EVENT_AP_STAIPASSIGNED])
	{
		eventFlags[SYSTEM_EVENT_AP_STAIPASSIGNED] = 0;
		Wifi_AP_Show_STA_IP_Assigned_Info(Wifi_EventInfo[SYSTEM_EVENT_AP_STAIPASSIGNED]);
	}
	else if(1 == eventFlags[SYSTEM_EVENT_AP_STADISCONNECTED])
	{
		eventFlags[SYSTEM_EVENT_AP_STADISCONNECTED] = 0;
		Wifi_AP_Show_STA_Disconnected_Info(Wifi_EventInfo[SYSTEM_EVENT_AP_STADISCONNECTED]);
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

void Wifi_PrintIPString(uint32_t ip)
{
	uint8_t Byte0 = ip & 0xFF;
	uint8_t Byte1 = (ip >> 8) & 0xFF;
	uint8_t Byte2 = (ip >> 16) & 0xFF;
	uint8_t Byte3 = (ip >> 24) & 0xFF;

	printf("%d.%d.%d.%d",Byte0,Byte1,Byte2,Byte3);
}

char * Wifi_ConvertMacToString(uint8_t Mac[])
{
	static char str[18];

	sprintf(str,"%02X:%02X:%02X:%02X:%02X:%02X",Mac[0],Mac[1],Mac[2],Mac[3],Mac[4],Mac[5]);

	str[17] = '\0';

	return str;
}

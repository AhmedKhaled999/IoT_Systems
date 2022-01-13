/*
 * 08_WIFI_Parse_JSON_Server
 */

/* ******************* Private Includes ******************* */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "cJson.h"

/* ******************* Private Macros ******************* */

#define MAX_SCAN_RESULT 20
#define WIFI_AP_CONFIG_SSID   			 "AG_ESP"
#define WIFI_AP_CONFIG_PASSWORD  		 "AG_ESP32"

/* ******************* Public Functions Prototype ******************* */

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event);
void Wifi_Init(wifi_mode_t mode);
void Wifi_Start(void);
void Wifi_Scan(bool block);
void Wifi_GetScanResult(void);
void Wifi_Process(uint8_t eventFlags[]);
void Wifi_Init_Access_Point(char * ssid, char * password);
void Wifi_AP_Show_STA_Connected_Info(system_event_info_t info);
void Wifi_AP_Show_STA_IP_Assigned_Info(system_event_info_t info);
void Wifi_AP_Show_STA_Disconnected_Info(system_event_info_t info);
void Http_PrintEventName(esp_http_client_event_id_t event_id);
void HttpServer_Init(void);
esp_err_t HttpServer_RootEP_Handler(httpd_req_t *request);
esp_err_t HttpServer_PersonEP_Handler(httpd_req_t *request);

/* ******************* Helper Functions Prototype ******************* */

void Wifi_PrintEventName(system_event_id_t event);
char * Wifi_GetAuthModeName(wifi_auth_mode_t authMode);
void Wifi_PrintScanResult(wifi_ap_record_t * ap_records ,uint16_t count);
void Wifi_PrintIPString(uint32_t ip);
char * Wifi_ConvertMacToString(uint8_t Mac[]);
char * Wifi_GetReasonName(uint8_t reason);

/* ******************* Private Global Variables ******************* */

static uint8_t Wifi_EventFlags[SYSTEM_EVENT_MAX] = {0};
static system_event_info_t Wifi_EventInfo[SYSTEM_EVENT_MAX];

/* ******************* Application Start Function ******************* */

void app_main(void)
{

	Wifi_Init(WIFI_MODE_AP);

	Wifi_Init_Access_Point(WIFI_AP_CONFIG_SSID,WIFI_AP_CONFIG_PASSWORD);

	Wifi_Start();

	HttpServer_Init();

	while(1)
	{

		Wifi_Process(Wifi_EventFlags);

		/* Tick Delay */

		vTaskDelay(100/portTICK_PERIOD_MS);
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

void Wifi_Scan(bool block)
{

	wifi_scan_config_t scan_config;

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


void Http_PrintEventName(esp_http_client_event_id_t event_id)
{
	char * eventNameList[] =
	{
			"HTTP_EVENT_ERROR",
			"HTTP_EVENT_ON_CONNECTED",
			"HTTP_EVENT_HEADERS_SENT",
			"HTTP_EVENT_ON_HEADER",
			"HTTP_EVENT_ON_DATA",
			"HTTP_EVENT_ON_FINISH",
			"HTTP_EVENT_DISCONNECTED"
	};

	printf("HTTP EVENT : %s \n",eventNameList[event_id]);
}

void HttpServer_Init(void)
{
	esp_err_t ret;

	/* Create HTTP Server */
	httpd_handle_t server = NULL;

	/* Create Server Configuration */
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	/* Start Server */
	ret = httpd_start(&server , &config);

	if(ret != ESP_OK)
	{
		return;
	}

	printf("HTTP_SERVER_STARTED_SUCCESSFULY \n");

	httpd_uri_t RootEndPoint;

	RootEndPoint.uri 		= "/";
	RootEndPoint.method		= HTTP_GET;
	RootEndPoint.handler  	= HttpServer_RootEP_Handler;
	RootEndPoint.user_ctx 	= NULL;

	ret = httpd_register_uri_handler(server,&RootEndPoint);

	if(ret != ESP_OK)
	{
		return;
	}

	httpd_uri_t PersonEndPoint;

	PersonEndPoint.uri 		= "/person";
	PersonEndPoint.method	= HTTP_GET;
	PersonEndPoint.handler  = HttpServer_PersonEP_Handler;
	PersonEndPoint.user_ctx = NULL;

	ret = httpd_register_uri_handler(server,&PersonEndPoint);

	if(ret != ESP_OK)
	{
		return;
	}
}

esp_err_t HttpServer_PersonEP_Handler(httpd_req_t *request)
{
	esp_err_t ret;

	printf("Someone requested %s URI \n",request->uri);

	char responseMessage[] = "{\"Name\":\"Omar\",\"Age\":26,\"Father\":{\"Name\":\"Mohamed\",\"Age\":55},\"Brothers\":[{\"Name\":\"Ali\",\"Age\":18},{\"Name\":\"Mostafa\",\"Age\":30}]}";

	(void)httpd_resp_set_type(request, HTTPD_TYPE_JSON);

	ret = httpd_resp_send(request,responseMessage,sizeof(responseMessage) - 1);


	return ret;
}

esp_err_t HttpServer_RootEP_Handler(httpd_req_t *request)
{
	esp_err_t ret;

	printf("Someone requested %s URI \n",request->uri);

	char responseMessage[] = "Welcome to ESP Web Server . It's The Root Page";

	(void)httpd_resp_set_type(request, HTTPD_TYPE_TEXT);

	ret = httpd_resp_send(request,responseMessage,sizeof(responseMessage) - 1);


	return ret;
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

char * Wifi_GetReasonName(uint8_t reason)
{
	char * retval;

	switch (reason)
	{
	case 1 :
		retval = "WIFI_REASON_UNSPECIFIED";
		break;
	case 2 :
		retval = "WIFI_REASON_AUTH_EXPIRE";
		break;
	case 3 :
		retval = "WIFI_REASON_AUTH_LEAVE";
		break;
	case 4 :
		retval = "WIFI_REASON_ASSOC_EXPIRE";
		break;
	case 5 :
		retval = "WIFI_REASON_ASSOC_TOOMANY";
		break;
	case 6 :
		retval = "WIFI_REASON_NOT_AUTHED";
		break;
	case 7 :
		retval = "WIFI_REASON_NOT_ASSOCED";
		break;
	case 8 :
		retval = "WIFI_REASON_ASSOC_LEAVE";
		break;
	case 9 :
		retval = "WIFI_REASON_ASSOC_NOT_AUTHED";
		break;
	case 10 :
		retval = "WIFI_REASON_DISASSOC_PWRCAP_BAD";
		break;
	case 11 :
		retval = "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
		break;
	case 13 :
		retval = "WIFI_REASON_IE_INVALID";
		break;
	case 14 :
		retval = "WIFI_REASON_MIC_FAILURE";
		break;
	case 15 :
		retval = "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
		break;
	case 16 :
		retval = "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
		break;
	case 17 :
		retval = "WIFI_REASON_IE_IN_4WAY_DIFFERS";
		break;
	case 18 :
		retval = "WIFI_REASON_GROUP_CIPHER_INVALID";
		break;
	case 19 :
		retval = "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
		break;
	case 20 :
		retval = "WIFI_REASON_AKMP_INVALID";
		break;
	case 21 :
		retval = "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
		break;
	case 22 :
		retval = "WIFI_REASON_INVALID_RSN_IE_CAP";
		break;
	case 23 :
		retval = "WIFI_REASON_802_1X_AUTH_FAILED";
		break;
	case 24 :
		retval = "WIFI_REASON_CIPHER_SUITE_REJECTED";
		break;
	case 53 :
		retval = "WIFI_REASON_INVALID_PMKID";
		break;
	case 200 :
		retval = "WIFI_REASON_BEACON_TIMEOUT";
		break;
	case 201 :
		retval = "WIFI_REASON_NO_AP_FOUND";
		break;
	case 202 :
		retval = "WIFI_REASON_AUTH_FAIL";
		break;
	case 203 :
		retval = "WIFI_REASON_ASSOC_FAIL";
		break;
	case 204 :
		retval = "WIFI_REASON_HANDSHAKE_TIMEOUT";
		break;
	case 205 :
		retval = "WIFI_REASON_CONNECTION_FAIL";
		break;
	case 206 :
		retval = "WIFI_REASON_AP_TSF_RESET";
		break;
	case 207 :
		retval = "WIFI_REASON_ROAMING";
		break;
	default :
		retval = "Unknown reason !!";
		break;
	}

	return retval;
}

/*
 * 09_WIFI_Hello_MQTT
 */

/* ******************* Private Includes ******************* */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "mqtt_client.h"

/* ******************* Private Macros ******************* */

#define MAX_SCAN_RESULT 20

#if 0
#define WIFI_STA_CONFIG_SSID   		    "Orange-72A1"
#define WIFI_STA_CONFIG_PASSWORD  		"YMTY57DA91R"
#endif

#if 1
#define WIFI_STA_CONFIG_SSID   		    "AG_Phone"
#define WIFI_STA_CONFIG_PASSWORD  		"AG_Phone2022"
#endif

#define MQTT_CLIENT_EVENT_COUNT			(8)
#define MQTT_CLIENT_SUBSCRIBE_TOPIC_NAME			"ESP/AG/subscribe"
#define MQTT_CLIENT_PUBLISH_TOPIC_NAME				"ESP/AG/publish"

/* ******************* Public Functions Prototype ******************* */

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event);
void Wifi_Init(wifi_mode_t mode);
void Wifi_Start(void);
void Wifi_Scan(bool block);
void Wifi_GetScanResult(void);
void Wifi_Process(uint8_t eventFlags[]);
void Wifi_ConnectToAp(char * ssid, char * password);
void Wifi_DisconnectFromAp(void);
void Wifi_Show_Station_Connected_Info(system_event_info_t info);
void Wifi_Show_Station_Got_IP_Info(system_event_info_t info);
void Wifi_Show_Station_Disconnected_Info(system_event_info_t info);
void MqttClient_Init(void);
void MqttClient_Process(void);
esp_err_t MqttClient_Event_Handler(esp_mqtt_event_handle_t event);

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

static esp_mqtt_client_handle_t MQTT_Client = NULL;
static uint8_t MqttEventFlags[MQTT_CLIENT_EVENT_COUNT] = {0};
static esp_mqtt_event_t MqttEventsInfo[MQTT_CLIENT_EVENT_COUNT];

/* ******************* Application Start Function ******************* */

void app_main(void)
{
	Wifi_Init(WIFI_MODE_STA);

	Wifi_Start();

	Wifi_Scan(false);

	while(1)
	{

		Wifi_Process(Wifi_EventFlags);

		MqttClient_Process();

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


void Wifi_ConnectToAp(char * ssid, char * password)
{
	wifi_config_t Station_Config = {0};

	strcpy((char *)Station_Config.sta.ssid, ssid);
	strcpy((char *)Station_Config.sta.password, password);

	esp_wifi_set_config(ESP_IF_WIFI_STA, &Station_Config);

	esp_wifi_connect();
}

void Wifi_DisconnectFromAp(void)
{
	esp_wifi_disconnect();
}

void Wifi_Show_Station_Connected_Info(system_event_info_t info)
{
	printf("Station Connected AP SSID: %s\n",info.connected.ssid);
	printf("SSID Length: %d\n",info.connected.ssid_len);
	printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",info.connected.bssid[0],
			info.connected.bssid[1],info.connected.bssid[2],
			info.connected.bssid[3],info.connected.bssid[4],
			info.connected.bssid[5]);
	printf("Channel : %d\n",info.connected.channel);
	printf("Authentication Mode : %s\n",Wifi_GetAuthModeName(info.connected.authmode));
}

void Wifi_Show_Station_Got_IP_Info(system_event_info_t info)
{
	printf("IP : ");
	Wifi_PrintIPString(info.got_ip.ip_info.ip.addr);
	printf("\n");

	printf("Netmask : ");
	Wifi_PrintIPString(info.got_ip.ip_info.netmask.addr);
	printf("\n");

	printf("Gateway : ");
	Wifi_PrintIPString(info.got_ip.ip_info.gw.addr);
	printf("\n");
}

void Wifi_Show_Station_Disconnected_Info(system_event_info_t info)
{
	printf("Event : SYSTEM_EVENT_STA_DISCONNECTED\n");
	printf("Disconnected From AP SSID: %s\n",info.disconnected.ssid);
	printf("SSID Length: %d\n",info.disconnected.ssid_len);
	printf("MAC Address: %s \n",Wifi_ConvertMacToString(info.disconnected.bssid));
	printf("Disconnection Reason : %s\n",Wifi_GetReasonName(info.disconnected.reason));
}

void Wifi_Process(uint8_t eventFlags[])
{
	if (eventFlags[SYSTEM_EVENT_SCAN_DONE] == 1)
	{
		eventFlags[SYSTEM_EVENT_SCAN_DONE] = 0;
		Wifi_GetScanResult();
		Wifi_ConnectToAp(WIFI_STA_CONFIG_SSID,WIFI_STA_CONFIG_PASSWORD);
	}
	else if(1 == eventFlags[SYSTEM_EVENT_STA_CONNECTED])
	{
		eventFlags[SYSTEM_EVENT_STA_CONNECTED] = 0;
		Wifi_PrintEventName(SYSTEM_EVENT_STA_CONNECTED);

		Wifi_Show_Station_Connected_Info(Wifi_EventInfo[SYSTEM_EVENT_STA_CONNECTED]);
	}
	else if(1 == eventFlags[SYSTEM_EVENT_STA_GOT_IP])
	{
		eventFlags[SYSTEM_EVENT_STA_GOT_IP] = 0;
		Wifi_PrintEventName(SYSTEM_EVENT_STA_GOT_IP);

		Wifi_Show_Station_Got_IP_Info(Wifi_EventInfo[SYSTEM_EVENT_STA_GOT_IP]);

		MqttClient_Init();

	}
	else if(1 == eventFlags[SYSTEM_EVENT_STA_DISCONNECTED])
	{
		eventFlags[SYSTEM_EVENT_STA_DISCONNECTED] = 0;
		Wifi_PrintEventName(SYSTEM_EVENT_STA_DISCONNECTED);

		Wifi_Show_Station_Disconnected_Info(Wifi_EventInfo[SYSTEM_EVENT_STA_DISCONNECTED]);

		printf("Restarting ......");
		esp_restart();
	}
	else
	{
		/* Do Nothing */
	}
}

void MqttClient_Init(void)
{
	/* Create MQTT Client Configuration */

	esp_mqtt_client_config_t config =
	{
			.uri = "mqtt://test.mosquitto.org",
			.event_handle = MqttClient_Event_Handler,
			.lwt_topic = MQTT_CLIENT_PUBLISH_TOPIC_NAME,
			.lwt_msg = "Hi Guys , AG_ESP is dropped",
			.disable_auto_reconnect = true,
	};


	/* Create MQTT Client */
	MQTT_Client = esp_mqtt_client_init(&config);

	/* Start MQTT Client and Connect to Broker */
	esp_mqtt_client_start(MQTT_Client);
}

void MqttClient_Process(void)
{

	static uint8_t msgFlag = 0;

	for(uint8_t event = 0; event < MQTT_CLIENT_EVENT_COUNT; event++)
	{
		if(MqttEventFlags[event] == 1)
		{
			MqttEventFlags[event] = 0;

			switch(event)
			{
			case MQTT_EVENT_CONNECTED :

				printf("MQTT_EVENT_CONNECTED \n");

				/* Subscribe to the MQTT Topic */

				esp_mqtt_client_subscribe(MQTT_Client,MQTT_CLIENT_SUBSCRIBE_TOPIC_NAME,2);

				break;

			case MQTT_EVENT_DISCONNECTED :

				printf("MQTT_EVENT_DISCONNECTED \n");

				break;

			case MQTT_EVENT_SUBSCRIBED :

				printf("MQTT_EVENT_SUBSCRIBED \n");

				break;
			case MQTT_EVENT_UNSUBSCRIBED :

				printf("MQTT_EVENT_UNSUBSCRIBED \n");

				/* Disconnect from the MQTT Broker */
#if 0
				esp_mqtt_client_disconnect(MQTT_Client);
#endif

#if 1
				/* Stop MQTT Client */
				esp_mqtt_client_stop(MQTT_Client);
#endif

				break;

			case MQTT_EVENT_PUBLISHED :

				printf("MQTT_EVENT_PUBLISHED \n");

				/* unSubscribe From the MQTT Topic */

				esp_mqtt_client_unsubscribe(MQTT_Client, MQTT_CLIENT_SUBSCRIBE_TOPIC_NAME);


				break;

			case MQTT_EVENT_DATA :

				printf("MQTT_EVENT_DATA \n");

				/* Print the topic of the received message */

				printf("Topic : %.*s\n",MqttEventsInfo[event].topic_len,MqttEventsInfo[event].topic);

				/* Print the payload of the received message */

				printf("Data  : %.*s\n",MqttEventsInfo[event].data_len,MqttEventsInfo[event].data);

				if(msgFlag == 0)
				{
					char data[] = "Hello From : AG_ESP";

					/* Publish to the MQTT Topic */
					esp_mqtt_client_publish(MQTT_Client,MQTT_CLIENT_PUBLISH_TOPIC_NAME,data,strlen(data),2,false);

					msgFlag = 1;
				}
				else
				{
					/* Do Nothing */
				}

				break;

			case MQTT_EVENT_BEFORE_CONNECT :

				printf("MQTT_EVENT_BEFORE_CONNECT \n");

				break;

			default :

				printf("Default MQTT Event \n");

				break;
			}
		}
		else
		{
			/* Do Nothing */
		}
	}

}

esp_err_t MqttClient_Event_Handler(esp_mqtt_event_handle_t event)
{
	esp_err_t retVal = ESP_OK;

	MqttEventFlags[event->event_id] = 1;
	memcpy(&(MqttEventsInfo[event->event_id]),event,sizeof(esp_mqtt_event_t));

	return retVal;
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

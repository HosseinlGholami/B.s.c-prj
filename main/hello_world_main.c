
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_event.h"
#include "nvs_flash.h"


#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTTS_SAMPLE";


void blink_task()
{
	gpio_set_direction(2, GPIO_MODE_OUTPUT);
    while(1) {
    	gpio_set_level(2, 0);
          printf("Off\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        gpio_set_level(2, 1);
        printf("On\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void salam()
{
	while(1){
    printf("salaaam\n");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
}


esp_err_t event_handler(void *ctx, system_event_t *event){
	 switch(event->event_id) {

	    case SYSTEM_EVENT_STA_START:
	    	esp_wifi_connect();
	        break;

	    case SYSTEM_EVENT_STA_CONNECTED :
				printf("wifi vasl shod be hichi dast nazan\n");
			break;

	    case SYSTEM_EVENT_STA_DISCONNECTED:
	    		printf("\t\tghate \n\n\t alan migardam bebinam chi hast \n ");
	    			wifi_scan_config_t scf = {
	    				.ssid = NULL,.bssid = NULL,.channel = 0,.show_hidden = 1};
	    			esp_wifi_scan_start(&scf, 0);
	    	break;

	    case SYSTEM_EVENT_STA_GOT_IP :
	    		printf("aghaaaaaaaaaaa ip4 gertam , eyvallll\n");
	    		printf("Our IP address is " IPSTR "\n",
	    		IP2STR(&event->event_info.got_ip.ip_info.ip));
	    		printf("We have now connected to a station and can do things...\n");

	    	break;

case SYSTEM_EVENT_SCAN_DONE :
	printf("Number of access points found: %d\n",event->event_info.scan_done.number);
				uint16_t apCount = event->event_info.scan_done.number;
				if (apCount == 0) return ESP_OK;
wifi_ap_record_t *list =(wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));
		int i;
			for (i=0; i<apCount; i++) {
					char *authmode;
						switch(list[i].authmode) {
							case WIFI_AUTH_OPEN:
									authmode = "WIFI_AUTH_OPEN";
									break;
							case WIFI_AUTH_WEP:
									authmode = "WIFI_AUTH_WEP";
									break;
							case WIFI_AUTH_WPA_PSK:
									authmode = "WIFI_AUTH_WPA_PSK";
									break;
							case WIFI_AUTH_WPA2_PSK:
									authmode = "WIFI_AUTH_WPA2_PSK";
									break;
							case WIFI_AUTH_WPA_WPA2_PSK:
									authmode = "WIFI_AUTH_WPA_WPA2_PSK";
									break;
							default:
								authmode = "Unknown";
									break;}
				printf(" ssid=%s, rssi=%d, authmode=%s\n",
					list[i].ssid, list[i].rssi, authmode);}
						free(list);
	 printf("\t\t\t agr laye ina bashe alan vasl misham \n\n\n\n");
	 esp_wifi_connect();
break;

default:
break;}
return ESP_OK;}



static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/v1.1/messages/be0e521d1ac64760a697a3987906638b", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/v1.1/action/be0e521d1ac64760a697a3987906638b", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/v1.1/errors/be0e521d1ac64760a697a3987906638b");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}


//extern const uint8_t iot_eclipse_org_pem_start[] asm("_binary_iot_eclipse_org_pem_start");
//extern const uint8_t iot_eclipse_org_pem_end[]   asm("_binary_iot_eclipse_org_pem_end");

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
    	.host= "mqtts",
    	.uri = "api.artik.cloud",
		.port = 8883,
		.client_id ="ESP32am",
		.username="be0e521d1ac64760a697a3987906638b",
		.password="a39f30d3e29d4d6187f940da735169bb ",
		//.transport=MQTT_TRANSPORT_OVER_SSL,
		.event_handle = mqtt_event_handler,
    //    .cert_pem = 0,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}


void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();}
    ESP_ERROR_CHECK( ret );

	tcpip_adapter_init();



	esp_event_loop_init(event_handler, NULL);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_STA);
wifi_config_t staConfig = {

	.sta = {.ssid="HGHWIFI",.password="hosseingho"}

};
	esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t *)&staConfig);
	esp_wifi_start();

	mqtt_app_start();

	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(&salam, "salam", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

}

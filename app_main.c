#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
// DHT
#include <freertos/FreeRTOS.h>
#include <string.h>
#include <esp_log.h>
#include <ets_sys.h>
#include <esp_idf_lib_helpers.h>
#include <driver/gpio.h>
#include <esp_err.h>
// OLED
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include <string.h>
// ADC
#include <driver/adc.h>

#include "MQTT.h"
#include "Oled.h"
#include "dht.h"

#define DHT11_PIN 33
#define OLED_CLK 22
#define OLED_SDA 21
#define Threshold 500
const char *STATUS_TAG = "Status";
char data[100]; // Khởi tạo một mảng ký tự đủ lớn để chứa chuỗi kết quả
int adc_val;

void Publish_data(char data[]){
    int msg_id = esp_mqtt_client_publish(client, "CE232.N21.2_PUB", data, strlen(data), 0, 0);
    if (!msg_id) ESP_LOGI(MQTT_TAG, "Send publish successful\n");
    else ESP_LOGI(MQTT_TAG, "Send publish failed\n");
}

void loop(void* arg )
{
    int16_t humidity, temperature;
    while(1)
    {
        char* weather;
        dht_read_data(0, (gpio_num_t)DHT11_PIN, &humidity, &temperature);
        adc_val = adc1_get_raw(ADC1_CHANNEL_6);
        if (adc_val < Threshold) weather = "Sunny";
        else weather = "Rainy";
        ESP_LOGI(STATUS_TAG, "Temperature: %d *C", temperature);
        ESP_LOGI(STATUS_TAG, "Humidity: %d%%", humidity);
        ESP_LOGI(STATUS_TAG, "Weather: %d %s", adc_val, weather);
        sprintf(data, "Temperature:%d*C\n\nHumidity:%d%%\n\nWeather:%s", temperature, humidity, weather);
        Publish_data(data);
        task_ssd1306_display_text(data);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }   
}

void app_main(void)
{
     esp_err_t ret;
    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_11db);

    ESP_ERROR_CHECK(i2c_master_init());
    ssd1306_init();
    wifi_init();
    wifi_connect();
    mqtt_config_init();
    mqtt_start();
    task_ssd1306_display_clear();
    xTaskCreate(loop, "looptask", configMINIMAL_STACK_SIZE *8, NULL, tskIDLE_PRIORITY + 1, NULL);
}


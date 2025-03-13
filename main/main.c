/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "tcp_client_v4.c"

#include "mysht40.h"


void vTaskData( void * pvParameters )
{
    char tcp_buffer[20];
    sht40data_t sht40_data;

    sht40_init();
    my_read_id();

    while (1)
    {
        sht40_data = sht40_data_read();

        if(sht40_data.data_flag == DATA_FLAG_ERROR)
        {
            continue;
        }

		memset(tcp_buffer,0x0,sizeof(tcp_buffer));
		sprintf(tcp_buffer,"%.2f %.2f end",sht40_data.temp,sht40_data.hum);
        printf("send buffer:%s\n",tcp_buffer);

        tcp_client(tcp_buffer,sizeof(tcp_buffer));
                
        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }
    
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    
    xTaskCreate( vTaskData, "mydata", 4096, NULL, 1, NULL );
}

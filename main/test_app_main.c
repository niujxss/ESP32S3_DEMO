/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "stdio.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO    6     // SCL 引脚号（GPIO6）
#define I2C_MASTER_SDA_IO    5     // SDA 引脚号（GPIO5）
#define I2C_MASTER_FREQ_HZ   100000 // I2C 时钟频率（100kHz）
#define I2C_DEVICE_ADDR  0x44  // 从机设备地址（7位）

// 配置总线参数
i2c_master_bus_config_t bus_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,   // 默认时钟源
    .i2c_port = I2C_NUM_0,               // 使用 I2C 控制器0
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .glitch_ignore_cnt = 7,              // 抗毛刺计数器
    .flags.enable_internal_pullup = true // 启用内部弱上拉！
};


i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

void unity_run_menu()
{
    printf("Hello! 开始执行 SHT40 驱动程序\n");
    

    // 创建 I2C 总线
    esp_err_t ret = i2c_new_master_bus(&bus_config, &bus_handle);
    if (ret != ESP_OK) {
        printf("I2C 总线初始化失败！错误码: %d\n", ret);
        return;
    }
 
    ESP_ERROR_CHECK(i2c_master_probe(bus_handle, I2C_DEVICE_ADDR, -1)); //检查从机是否存在

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, // 7位地址模式
        .device_address = I2C_DEVICE_ADDR,     // 设备地址
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,    // 通信速率
    };

    // 向总线添加设备
    ret = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (ret != ESP_OK) {
        printf("I2C 设备添加失败！错误码: %d\n", ret);
        i2c_del_master_bus(bus_handle);
    }

}

void my_read_id(void)
{
    uint8_t data_to_send[] = {0x89}; //读取SHT40 ID的命令
    uint8_t receive_buffer[6];
    size_t bytes_read = 6;
    uint8_t ret;


    // 创建传输命令
    ret = i2c_master_transmit(dev_handle, data_to_send, sizeof(data_to_send), -1);
    if (ret != ESP_OK) {
        printf("I2C 发送失败！错误码: %d\n", ret);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);  //等待SHT40处理，一定要等待一会


    // 接收数据
    ret = i2c_master_receive(dev_handle, receive_buffer, sizeof(receive_buffer), -1);
    if (ret == ESP_OK) {
        printf("收到传感器ID %d 字节数据: ", bytes_read);
        for (int i = 0; i < bytes_read; i++) {
            printf("%02X ", receive_buffer[i]);
        }
        printf("\n");
    }
}

void my_read_data(uint8_t * buffer)
{
    //写入指令
    uint8_t controlid = 0xFD; //读取数据指令
    uint8_t ret;

   ret = i2c_master_transmit(dev_handle,&controlid,1,-1);
       if (ret != ESP_OK) {
        printf("I2C 发送失败！错误码: %d\n", ret);
        return;
    }

    //等待数据计算
    vTaskDelay(1000 / portTICK_PERIOD_MS); //等待SHT40处理，一定要等待一会
    
    //读取数据
    ret = i2c_master_receive(dev_handle,buffer,6,-1);
     if (ret != ESP_OK) {
        printf("接收传感器数据失败\n");
     }

}

void sht40_data_read(void)
{
        uint16_t recovery_temper = 0u;
        uint16_t recovery_hum = 0u;
        float temp = 0.f;
        float hum = 0.f;
        uint8_t buffer[6];

        my_read_data(buffer);
        recovery_temper = ((uint16_t)buffer[0]<<8)|buffer[1];
        temp = -45 + 175*((float)recovery_temper/65535);

        if(temp < -40.f)
        {
            printf("湿度异常 %.2f\n",temp);
            return;
        }
 
        /* 转换湿度数据 */
        recovery_hum = ((uint16_t)buffer[3]<<8)|buffer[4];
        hum = -6 + 125 * ((float)recovery_hum / 65535);
		if(hum >= 100)   //根据数据手册编写
		{		
            hum = 100;
        }
		else if(hum <= 0)
        {
            hum = 0;
        }
				
		printf("温度为：%.2f 湿度为：%.2f end\n",temp,hum);
}

void app_main(void)
{
    unity_run_menu();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    my_read_id();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("开始测量温湿度！！\n");
     while (1)
     {
        sht40_data_read();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
     }
    
}


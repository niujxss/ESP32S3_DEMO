#ifndef MYSHT40_H
#define MYSHT40_H

#include "stdio.h"
#include "driver/i2c_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


#define DATA_FLAG_OK 0
#define DATA_FLAG_ERROR 1

typedef struct
{
    float temp; //温度
    float hum;  // 湿度
    uint8_t data_flag;//数据是否有效
}sht40data_t;





void sht40_init();
void my_read_id(void);
sht40data_t sht40_data_read(void);


#endif

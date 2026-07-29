/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-08-26
 * @brief       EEPORM实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "led.h"
#include "iic.h"
#include "xl9555.h"
#include "24cxx.h"
#include "freertos/queue.h"
#include "usart.h"


i2c_obj_t i2c0_master;

const uint8_t g_text_buf[] = {"ESP32-S3 EEPROM"};   /* 要写入到24c02的字符串数组 */
#define TEXT_SIZE   sizeof(g_text_buf)              /* TEXT字符串长度 */

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    uint16_t i = 0;
    uint8_t err = 0;
    uint8_t key;
    uint8_t datatemp[TEXT_SIZE];
    esp_err_t ret;
    uint8_t len = 0;
    uint16_t times = 0;
    unsigned char data[RX_BUF_SIZE] = {0};
    
    ret = nvs_flash_init();             /* 初始化NVS */

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    led_init();
	 LED(1);                          /* 初始化LED */
	    i2c0_master = iic_init(I2C_NUM_0);  /* 初始化IIC0 */
		    xl9555_init(i2c0_master);           /* 初始化XL9555 */


	usart_init(115200);
   

    while(1)
    {
        key = xl9555_key_scan(0);

        switch (key)
        {
            case KEY2_PRES:     /* KEY2被按下 */
            {
                LED_TOGGLE();   /* LED状态翻转 */
                break;
            }
			 case KEY1_PRES:     /* KEY1被按下 */
            {
                printf("hello esp32-s3!\n");
                break;
            }
            default:
            {
                break;
            }
        }
       uart_get_buffered_data_len(USART_UX, (size_t*) &len);                           /* 获取环形缓冲区数据长度 */

        if (len > 0)                                                                    /* 判断数据长度 */
        {
            memset(data, 0, RX_BUF_SIZE);                                               /* 对缓冲区清零 */
            printf("\n您发送的消息为:\n");
            uart_read_bytes(USART_UX, data, len, 100);                                  /* 读数据 */
            uart_write_bytes(USART_UX, (const char*)data, strlen((const char*)data));   /* 写数据 */
			  /* ========== LED 连闪 5 次，间隔 200ms ========== */
             for (int i = 0; i < 5; i++)
    {
        LED(1);                          /* LED 亮 */
        vTaskDelay(200 / portTICK_PERIOD_MS); /* 延时 200ms */
        LED(0);                          /* LED 灭 */
        vTaskDelay(200 / portTICK_PERIOD_MS); /* 延时 200ms */
        }
	}
        
        
        vTaskDelay(10);
    }
}

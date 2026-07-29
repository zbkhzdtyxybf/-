#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "W25Q64.h"
#include "Key.h"
#include "LED.h"
#include "Serial.h"
#include "MPU6050.h"
#include "Servo.h"

// ========== 全局变量 ==========
volatile uint8_t RxBuffer[256];
volatile uint8_t RxCount = 0;
volatile uint8_t RxComplete = 0;
volatile uint8_t RxActive = 0;

uint8_t KeyNum;
float Angle = 0;
volatile uint8_t OLED_Page = 0;              // OLED显示页面 0/1

uint8_t ID;
int16_t AX, AY, AZ, GX, GY, GZ;

// Flash存储地址
#define FLASH_DATA_ADDR     0x000000
#define FLASH_LEN_ADDR      0x000000
#define FLASH_DATA_START    0x000002

// ========== 函数声明 ==========
void Flash_SaveData(uint8_t *data, uint16_t len);
void Flash_ReadAndSend(void);
void Show_Page0(void);
void Show_Page1(void);

// ========== Flash保存数据 ==========
void Flash_SaveData(uint8_t *data, uint16_t len)
{
    W25Q64_SectorErase(FLASH_DATA_ADDR);
    
    uint8_t len_buf[2];
    len_buf[0] = (len >> 8) & 0xFF;
    len_buf[1] = len & 0xFF;
    W25Q64_PageProgram(FLASH_DATA_ADDR, len_buf, 2);
    
    uint32_t addr = FLASH_DATA_START;
    uint16_t remaining = len;
    uint16_t offset = 0;
    
    while (remaining > 0) {
        uint16_t page_len = (remaining > 256) ? 256 : remaining;
        W25Q64_PageProgram(addr, &data[offset], page_len);
        addr += page_len;
        offset += page_len;
        remaining -= page_len;
    }
}

// ========== Flash读取并发送 ==========
void Flash_ReadAndSend(void)
{
    uint8_t len_buf[2];
    W25Q64_ReadData(FLASH_LEN_ADDR, len_buf, 2);
    uint16_t data_len = (len_buf[0] << 8) | len_buf[1];
    
    if (data_len == 0 || data_len > 256) {
        if (OLED_Page == 0) {
            OLED_ShowString(4, 1, "No Data!  ");
        }
        return;
    }
    
    uint8_t read_buf[256];
    W25Q64_ReadData(FLASH_DATA_START, read_buf, data_len);
    
    LED2_ON();      // 红灯亮：发送中
    Serial_SendArray(read_buf, data_len);
    LED2_OFF();     // 红灯灭
    
    if (OLED_Page == 0) {
        OLED_ShowString(4, 1, "Send:      ");
        for (uint16_t i = 0; i < data_len && i < 16; i++) {
            OLED_ShowChar(4, 6 + i, read_buf[i]);
        }
    }
}

// ========== 页面0：串口/Flash ==========
void Show_Page0(void)
{
    OLED_ShowString(1, 1, "UART/Flash  ");
    OLED_ShowString(2, 1, "Wait data...");
    // 第3行显示接收的数据
    // 第4行显示状态
}

// ========== 页面1：MPU6050 + 舵机整合 ==========
void Show_Page1(void)
{
    // 读取MPU6050数据
    MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
    
    // 第1行：角度 + AX
    OLED_ShowString(1, 1, "A:");
    OLED_ShowNum(1, 3, (uint16_t)Angle, 3);
    OLED_ShowString(1, 7, "AX:");
    OLED_ShowSignedNum(1, 10, AX, 5);
    
    // 第2行：AY + GX
    OLED_ShowString(2, 1, "AY:");
    OLED_ShowSignedNum(2, 4, AY, 5);
    OLED_ShowString(2, 10, "GX:");
    OLED_ShowSignedNum(2, 13, GX, 3);
    
    // 第3行：AZ + GY
    OLED_ShowString(3, 1, "AZ:");
    OLED_ShowSignedNum(3, 4, AZ, 5);
    OLED_ShowString(3, 10, "GY:");
    OLED_ShowSignedNum(3, 13, GY, 3);
    
    // 第4行：GZ + 提示
    OLED_ShowString(4, 1, "GZ:");
    OLED_ShowSignedNum(4, 4, GZ, 5);
    OLED_ShowString(4, 10, "K1:+30");
}

// ========== 主函数 ==========
int main(void)
{
    // 初始化所有模块
    OLED_Init();
    W25Q64_Init();
    Serial_Init();
    Key_Init();
    LED_Init();
    MPU6050_Init();
    Servo_Init();
    
    // 验证Flash
    uint8_t MID;
    uint16_t DID;
    W25Q64_ReadID(&MID, &DID);
    
    // 验证MPU6050
    ID = MPU6050_GetID();
    
    // 检查MPU6050是否在线
    if (ID != 0x68) {
        OLED_ShowString(1, 1, "MPU FAIL!   ");
        OLED_ShowHexNum(1, 12, ID, 2);
        Delay_ms(2000);
    }
    
    // 初始显示页面0
    OLED_Clear();
    Show_Page0();
    
    while (1)
    {
        KeyNum = Key_GetNum();
        
        // 按键3：PB15 切换页面
        if (KeyNum == 3) {
            OLED_Page++;
            if (OLED_Page > 1) OLED_Page = 0;
            OLED_Clear();
        }
        
        // 根据页面显示内容
        if (OLED_Page == 0) {
            // 页面0：串口接收处理
            
            // 串口接收完成
            if (Serial_RxFlag == 1) {
                Serial_RxFlag = 0;
                
                // 绿灯闪烁
                LED1_ON();
                Delay_ms(100);
                LED1_OFF();
                
                // 获取数据长度
                uint16_t data_len = 0;
                while (Serial_RxPacket[data_len] != '\0') {
                    data_len++;
                }
                
                // 保存到Flash
                Flash_SaveData((uint8_t *)Serial_RxPacket, data_len);
                
                // 显示
                OLED_ShowString(3, 1, "Rx:        ");
                for (uint8_t i = 0; i < data_len && i < 10; i++) {
                    OLED_ShowChar(3, 4 + i, Serial_RxPacket[i]);
                }
                OLED_ShowString(4, 1, "Saved!     ");
            }
            
            // 按键2：PB14 读取Flash并发送
            if (KeyNum == 2) {
                Flash_ReadAndSend();
            }
        }
        else if (OLED_Page == 1) {
            // 页面1：MPU6050 + 舵机整合
            
            // 按键1：PB1 舵机角度+30
            if (KeyNum == 1) {
                Angle += 30;
                if (Angle > 180) Angle = 0;
                Servo_SetAngle(Angle);
            }
            
            // 显示整合页面
            Show_Page1();
        }
        
        Delay_ms(100);  // 刷新间隔
    }
}

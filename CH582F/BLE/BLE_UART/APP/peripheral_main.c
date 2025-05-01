/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.1
 * Date               : 2024/08/10
 * Description        : P14項目主程式，包含試片插入偵測與類型判別功能
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

/******************************************************************************/
/* 頭文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "app_uart.h"
#include "P14_Init.h"
#include "P14_BLE_Protocol.h"  // 添加藍牙協議頭文件

/* 外部函數聲明 */
uint32_t millis(void);  // 獲取系統時間(毫秒)

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/* 全局變量 */
static volatile uint32_t g_systemTicks = 0;  // 系統時間計數(毫秒)
static volatile uint32_t g_lastStripCheckTime = 0;

/* 函數宣告 */
void BLE_Protocol_Init(void);
static void RegisterBLECallback(void);
void P14_StripDetection_Init(void);
void P14_StripDetection_Process(void);
void SysTick_Init(void);

/**
 * @brief 獲取系統時間(毫秒)
 * 
 * @return uint32_t 系統運行時間(毫秒)
 */
uint32_t millis(void)
{
    return g_systemTicks;
}

/**
 * @brief SysTick中斷處理函數
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void SysTick_Handler(void)
{
    g_systemTicks++;
    
    /* 清除中斷標誌 */
    SysTick->SR = 0;
}

/**
 * @brief 初始化SysTick定時器
 */
void SysTick_Init(void)
{
    /* 關閉SysTick定時器 */
    SysTick->CTLR = 0;
    
    /* 清除計數器和中斷標誌 */
    SysTick->CNT = 0;
    SysTick->SR = 0;
    
    /* 設置重載值為1ms中斷 */
    SysTick->CMP = FREQ_SYS / 1000;  // 使用CH58x中定義的系統頻率
    
    /* 使能SysTick中斷 */
    PFIC_EnableIRQ(SysTick_IRQn);
    
    /* 使能SysTick定時器：使用系統時鐘、自動重載、使能中斷、使能計數器 */
    SysTick->CTLR = SysTick_CTLR_INIT | SysTick_CTLR_STRE | SysTick_CTLR_STCLK | SysTick_CTLR_STIE | SysTick_CTLR_STE;
    
    PRINT("SysTick Timer Initialized at 1ms interval\r\n");
}

/**
 * @brief 註冊藍牙回調函數
 */
static void RegisterBLECallback(void)
{
    // 註冊藍牙UART回調函數
    ble_uart_set_callback(P14_BLE_UartCallback);
}

/**
 * @brief 初始化藍牙協議處理模塊
 */
void BLE_Protocol_Init(void)
{
    // 初始化藍牙協議狀態機
    // 避免遞迴調用
    P14_BLE_ProtocolInit();
}

/**
 * @brief 初始化試片偵測功能
 */
void P14_StripDetection_Init(void)
{
    /* 初始化試片插入偵測 */
    P14_StripDetectInit();
    
    /* 初始化計時器 */
    g_lastStripCheckTime = 0;
    
    PRINT("Strip Detection Initialized\r\n");
}

/**
 * @brief 試片插入處理流程
 */
void P14_StripDetection_Process(void)
{
    uint32_t currentTime = millis();
    static uint8_t processingFlag = 0;  // 正在處理試片偵測的標誌
    static uint32_t lastFailTime = 0;   // 上次偵測失敗的時間
    static uint8_t failCount = 0;       // 連續失敗次數
    static uint8_t retryCount = 0;      // 重試次數
    
    /* 檢查是否已到檢查間隔時間，且未處於處理中狀態 */
    if ((currentTime - g_lastStripCheckTime >= STRIP_DETECT_INTERVAL) && !processingFlag) {
        g_lastStripCheckTime = currentTime;
        
        /* 檢查是否有試片插入 */
        if (P14_CheckStripInsertion()) {
            /* 設置處理中標誌，避免重複觸發 */
            processingFlag = 1;
            
            PRINT("試片插入偵測成功！\r\n");
            
            /* 判斷試片類型前開啟LED提示 */
            GPIOA_SetBits(GPIO_Pin_12);  // 假設PA12是LED指示燈
            
            /* 判斷試片類型 */
            StripType_TypeDef type = P14_IdentifyStripType();
            
            /* 關閉LED提示 */
            GPIOA_ResetBits(GPIO_Pin_12);
            
            /* 顯示試片類型 */
            PRINT("識別到的試片類型: ");
            switch (type) {
                case STRIP_TYPE_GLV:
                    PRINT("GLV (血糖)\r\n");
                    break;
                case STRIP_TYPE_U:
                    PRINT("U (尿酸)\r\n");
                    break;
                case STRIP_TYPE_C:
                    PRINT("C (總膽固醇)\r\n");
                    break;
                case STRIP_TYPE_TG:
                    PRINT("TG (三酸甘油脂)\r\n");
                    break;
                case STRIP_TYPE_GAV:
                    PRINT("GAV (血糖V2)\r\n");
                    break;
                default:
                    PRINT("未知\r\n");
                    break;
            }
            
            /* 檢查是否成功識別試片類型 */
            if (type != STRIP_TYPE_UNKNOWN) {
                /* 通知CH32V203試片類型 */
                P14_NotifyStripInserted(type);
                PRINT("已通知CH32V203試片類型\r\n");
                
                /* 重設處理中標誌和錯誤計數 */
                processingFlag = 0;
                failCount = 0;
                retryCount = 0;
            } else {
                /* 試片類型識別失敗 */
                lastFailTime = currentTime;
                
                /* 增加失敗計數 */
                failCount++;
                
                /* 判斷是否需要重試 */
                if (retryCount < 3) {
                    PRINT("試片類型識別失敗，第%d次重試中...\r\n", retryCount + 1);
                    retryCount++;
                    
                    /* 短暫延遲後再次嘗試 */
                    DelayMs(200);
                    
                    /* 保持處理中狀態，但允許重新識別 */
                    processingFlag = 0;
                } else {
                    /* 超過最大重試次數 */
                    PRINT("試片類型識別失敗，請重新插入試片\r\n");
                    
                    /* 重設試片狀態 */
                    P14_StripStateReset();
                    
                    /* 重設重試計數 */
                    retryCount = 0;
                    processingFlag = 0;
                    
                    /* 如果連續失敗次數過多，提示可能有硬體問題 */
                    if (failCount > 5) {
                        PRINT("警告：連續多次識別失敗，請檢查儀器和試片\r\n");
                    }
                }
            }
        }
    }
    
    /* 定期檢查是否需要清除處理中標誌 */
    if (processingFlag && (currentTime - lastFailTime > 5000)) {
        /* 5秒後自動清除處理中標誌，防止卡死 */
        processingFlag = 0;
        retryCount = 0;
        PRINT("試片識別操作超時，自動重設\r\n");
        P14_StripStateReset();
    }
    
    /* 如果有連續失敗超過10次，間隔久一點再提示可能的硬體問題 */
    if (failCount > 10 && (currentTime % 60000) < 1000) {  // 每分鐘提示一次
        PRINT("系統警告：試片識別連續失敗次數過多，請檢查設備連接\r\n");
    }
}

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : 主循環
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        app_uart_process();
        
        /* 試片插入偵測處理 */
        P14_StripDetection_Process();
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : 主函數
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    /* P14 V2.1 系統初始化 */
    P14_CH582F_System_Init();
    
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    PRINT("P14 V2.1 System Starting...\r\n");
    
    /* 初始化SysTick定時器 */
    SysTick_Init();
    
    /* BLE相關初始化 */
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    /* 初始化P14藍牙協議 */
    BLE_Protocol_Init();
    PRINT("P14 BLE Protocol Initialized\r\n");
    
    /* 初始化UART */
    app_uart_init();
    PRINT("UART Initialized\r\n");
    
    /* 初始化試片偵測 */
    P14_StripDetection_Init();
    
    /* 註冊BLE回調函數 */
    RegisterBLECallback();
    
    PRINT("P14 V2.1 System Ready\r\n");
    
    /* 進入主循環 */
    Main_Circulation();
}

/******************************** endfile @ main ******************************/

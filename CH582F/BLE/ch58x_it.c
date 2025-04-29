/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch58x_it.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2023/05/15
 * Description        : CH582F中斷處理程式
 *********************************************************************************
 * Copyright (c) 2023 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* 頭文件包含 */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"
#include "ble_uart_protocol.h"

/* 全局變數 */
uint8_t uart1RxBuffer[256];
volatile uint8_t uart1RxCount = 0;

/* 防彈跳相關變數 */
#define DEBOUNCE_TIME 20    // 20ms防彈跳時間
volatile uint8_t stripDetect3Flag = 0;  // 試片3腳檢測旗標
volatile uint8_t stripDetect5Flag = 0;  // 試片5腳檢測旗標
volatile uint8_t stripDebounceInProgress = 0;  // 防彈跳處理進行中旗標

/* 試片檢測相關變數 */
typedef struct {
    uint8_t pin3Status;     // 第3腳狀態 (0=High, 1=Low)
    uint8_t pin5Status;     // 第5腳狀態 (0=High, 1=Low)
} StripDetectStatus_t;

StripDetectStatus_t stripStatus = {0, 0};

/* 試片類型定義 */
typedef enum {
    PROTOCOL_STRIP_UNKNOWN = 0,   // 未知類型
    PROTOCOL_STRIP_GLV,           // 血糖(GLV試片)
    PROTOCOL_STRIP_U,             // 尿酸
    PROTOCOL_STRIP_C,             // 總膽固醇
    PROTOCOL_STRIP_TG,            // 三酸甘油脂
    PROTOCOL_STRIP_GAV            // 血糖(GAV試片)
} StripType_t;

/* 函數聲明 */
void StripDetectProcess(void);
void SendStripDetectedMessage(uint8_t stripType);

/******************************************************************************/
/* 中斷處理函數 */

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中斷處理函數 - 處理與CH32V203的通訊
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void UART1_IRQHandler(void)
{
    uint8_t rxData;
    
    /* 接收中斷 */
    if(UART1_GetITFlag(RB_IER_RECV_RDY))
    {
        /* 清除中斷標誌 */
        UART1_ClearITFlag(RB_IER_RECV_RDY);
        
        /* 讀取接收數據 */
        rxData = UART1_RecvByte();
        
        /* 儲存接收數據到緩衝區 */
        if(uart1RxCount < sizeof(uart1RxBuffer))
        {
            uart1RxBuffer[uart1RxCount++] = rxData;
        }
    }
}

/*********************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO中斷處理函數 - 處理試片插入檢測
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void GPIO_IRQHandler(void)
{
    /* 判斷是否為試片插入檢測引腳中斷 - PB11 (Strip_Detect_3) */
    if(GPIOB_ReadITFlagBit(GPIO_Pin_11))
    {
        /* 清除中斷標誌 */
        GPIOB_ClearITFlagBit(GPIO_Pin_11);
        
        /* 設置第3腳檢測旗標 */
        stripDetect3Flag = 1;
        
        /* 啟動防彈跳處理 */
        if (!stripDebounceInProgress) {
            stripDebounceInProgress = 1;
            TMR0_TimerInit(FCLK/1000*DEBOUNCE_TIME);  // 設置定時器為DEBOUNCE_TIME毫秒
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);    // 啟用定時器結束中斷
            PFIC_EnableIRQ(TMR0_IRQn);                // 啟用定時器中斷
        }
    }
    
    /* 判斷是否為試片插入檢測引腳中斷 - PA15 (Strip_Detect_5) */
    if(GPIOA_ReadITFlagBit(GPIO_Pin_15))
    {
        /* 清除中斷標誌 */
        GPIOA_ClearITFlagBit(GPIO_Pin_15);
        
        /* 設置第5腳檢測旗標 */
        stripDetect5Flag = 1;
        
        /* 啟動防彈跳處理 */
        if (!stripDebounceInProgress) {
            stripDebounceInProgress = 1;
            TMR0_TimerInit(FCLK/1000*DEBOUNCE_TIME);  // 設置定時器為DEBOUNCE_TIME毫秒
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);    // 啟用定時器結束中斷
            PFIC_EnableIRQ(TMR0_IRQn);                // 啟用定時器中斷
        }
    }
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   定時器0中斷處理函數 - 用於試片插入防彈跳處理
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void TMR0_IRQHandler(void)
{
    /* 清除中斷標誌 */
    TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
    
    /* 停用定時器 */
    TMR0_ITCfg(DISABLE, TMR0_3_IT_CYC_END);
    PFIC_DisableIRQ(TMR0_IRQn);
    
    /* 防彈跳時間結束，處理試片檢測 */
    stripDebounceInProgress = 0;
    
    /* 如果有檢測旗標被設置，執行試片檢測處理 */
    if (stripDetect3Flag || stripDetect5Flag) {
        StripDetectProcess();
        
        /* 清除檢測旗標 */
        stripDetect3Flag = 0;
        stripDetect5Flag = 0;
    }
}

/*********************************************************************
 * @fn      StripDetectProcess
 *
 * @brief   試片檢測處理函數 - 檢測試片類型並通知CH32V203
 *
 * @return  none
 */
void StripDetectProcess(void)
{
    uint8_t stripType = PROTOCOL_STRIP_UNKNOWN;
    
    /* 讀取Strip_Detect_3狀態 */
    if (GPIOB_ReadPortPin(GPIO_Pin_11)) {
        stripStatus.pin3Status = 0;  // 高電平
    } else {
        stripStatus.pin3Status = 1;  // 低電平
    }
    
    /* 讀取Strip_Detect_5狀態 */
    if (GPIOA_ReadPortPin(GPIO_Pin_15)) {
        stripStatus.pin5Status = 0;  // 高電平
    } else {
        stripStatus.pin5Status = 1;  // 低電平
    }
    
    /* 啟用MCU上的T1量測電路，用於後續的T1_OUT電壓測量 */
    /* 發送命令給CH32V203開始T1量測 */
    uint8_t cmd_data[1] = {0x01};  // 0x01表示開始T1量測
    BLE_Protocol_SendFrame(0x10, cmd_data, 1);  // 使用命令ID 0x10指示開始T1量測
    
    /* 等待CH32V203返回T1量測結果 */
    /* 注意：實際應用中應使用異步處理或定時器，這裡簡化為同步等待 */
    Delay_Ms(10);
    
    /* 根據試片類型發送通知 */
    /* 注意：這裡假設已經收到了T1_OUT測量結果，實際中需要在回應處理中完成 */
    /* 此處僅依據腳位初步判斷試片類型，完整判斷需結合T1_OUT電壓值 */
    
    if (stripStatus.pin3Status == 1 && stripStatus.pin5Status == 0) {
        /* 可能是GLV或U型試片，需結合T1_OUT電壓判斷 */
        stripType = PROTOCOL_STRIP_GLV;  // 暫定為GLV型
    } else if (stripStatus.pin3Status == 1 && stripStatus.pin5Status == 1) {
        stripType = PROTOCOL_STRIP_C;  // 總膽固醇
    } else if (stripStatus.pin3Status == 0 && stripStatus.pin5Status == 1) {
        stripType = PROTOCOL_STRIP_TG;  // 三酸甘油脂
    } else if (stripStatus.pin3Status == 0 && stripStatus.pin5Status == 0) {
        stripType = PROTOCOL_STRIP_GAV;  // GAV型血糖
    }
    
    /* 發送試片類型通知 */
    SendStripDetectedMessage(stripType);
}

/*********************************************************************
 * @fn      SendStripDetectedMessage
 *
 * @brief   發送試片檢測通知給CH32V203
 *
 * @param   stripType - 檢測到的試片類型
 *
 * @return  none
 */
void SendStripDetectedMessage(uint8_t stripType)
{
    uint8_t data[2];
    
    /* 準備試片檢測數據 */
    data[0] = stripType;                   // 試片類型
    data[1] = (stripStatus.pin3Status << 1) | stripStatus.pin5Status; // 腳位狀態
    
    /* 發送試片檢測通知 */
    BLE_Protocol_SendFrame(0x11, data, 2);  // 使用命令ID 0x11指示試片類型通知
    
    /* 如果是已知類型，啟用相應的測量電路 */
    if (stripType == PROTOCOL_STRIP_GAV) {
        /* 對於GAV試片，啟用T3電極量測 */
        GPIOB_SetBits(GPIO_Pin_10);  // T3_IN_SEL引腳拉高
    }
} 
/********************************** (C) COPYRIGHT *******************************
 * File Name          : StripDetect.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/10
 * Description        : P14項目試片插入偵測與類型判別模組
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "CH58x_common.h"
#include "P14_Init.h"

/* 外部函數聲明 */
uint32_t millis(void);  // 獲取系統時間(毫秒)

/* 全局變數 */
static volatile StripState_TypeDef g_stripState = STRIP_STATE_NONE;
static volatile uint32_t g_stripInsertTime = 0;
static volatile uint8_t g_stripDetectFlag = 0;
static volatile StripType_TypeDef g_stripType = STRIP_TYPE_UNKNOWN;

/* 用於向CH32V203發送命令的緩存 */
static uint8_t g_cmdBuffer[8];

    /* UART接收緩衝區 */
static volatile uint8_t g_rxBuffer[16];
static volatile uint8_t g_rxIndex = 0;
static volatile uint8_t g_rxComplete = 0;
static volatile uint32_t g_lastRxTime = 0;
#define UART_TIMEOUT_MS  100   // UART接收超時時間

/*********************************************************************
 * @fn      P14_StripDetectInit
 *
 * @brief   初始化試片偵測功能
 *
 * @param   none
 *
 * @return  none
 */
void P14_StripDetectInit(void)
{
    /* 初始化中斷PIN */
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PU);     // Strip_Detect_3, 上拉輸入
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);     // Strip_Detect_5, 上拉輸入
    
    /* 設定為下降沿觸發中斷 */
    GPIOB_ITModeCfg(GPIO_Pin_11, GPIO_ITMode_FallEdge);
    GPIOA_ITModeCfg(GPIO_Pin_15, GPIO_ITMode_FallEdge);
    
    /* 啟用GPIO中斷 */
    PFIC_EnableIRQ(GPIO_B_IRQn);
    PFIC_EnableIRQ(GPIO_A_IRQn);
    
    /* 初始化狀態 */
    g_stripState = STRIP_STATE_NONE;
    g_stripDetectFlag = 0;
    g_stripType = STRIP_TYPE_UNKNOWN;
    g_rxIndex = 0;
    g_rxComplete = 0;
}

/*********************************************************************
 * @fn      P14_CheckStripInsertion
 *
 * @brief   檢查試片插入狀態，處理防彈跳
 *
 * @param   none
 *
 * @return  1: 試片確認插入，需進行類型判別
 *          0: 試片未完成插入流程
 */
uint8_t P14_CheckStripInsertion(void)
{
    uint8_t stripDetected = 0;
    uint32_t currentTime = millis();
    
    /* 檢查是否有試片插入事件發生 */
    if (g_stripDetectFlag) {
        switch (g_stripState) {
            case STRIP_STATE_NONE:
                /* 進入插入防彈跳階段 */
                g_stripState = STRIP_STATE_INSERTING;
                g_stripInsertTime = currentTime;
                break;
                
            case STRIP_STATE_INSERTING:
                /* 防彈跳時間檢查 */
                if (currentTime - g_stripInsertTime > STRIP_DEBOUNCE_TIME) {
                    /* 再次確認試片確實插入（通過Strip_Detect_3或Strip_Detect_5的電平） */
                    if ((!(GPIOB_ReadPortPin(GPIO_Pin_11)) || !(GPIOA_ReadPortPin(GPIO_Pin_15)))) {
                        g_stripState = STRIP_STATE_INSERTED;
                        stripDetected = 1;
                    } else {
                        /* 誤觸發，恢復為無試片狀態 */
                        g_stripState = STRIP_STATE_NONE;
                    }
                    g_stripDetectFlag = 0;
                }
                break;
                
            default:
                g_stripDetectFlag = 0;
                break;
        }
    }
    
    return stripDetected;
}

/*********************************************************************
 * @fn      P14_UART1_RxHandler
 *
 * @brief   UART1接收資料處理
 *
 * @param   rx_data - 接收的資料
 *
 * @return  none
 */
void P14_UART1_RxHandler(uint8_t rx_data)
{
    /* 更新接收時間戳 */
    g_lastRxTime = millis();
    
    /* 開始標記檢查 */
    if (g_rxIndex == 0 && rx_data == 0xAA) {
        g_rxBuffer[g_rxIndex++] = rx_data;
    }
    /* 資料接收 */
    else if (g_rxIndex > 0) {
        /* 防止緩衝區溢出 */
        if (g_rxIndex < sizeof(g_rxBuffer)) {
            g_rxBuffer[g_rxIndex++] = rx_data;
        }
        
        /* 檢查結束標記 */
        if (rx_data == 0xBB && g_rxIndex >= 4) {
            g_rxComplete = 1;
        }
    }
}

/*********************************************************************
 * @fn      P14_UART1_CheckTimeout
 *
 * @brief   檢查UART1接收超時
 *
 * @param   none
 *
 * @return  none
 */
void P14_UART1_CheckTimeout(void)
{
    /* 如果正在接收資料但已超時，重置接收緩衝區 */
    uint32_t currentTime = millis();
    if (g_rxIndex > 0 && !g_rxComplete && 
        (currentTime - g_lastRxTime > UART_TIMEOUT_MS)) {
        g_rxIndex = 0;
    }
}

/*********************************************************************
 * @fn      P14_IdentifyStripType
 *
 * @brief   判斷試片類型
 *
 * @param   none
 *
 * @return  StripType_TypeDef: 試片類型
 */
StripType_TypeDef P14_IdentifyStripType(void)
{
    StripType_TypeDef type = STRIP_TYPE_UNKNOWN;
    uint16_t t1_out_value = 0;
    uint8_t t1_out_near_2p5v = 0;
    uint8_t retry_count = 0;
    uint32_t timeout_start = 0;
    
    /* 讀取Pin3和Pin5的狀態 */
    uint8_t pin3_state = GPIOB_ReadPortPin(GPIO_Pin_11) ? 1 : 0;  // Strip_Detect_3
    uint8_t pin5_state = GPIOA_ReadPortPin(GPIO_Pin_15) ? 1 : 0;  // Strip_Detect_5
    
    /* 重設接收狀態 */
    g_rxIndex = 0;
    g_rxComplete = 0;
    
    /* 嘗試最多3次獲取T1_OUT電壓 */
    while (retry_count < 3) {
        /* 發送命令給CH32V203進行T1_OUT電壓測量 */
        g_cmdBuffer[0] = 0xAA;  // 命令開始標記
        g_cmdBuffer[1] = 0x01;  // 命令類型：測量T1_OUT
        g_cmdBuffer[2] = 0xBB;  // 命令結束標記
        
        /* 發送命令 */
        for (uint8_t i = 0; i < 3; i++) {
            UART1_SendByte(g_cmdBuffer[i]);
        }
        
        /* 等待回應，使用超時機制 */
        timeout_start = millis();
        while (!g_rxComplete) {
            /* 檢查超時 */
            if (millis() - timeout_start > 200) {
                break;
            }
            
            /* 檢查接收超時 */
            P14_UART1_CheckTimeout();
            
            /* 短暫延遲 */
            DelayMs(1);
        }
        
        /* 檢查是否收到完整回應 */
        if (g_rxComplete && g_rxIndex >= 5) {
            /* 檢查回應格式 [0xAA][CMD][ADC_VALUE_H][ADC_VALUE_L][0xBB] */
            if (g_rxBuffer[0] == 0xAA && g_rxBuffer[1] == 0x01 && g_rxBuffer[4] == 0xBB) {
                /* 解析ADC值 */
                t1_out_value = ((uint16_t)g_rxBuffer[2] << 8) | g_rxBuffer[3];
                
                /* 重設接收狀態 */
                g_rxIndex = 0;
                g_rxComplete = 0;
                
                /* 成功獲取數據，跳出循環 */
                break;
            }
        }
        
        /* 重設接收狀態 */
        g_rxIndex = 0;
        g_rxComplete = 0;
        
        /* 增加重試計數器 */
        retry_count++;
        
        /* 重試間隔 */
        DelayMs(50);
    }
    
    /* 判斷T1_OUT電壓是否接近2.5V (大約3000/4096 * 3.3V = 2.4V) */
    t1_out_near_2p5v = (t1_out_value > 3000) ? 1 : 0;
    
    /* 根據試片類型判別機制文件進行類型判斷 */
    if (pin3_state == 0 && pin5_state == 1 && t1_out_near_2p5v) {
        type = STRIP_TYPE_GLV;     // 血糖(GLV試片)
    } else if (pin3_state == 0 && pin5_state == 1 && !t1_out_near_2p5v) {
        type = STRIP_TYPE_U;       // 尿酸
    } else if (pin3_state == 0 && pin5_state == 0 && t1_out_near_2p5v) {
        type = STRIP_TYPE_C;       // 總膽固醇
    } else if (pin3_state == 1 && pin5_state == 0 && !t1_out_near_2p5v) {
        type = STRIP_TYPE_TG;      // 三酸甘油酯
    } else if (pin3_state == 1 && pin5_state == 0 && t1_out_near_2p5v) {
        type = STRIP_TYPE_GAV;     // 血糖(GAV試片)
    } else {
        type = STRIP_TYPE_UNKNOWN; // 未知類型
    }
    
    /* 儲存檢測到的試片類型 */
    g_stripType = type;
    g_stripState = STRIP_STATE_READY;
    
    return type;
}

/*********************************************************************
 * @fn      P14_NotifyStripInserted
 *
 * @brief   通知CH32V203已檢測到試片並已判別類型
 *
 * @param   type - 試片類型
 *
 * @return  none
 */
void P14_NotifyStripInserted(StripType_TypeDef type)
{
    /* 構建通知命令 */
    g_cmdBuffer[0] = 0xAA;           // 命令開始標記
    g_cmdBuffer[1] = 0x02;           // 命令類型：試片插入通知
    g_cmdBuffer[2] = (uint8_t)type;  // 試片類型
    g_cmdBuffer[3] = 0xBB;           // 命令結束標記
    
    /* 發送命令 */
    for (uint8_t i = 0; i < 4; i++) {
        UART1_SendByte(g_cmdBuffer[i]);
    }
}

/*********************************************************************
 * @fn      P14_StripStateReset
 *
 * @brief   重設試片狀態
 *
 * @param   none
 *
 * @return  none
 */
void P14_StripStateReset(void)
{
    g_stripState = STRIP_STATE_NONE;
    g_stripDetectFlag = 0;
    g_stripType = STRIP_TYPE_UNKNOWN;
}

/*********************************************************************
 * @fn      GPIOB_IRQHandler
 *
 * @brief   GPIOB中斷處理函數，用於Strip_Detect_3的檢測
 *
 * @param   none
 *
 * @return  none
 */
__INTERRUPT void GPIOB_IRQHandler(void)
{
    /* 檢查是否為Strip_Detect_3(PB11)觸發 */
    if ((R16_PB_INT_IF & GPIO_Pin_11) && (g_stripState == STRIP_STATE_NONE)) {
        g_stripDetectFlag = 1;
    }
    
    /* 清除中斷標誌 */
    R16_PB_INT_IF = GPIO_Pin_11;
}

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA中斷處理函數，用於Strip_Detect_5的檢測
 *
 * @param   none
 *
 * @return  none
 */
__INTERRUPT void GPIOA_IRQHandler(void)
{
    /* 檢查是否為Strip_Detect_5(PA15)觸發 */
    if ((R16_PA_INT_IF & GPIO_Pin_15) && (g_stripState == STRIP_STATE_NONE)) {
        g_stripDetectFlag = 1;
    }
    
    /* 清除中斷標誌 */
    R16_PA_INT_IF = GPIO_Pin_15;
} 
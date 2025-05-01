/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_BLE_Protocol.c
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/10
 * Description        : P14項目CH582F藍牙通訊協議實現
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#include "CH58x_common.h"
#include "P14_BLE_Protocol.h"
#include "P14_Init.h"

/* 全局變數 */
static uint8_t g_rxBuffer[32];
static uint8_t g_rxIndex = 0;
static uint8_t g_cmdState = 0;  // 0: 等待命令開始, 1: 等待命令類型, 2: 等待數據, 3: 等待結束

/* ADC測量函數 - 此處為模擬實現 */
static uint16_t Get_ADC_Value_Mock(void) {
    /* 在實際應用中，此函數應該使用CH582F的ADC模組來測量實際電壓 */
    /* 這裡僅返回一個模擬值用於測試 */
    return 3500; // 模擬接近3.5V的ADC值
}

/*********************************************************************
 * @fn      P14_BLE_ProtocolInit
 *
 * @brief   初始化藍牙協議處理
 *
 * @param   none
 *
 * @return  none
 */
void P14_BLE_ProtocolInit(void)
{
    /* 重置接收緩衝區 */
    memset(g_rxBuffer, 0, sizeof(g_rxBuffer));
    g_rxIndex = 0;
    g_cmdState = 0;
    
    PRINT("P14 BLE Protocol initialized\r\n");
}

/*********************************************************************
 * @fn      P14_BLE_UartCallback
 *
 * @brief   UART接收數據回調函數
 *
 * @param   data - 接收到的數據
 * @param   length - 數據長度
 *
 * @return  none
 */
void P14_BLE_UartCallback(uint8_t *data, uint16_t length)
{
    /* 處理從CH32V203接收到的數據 */
    if (data && length > 0) {
        for (uint16_t i = 0; i < length; i++) {
            /* 協議狀態機 */
            switch (g_cmdState) {
                case 0:  // 等待命令開始
                    if (data[i] == CMD_START_MARKER) {
                        g_rxBuffer[0] = CMD_START_MARKER;
                        g_rxIndex = 1;
                        g_cmdState = 1;
                    }
                    break;
                    
                case 1:  // 等待命令類型
                    g_rxBuffer[g_rxIndex++] = data[i];
                    
                    /* 根據命令類型決定後續處理 */
                    switch (data[i]) {
                        case CMD_MEASURE_T1_OUT:
                            /* 收到T1_OUT測量結果 */
                            g_cmdState = 2;
                            break;
                            
                        case CMD_STRIP_TYPE_RESULT:
                            /* 收到試片類型結果 */
                            g_cmdState = 2;
                            break;
                            
                        default:
                            /* 未知命令類型，重置狀態機 */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                            break;
                    }
                    break;
                    
                case 2:  // 等待數據(ADC高字節或結果代碼)
                    g_rxBuffer[g_rxIndex++] = data[i];
                    g_cmdState = 3;
                    break;
                    
                case 3:  // 等待下一個數據或結束標記
                    g_rxBuffer[g_rxIndex++] = data[i];
                    
                    /* 根據上一個命令類型判斷 */
                    if (g_rxBuffer[1] == CMD_MEASURE_T1_OUT) {
                        /* T1_OUT測量結果需要4個字節的命令: [0xAA][0x01][HIGH][LOW][0xBB] */
                        if (g_rxIndex >= 4 && data[i] == CMD_END_MARKER) {
                            /* 處理完整的T1_OUT測量結果 */
                            uint16_t adcValue = (uint16_t)g_rxBuffer[2] << 8 | g_rxBuffer[3];
                            PRINT("T1_OUT ADC值: %d\r\n", adcValue);
                            
                            /* 重置狀態機 */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        } else if (g_rxIndex > 5) {
                            /* 數據太長，可能是錯誤的命令，重置狀態機 */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        }
                    } else if (g_rxBuffer[1] == CMD_STRIP_TYPE_RESULT) {
                        /* 試片類型結果需要3個字節的命令: [0xAA][0x03][TYPE][0xBB] */
                        if (g_rxIndex >= 3 && data[i] == CMD_END_MARKER) {
                            /* 處理完整的試片類型結果 */
                            StripType_TypeDef type = (StripType_TypeDef)g_rxBuffer[2];
                            PRINT("接收到試片類型結果: %d\r\n", type);
                            
                            /* 重置狀態機 */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        } else if (g_rxIndex > 4) {
                            /* 數據太長，可能是錯誤的命令，重置狀態機 */
                            g_cmdState = 0;
                            g_rxIndex = 0;
                        }
                    } else {
                        /* 未知命令類型，重置狀態機 */
                        g_cmdState = 0;
                        g_rxIndex = 0;
                    }
                    break;
                    
                default:
                    /* 未知狀態，重置狀態機 */
                    g_cmdState = 0;
                    g_rxIndex = 0;
                    break;
            }
            
            /* 防止緩衝區溢出 */
            if (g_rxIndex >= sizeof(g_rxBuffer)) {
                g_cmdState = 0;
                g_rxIndex = 0;
            }
        }
    }
} 
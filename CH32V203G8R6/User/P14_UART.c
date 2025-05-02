/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_UART.c
 * Author             : HMD Team
 * Version            : V1.0
 * Date               : 2024/05/08
 * Description        : P14多功能生化儀讀數UART通訊處理
 *********************************************************************************
 * Copyright (c) 2024 HMD. All rights reserved.
 *******************************************************************************/

#include "P14_UART.h"
#include "param_storage.h"
#include <string.h>

/* 定義緩衝區 */
static uint8_t rxBuffer[UART_RX_BUFFER_SIZE];
static uint16_t rxBufferHead = 0;
static uint16_t rxBufferTail = 0;

static uint8_t txBuffer[UART_TX_BUFFER_SIZE];
static uint8_t packetBuffer[MAX_PACKET_SIZE + 6]; // 用於暫存解析後的封包

/* 協議狀態變數 */
static uint8_t currentCode = 25;            // 當前試紙校碼
static uint16_t currentEvent = EVENT_NONE;  // 當前事件類型
uint8_t bloodSampleReady = 0;        // 血液樣本準備狀態
uint8_t bloodCountdown = 0;          // 血液樣本倒計時
uint8_t sampleProcessActive = 0;     // 樣本處理狀態

/*********************************************************************
 * @fn      UART_Init
 *
 * @brief   初始化UART通訊
 *
 * @return  none
 */
void UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* USART2 TX (PA2) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2 RX (PA3) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2配置 - 波特率115200，8數據位，1停止位，無校驗位，無流控 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    /* 配置USART2接收中斷 */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 啟用接收中斷 */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    /* 啟用USART2 */
    USART_Cmd(USART2, ENABLE);

    /* 清空緩衝區 */
    rxBufferHead = 0;
    rxBufferTail = 0;

    printf("UART初始化完成，波特率115200\r\n");
}

/*********************************************************************
 * @fn      UART_SendData
 *
 * @brief   通過UART發送數據
 *
 * @param   data - 待發送的數據
 * @param   len - 數據長度
 *
 * @return  none
 */
void UART_SendData(uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, data[i]);
    }
}

/*********************************************************************
 * @fn      UART_CalculateChecksum
 *
 * @brief   計算校驗和
 *
 * @param   cmdId - 命令ID
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  校驗和
 */
uint8_t UART_CalculateChecksum(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    uint16_t sum = cmdId;
    for(uint8_t i = 0; i < dataLen; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

/*********************************************************************
 * @fn      UART_SendPacket
 *
 * @brief   發送協議封包
 *
 * @param   cmdId - 命令ID
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  none
 */
void UART_SendPacket(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    int idx = 0;
    
    // 組裝封包
    txBuffer[idx++] = PROTOCOL_START_MARKER;
    txBuffer[idx++] = cmdId;
    txBuffer[idx++] = dataLen;
    
    if(dataLen > 0 && data != NULL)
    {
        memcpy(&txBuffer[idx], data, dataLen);
        idx += dataLen;
    }
    
    txBuffer[idx++] = UART_CalculateChecksum(cmdId, data, dataLen);
    txBuffer[idx++] = PROTOCOL_END_MARKER;
    
    // 發送封包
    UART_SendData(txBuffer, idx);
    
    printf("發送封包: ID=0x%02X, 長度=%d\r\n", cmdId, dataLen);
}

/*********************************************************************
 * @fn      UART_SendError
 *
 * @brief   發送錯誤響應
 *
 * @param   originalCmdId - 原始命令ID
 * @param   errorCode - 錯誤代碼
 *
 * @return  none
 */
void UART_SendError(uint8_t originalCmdId, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = originalCmdId;
    data[1] = errorCode;
    
    UART_SendPacket(CMD_ERROR_ACK, data, 2);
    
    printf("發送錯誤: 原始命令=0x%02X, 錯誤碼=0x%02X\r\n", originalCmdId, errorCode);
}

/*********************************************************************
 * @fn      UART_HandleSyncTime
 *
 * @brief   處理同步時間命令
 *
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  none
 */
void UART_HandleSyncTime(uint8_t *data, uint8_t dataLen)
{
    // 檢查數據長度
    if(dataLen != 7)
    {
        UART_SendError(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // 解析時間數據
    uint16_t year = (data[0] << 8) | data[1];
    uint8_t month = data[2];
    uint8_t date = data[3];
    uint8_t hour = data[4];
    uint8_t minute = data[5];
    uint8_t second = data[6];
    
    // 設置系統時間
    if(PARAM_UpdateDateTime(year, month, date, hour, minute, second))
    {
        printf("同步時間: 20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
               year, month, date, hour, minute, second);
        
        // 發送成功響應
        uint8_t response = 0x00;  // 成功
        UART_SendPacket(CMD_SYNC_TIME_ACK, &response, 1);
    }
    else
    {
        // 發送錯誤響應
        UART_SendError(CMD_SYNC_TIME, ERR_DATA_FORMAT);
    }
}

/*********************************************************************
 * @fn      UART_HandleDeviceStatusRequest
 *
 * @brief   處理設備狀態請求
 *
 * @return  none
 */
void UART_HandleDeviceStatusRequest(void)
{
    uint8_t data[8];
    uint16_t batteryVoltage;
    uint16_t temperature;
    uint16_t stripType;
    uint16_t stripStatus = 0; // 預設為正常
    
    // 獲取系統參數
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // 獲取當前測量參數
    stripType = basicParams.stripType;
    
    // 獲取電池電壓和溫度
    batteryVoltage = 3000; // 模擬值，實際應從ADC讀取
    temperature = 250;     // 模擬值，實際應從溫度感測器讀取
    
    // 檢查電池電壓是否過低
    if(batteryVoltage < 2600)  // 2.6V以下為電量過低
    {
        stripStatus = ERR_LOW_BATTERY;
    }
    
    // 檢查溫度是否在範圍內
    if(temperature > basicParams.tempHighLimit * 10)
    {
        stripStatus = ERR_TEMP_HIGH;
    }
    else if(temperature < basicParams.tempLowLimit * 10)
    {
        stripStatus = ERR_TEMP_LOW;
    }
    
    // 組裝數據
    data[0] = (stripType >> 8) & 0xFF;
    data[1] = stripType & 0xFF;
    data[2] = (stripStatus >> 8) & 0xFF;
    data[3] = stripStatus & 0xFF;
    data[4] = (batteryVoltage >> 8) & 0xFF;
    data[5] = batteryVoltage & 0xFF;
    data[6] = (temperature >> 8) & 0xFF;
    data[7] = temperature & 0xFF;
    
    // 發送回應
    UART_SendPacket(CMD_DEVICE_STATUS_ACK, data, 8);
    
    printf("設備狀態請求響應: 項目=%d, 狀態=%d, 電壓=%dmV, 溫度=%.1f°C\r\n",
          stripType, stripStatus, batteryVoltage, temperature / 10.0);
}

/*********************************************************************
 * @fn      UART_HandleSetCodeEvent
 *
 * @brief   處理設定CODE和EVENT命令
 *
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  none
 */
void UART_HandleSetCodeEvent(uint8_t *data, uint8_t dataLen)
{
    // 檢查數據長度
    if(dataLen != 3)
    {
        UART_SendError(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // 設置CODE和EVENT
    currentCode = data[0];
    currentEvent = (data[1] << 8) | data[2];
    
    printf("設置CODE=%d, EVENT=%d\r\n", currentCode, currentEvent);
    
    // 設置系統參數
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    basicParams.defaultEvent = currentEvent;
    PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // 響應確認
    uint8_t response = 0x00;  // 成功
    UART_SendPacket(CMD_CODE_EVENT_ACK, &response, 1);
}

/*********************************************************************
 * @fn      UART_HandleBloodSampleRequest
 *
 * @brief   處理血液樣本檢測請求
 *
 * @return  none
 */
void UART_HandleBloodSampleRequest(void)
{
    // 檢查設備狀態
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // 模擬檢測過程
    if(bloodSampleReady == 0)
    {
        // 模擬血液還未準備好
        UART_SendError(CMD_BLOOD_SAMPLE_STATUS, ERR_MEASURE_TIMEOUT);
    }
    else
    {
        // 血液準備好了，啟動倒計時
        if(!sampleProcessActive)
        {
            bloodCountdown = 5; // 5秒倒計時
            sampleProcessActive = 1;
        }
        
        // 發送倒計時響應
        UART_SendPacket(CMD_BLOOD_SAMPLE_ACK, &bloodCountdown, 1);
        
        printf("血液檢測通知: 倒計時%d秒\r\n", bloodCountdown);
        
        // 更新倒計時
        if(bloodCountdown > 0)
        {
            bloodCountdown--;
        }
        else
        {
            // 倒計時結束，準備發送結果
            sampleProcessActive = 0;
        }
    }
}

/*********************************************************************
 * @fn      UART_HandleResultRequest
 *
 * @brief   處理請求測試結果命令
 *
 * @return  none
 */
void UART_HandleResultRequest(void)
{
    uint8_t data[25];
    int idx = 0;
    
    // 檢查是否有測試結果
    if(!sampleProcessActive && bloodCountdown == 0 && bloodSampleReady)
    {
        // 獲取當前時間
        uint8_t year, month, date, hour, minute, second;
        PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second);
        
        // 獲取當前測試項目
        BasicSystemBlock basicParams;
        PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
        
        // 獲取最新測試記錄
        TestRecord testRecord;
        uint16_t recordCount = PARAM_GetTestRecordCount();
        uint16_t actualCount = 0;
        
        if(recordCount > 0)
        {
            // 讀取最新記錄
            PARAM_ReadTestRecords(&testRecord, recordCount - 1, 1, &actualCount);
            
            // 組裝數據
            data[idx++] = (testRecord.resultStatus >> 8) & 0xFF;
            data[idx++] = testRecord.resultStatus & 0xFF;
            data[idx++] = (testRecord.resultValue >> 8) & 0xFF;
            data[idx++] = testRecord.resultValue & 0xFF;
            data[idx++] = (testRecord.testType >> 8) & 0xFF;
            data[idx++] = testRecord.testType & 0xFF;
            data[idx++] = (testRecord.eventType >> 8) & 0xFF;
            data[idx++] = testRecord.eventType & 0xFF;
            data[idx++] = currentCode;  // 使用當前校碼
            
            // 時間
            data[idx++] = (testRecord.year >> 8) & 0xFF;
            data[idx++] = testRecord.year & 0xFF;
            data[idx++] = (testRecord.month >> 8) & 0xFF;
            data[idx++] = testRecord.month & 0xFF;
            data[idx++] = (testRecord.date >> 8) & 0xFF;
            data[idx++] = testRecord.date & 0xFF;
            data[idx++] = (testRecord.hour >> 8) & 0xFF;
            data[idx++] = testRecord.hour & 0xFF;
            data[idx++] = (testRecord.minute >> 8) & 0xFF;
            data[idx++] = testRecord.minute & 0xFF;
            data[idx++] = (testRecord.second >> 8) & 0xFF;
            data[idx++] = testRecord.second & 0xFF;
            
            // 電壓和溫度
            data[idx++] = (testRecord.batteryVoltage >> 8) & 0xFF;
            data[idx++] = testRecord.batteryVoltage & 0xFF;
            data[idx++] = (testRecord.temperature >> 8) & 0xFF;
            data[idx++] = testRecord.temperature & 0xFF;
            
            // 發送回應
            UART_SendPacket(CMD_RESULT_ACK, data, idx);
            
            printf("測試結果: 值=%d, 項目=%d, 事件=%d, 校碼=%d\r\n",
                   testRecord.resultValue, testRecord.testType, 
                   testRecord.eventType, currentCode);
            
            // 重置血液樣本狀態
            bloodSampleReady = 0;
        }
        else
        {
            // 沒有測試記錄，發送錯誤
            UART_SendError(CMD_REQ_RESULT, ERR_DATA_FORMAT);
        }
    }
    else
    {
        // 發送錯誤
        UART_SendError(CMD_REQ_RESULT, ERR_MEASURE_TIMEOUT);
    }
}

/*********************************************************************
 * @fn      UART_HandleRawDataRequest
 *
 * @brief   處理請求RAW DATA命令
 *
 * @return  none
 */
void UART_HandleRawDataRequest(void)
{
    // RAW DATA暫時使用模擬數據
    uint8_t rawData[50];
    
    // 填充模擬的RAW DATA
    for(int i = 0; i < 50; i++)
    {
        rawData[i] = i;
    }
    
    uint8_t data[52];
    data[0] = 0;  // 高位
    data[1] = 50; // 長度
    memcpy(&data[2], rawData, 50);
    
    // 發送回應
    UART_SendPacket(CMD_RAW_DATA_ACK, data, 52);
    
    printf("RAW DATA請求已響應\r\n");
}

/*********************************************************************
 * @fn      UART_ForwardBLECommand
 *
 * @brief   從BLE命令轉發至CH32V203G8R6
 *
 * @param   cmdId - 命令ID
 * @param   data - 數據
 * @param   dataLen - 數據長度
 *
 * @return  none
 */
void UART_ForwardBLECommand(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    // 根據命令ID進行相應處理
    switch(cmdId)
    {
        case CMD_SYNC_TIME:
            UART_HandleSyncTime(data, dataLen);
            break;
            
        case CMD_REQ_DEVICE_STATUS:
            UART_HandleDeviceStatusRequest();
            break;
            
        case CMD_SET_CODE_EVENT:
            UART_HandleSetCodeEvent(data, dataLen);
            break;
            
        case CMD_BLOOD_SAMPLE_STATUS:
            UART_HandleBloodSampleRequest();
            break;
            
        case CMD_REQ_RESULT:
            UART_HandleResultRequest();
            break;
            
        case CMD_REQ_RAW_DATA:
            UART_HandleRawDataRequest();
            break;
            
        default:
            // 不支持的命令
            UART_SendError(cmdId, ERR_COMMAND_UNSUPPORTED);
            break;
    }
}

/*********************************************************************
 * @fn      UART_ProcessData
 *
 * @brief   處理接收到的UART數據
 *
 * @return  none
 */
void UART_ProcessData(void)
{
    static enum {
        STATE_IDLE,
        STATE_CMD,
        STATE_LEN,
        STATE_DATA,
        STATE_CHECKSUM,
        STATE_END
    } state = STATE_IDLE;
    
    static uint8_t cmdId = 0;
    static uint8_t dataLen = 0;
    static uint8_t dataIdx = 0;
    static uint8_t checksum = 0;
    
    // 檢查是否有數據待處理
    while(rxBufferHead != rxBufferTail)
    {
        uint8_t data = rxBuffer[rxBufferTail];
        rxBufferTail = (rxBufferTail + 1) % UART_RX_BUFFER_SIZE;
        
        // 協議狀態機
        switch(state)
        {
            case STATE_IDLE:
                if(data == PROTOCOL_START_MARKER)
                {
                    packetBuffer[0] = data;
                    state = STATE_CMD;
                }
                break;
                
            case STATE_CMD:
                cmdId = data;
                packetBuffer[1] = data;
                state = STATE_LEN;
                break;
                
            case STATE_LEN:
                dataLen = data;
                packetBuffer[2] = data;
                
                if(dataLen == 0)
                {
                    state = STATE_CHECKSUM;
                }
                else if(dataLen > MAX_PACKET_SIZE)
                {
                    // 數據長度超出上限，重置狀態
                    state = STATE_IDLE;
                }
                else
                {
                    dataIdx = 0;
                    state = STATE_DATA;
                }
                break;
                
            case STATE_DATA:
                packetBuffer[3 + dataIdx] = data;
                dataIdx++;
                
                if(dataIdx >= dataLen)
                {
                    state = STATE_CHECKSUM;
                }
                break;
                
            case STATE_CHECKSUM:
                checksum = data;
                packetBuffer[3 + dataLen] = data;
                state = STATE_END;
                break;
                
            case STATE_END:
                packetBuffer[4 + dataLen] = data;
                
                if(data == PROTOCOL_END_MARKER)
                {
                    // 檢查校驗和
                    uint8_t calculatedChecksum = UART_CalculateChecksum(cmdId, &packetBuffer[3], dataLen);
                    
                    if(calculatedChecksum == checksum)
                    {
                        // 校驗正確，處理命令
                        UART_ForwardBLECommand(cmdId, &packetBuffer[3], dataLen);
                    }
                    else
                    {
                        // 校驗錯誤
                        UART_SendError(cmdId, ERR_CHECKSUM);
                    }
                }
                
                // 重置狀態
                state = STATE_IDLE;
                break;
        }
    }
}

/*********************************************************************
 * @fn      UART_RxCallback
 *
 * @brief   UART接收數據回調
 *
 * @param   data - 數據
 * @param   len - 長度
 *
 * @return  none
 */
void UART_RxCallback(uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        // 將數據存入接收緩衝區
        rxBuffer[rxBufferHead] = data[i];
        rxBufferHead = (rxBufferHead + 1) % UART_RX_BUFFER_SIZE;
        
        // 檢查緩衝區是否溢出
        if(rxBufferHead == rxBufferTail)
        {
            // 緩衝區已滿，丟棄最早的數據
            rxBufferTail = (rxBufferTail + 1) % UART_RX_BUFFER_SIZE;
        }
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   USART2中斷處理函數
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // 讀取接收到的數據
        uint8_t data = USART_ReceiveData(USART2);
        
        // 調用接收回調
        UART_RxCallback(&data, 1);
        
        // 清除中斷標誌
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
} 
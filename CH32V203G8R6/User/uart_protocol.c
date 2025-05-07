#include "uart_protocol.h"
#include "debug.h"
#include "flash_param.h"
#include <string.h>

/* 全局變數定義 */
static UartProtocol_TypeDef rxPacket;  // 接收資料包
static UartRxState_TypeDef rxState;    // 接收狀態機
static uint8_t rxDataCount;            // 資料計數器
static uint8_t txBuffer[PROTOCOL_MAX_PACKET_LEN]; // 發送緩衝區
static TestRecord_TypeDef Test_Record; // 測試記錄

/* DMA 接收緩衝區 */
static uint8_t rxDMABuffer[PROTOCOL_MAX_PACKET_LEN];
static volatile uint8_t dmaRxCompleteFlag = 0;  // DMA接收完成標志

/* 通用錯誤響應數據 */
static uint8_t errorData[2];

/* 函數前向聲明 */
static void UART_Process_Packet(void);
static uint8_t UART_Find_Packet_In_Buffer(uint8_t *buffer, uint16_t size);

/*********************************************************************
 * @fn      UART_Calculate_Checksum
 *
 * @brief   計算校驗和
 *
 * @param   cmdId   - 指令ID
 *          dataLen - 資料長度
 *          data    - 資料指針
 *
 * @return  計算得到的校驗和
 */
uint8_t UART_Calculate_Checksum(uint8_t cmdId, uint8_t dataLen, uint8_t *data)
{
    uint16_t sum = cmdId + dataLen;
    
    for(uint8_t i = 0; i < dataLen; i++) {
        sum += data[i];
    }
    
    return (uint8_t)(sum & 0xFF);
}

/*********************************************************************
 * @fn      UART_Find_Packet_In_Buffer
 *
 * @brief   從DMA緩衝區中查找完整的數據包
 *
 * @param   buffer  - DMA緩衝區
 *          size    - 緩衝區大小
 *
 * @return  找到完整數據包返回1，否則返回0
 */
static uint8_t UART_Find_Packet_In_Buffer(uint8_t *buffer, uint16_t size)
{
    uint16_t i = 0;
    uint8_t dataLen = 0;
    
    // 尋找起始標記
    while(i < size - PROTOCOL_HEADER_LEN - PROTOCOL_FOOTER_LEN)
    {
        if(buffer[i] == PROTOCOL_START_MARK)
        {
            // 獲取數據長度
            dataLen = buffer[i + 2];
            
            // 檢查緩衝區是否有足夠的數據
            if(i + PROTOCOL_HEADER_LEN + dataLen + PROTOCOL_FOOTER_LEN <= size)
            {
                // 檢查結束標記
                if(buffer[i + PROTOCOL_HEADER_LEN + dataLen + 1] == PROTOCOL_END_MARK)
                {
                    // 複製數據到rxPacket
                    rxPacket.startMark = buffer[i];
                    rxPacket.cmdId = buffer[i + 1];
                    rxPacket.dataLen = dataLen;
                    memcpy(rxPacket.data, &buffer[i + 3], dataLen);
                    rxPacket.checksum = buffer[i + PROTOCOL_HEADER_LEN + dataLen];
                    rxPacket.endMark = buffer[i + PROTOCOL_HEADER_LEN + dataLen + 1];
                    
                    // 處理完整數據包
                    UART_Process_Packet();
                    return 1;
                }
            }
        }
        i++;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      UART_Protocol_Init
 *
 * @brief   初始化UART通訊協議
 *
 * @return  none
 */
void UART_Protocol_Init(void)
{
    // 初始化UART2
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};
    
    // 啟用UART2、GPIOA和DMA1時鐘
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // 設定UART2 Tx (PA2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 設定UART2 Rx (PA3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置UART2參數
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    
    // 配置DMA接收，UART2 Rx使用DMA1 Channel6
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rxDMABuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = PROTOCOL_MAX_PACKET_LEN;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    
    // 配置DMA中斷
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
    
    // 配置DMA中斷優先級
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 啟用UART2 DMA接收
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    
    // 啟用DMA通道
    DMA_Cmd(DMA1_Channel6, ENABLE);
    
    // 配置UART2中斷優先級 (保留UART中斷以處理錯誤和空閒檢測)
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 啟用UART2接收和錯誤中斷，以及空閒中斷
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); // 禁用RXNE中斷，使用DMA代替
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);  // 啟用IDLE中斷，用於檢測數據幀結束
    USART_ITConfig(USART2, USART_IT_PE | USART_IT_ERR, ENABLE); // 啟用奇偶校驗錯誤和其他錯誤中斷
    
    // 啟用UART2
    USART_Cmd(USART2, ENABLE);
    
    // 初始化接收狀態機
    rxState = UART_RX_WAIT_START;
    rxDataCount = 0;
    dmaRxCompleteFlag = 0;
    
    // 清空DMA接收緩衝區
    memset(rxDMABuffer, 0, PROTOCOL_MAX_PACKET_LEN);
}

/*********************************************************************
 * @fn      UART_Send_Byte
 *
 * @brief   發送單個字節數據
 *
 * @param   b - 要發送的字節
 *
 * @return  none
 */
static void UART_Send_Byte(uint8_t b)
{
    // 等待發送緩衝區為空
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    
    // 發送數據
    USART_SendData(USART2, b);
}

/*********************************************************************
 * @fn      UART_Send_Packet
 *
 * @brief   發送數據包
 *
 * @param   cmdId   - 指令ID
 *          data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Send_Packet(uint8_t cmdId, uint8_t *data, uint8_t dataLen)
{
    uint8_t checksum;
    uint16_t i = 0;
    
    // 計算校驗和
    checksum = UART_Calculate_Checksum(cmdId, dataLen, data);
    
    // 構建發送數據
    txBuffer[i++] = PROTOCOL_START_MARK;
    txBuffer[i++] = cmdId;
    txBuffer[i++] = dataLen;
    
    // 添加數據
    for(uint8_t j = 0; j < dataLen; j++) {
        txBuffer[i++] = data[j];
    }
    
    // 添加校驗和和結束標記
    txBuffer[i++] = checksum;
    txBuffer[i++] = PROTOCOL_END_MARK;
    
    // 發送數據包
    for(uint16_t j = 0; j < i; j++) {
        UART_Send_Byte(txBuffer[j]);
    }
}

/*********************************************************************
 * @fn      UART_Send_Error
 *
 * @brief   發送錯誤響應
 *
 * @param   originalCmd - 原始指令ID
 *          errorCode   - 錯誤代碼
 *
 * @return  none
 */
void UART_Send_Error(uint8_t originalCmd, uint8_t errorCode)
{
    errorData[0] = originalCmd;
    errorData[1] = errorCode;
    
    UART_Send_Packet(CMD_ERROR_ACK, errorData, 2);
}

/*********************************************************************
 * @fn      UART_Process_Packet
 *
 * @brief   處理完整接收的數據包
 *
 * @return  none
 */
static void UART_Process_Packet(void)
{
    // 重新計算校驗和
    uint8_t calcChecksum = UART_Calculate_Checksum(rxPacket.cmdId, rxPacket.dataLen, rxPacket.data);
    
    // 校驗和檢查
    if(calcChecksum != rxPacket.checksum) {
        UART_Send_Error(rxPacket.cmdId, ERR_CHECKSUM);
        return;
    }
    
    // 根據指令ID執行相應處理
    switch(rxPacket.cmdId) {
        case CMD_SYNC_TIME:
            UART_Handle_SyncTime(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_REQ_STATUS:
            UART_Handle_ReqStatus(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_SET_CODE_EVENT:
            UART_Handle_SetCodeEvent(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_CHECK_BLOOD:
            UART_Handle_CheckBlood(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_REQ_RESULT:
            UART_Handle_ReqResult(rxPacket.data, rxPacket.dataLen);
            break;
            
        case CMD_REQ_RAW_DATA:
            UART_Handle_ReqRawData(rxPacket.data, rxPacket.dataLen);
            break;
            
        default:
            // 不支援的指令
            UART_Send_Error(rxPacket.cmdId, ERR_COMMAND_UNSUPPORTED);
            break;
    }
}

/*********************************************************************
 * @fn      UART_Protocol_Process
 *
 * @brief   處理接收到的字節數據
 *
 * @param   rxByte - 接收到的字節
 *
 * @return  none
 */
void UART_Protocol_Process(uint8_t rxByte)
{
    switch(rxState) {
        case UART_RX_WAIT_START:
            if(rxByte == PROTOCOL_START_MARK) {
                rxPacket.startMark = rxByte;
                rxState = UART_RX_WAIT_CMD;
            }
            break;
            
        case UART_RX_WAIT_CMD:
            rxPacket.cmdId = rxByte;
            rxState = UART_RX_WAIT_LEN;
            break;
            
        case UART_RX_WAIT_LEN:
            rxPacket.dataLen = rxByte;
            rxDataCount = 0;
            
            if(rxPacket.dataLen > 0) {
                rxState = UART_RX_WAIT_DATA;
            } else {
                rxState = UART_RX_WAIT_CHECKSUM;
            }
            break;
            
        case UART_RX_WAIT_DATA:
            rxPacket.data[rxDataCount++] = rxByte;
            
            if(rxDataCount >= rxPacket.dataLen) {
                rxState = UART_RX_WAIT_CHECKSUM;
            }
            break;
            
        case UART_RX_WAIT_CHECKSUM:
            rxPacket.checksum = rxByte;
            rxState = UART_RX_WAIT_END;
            break;
            
        case UART_RX_WAIT_END:
            rxPacket.endMark = rxByte;
            
            if(rxByte == PROTOCOL_END_MARK) {
                // 完整包，進行處理
                UART_Process_Packet();
            } else {
                // 結束標記錯誤
                UART_Send_Error(rxPacket.cmdId, ERR_DATA_FORMAT);
            }
            
            // 重設狀態機
            rxState = UART_RX_WAIT_START;
            break;
            
        default:
            rxState = UART_RX_WAIT_START;
            break;
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
    /* 處理USART2空閒中斷，用於檢測一幀數據的接收完成 */
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        /* 讀取USART2->STATR，再讀取USART2->DATAR清除IDLE中斷標誌 */
        volatile uint8_t temp = USART2->STATR;
        temp = USART2->DATAR;
        (void)temp; // 避免編譯器警告
        
        /* 計算接收到的數據長度 */
        uint16_t rxCount = PROTOCOL_MAX_PACKET_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);
        
        /* 當接收數據長度大於最小有效數據包長度時，嘗試解析 */
        if(rxCount >= (PROTOCOL_HEADER_LEN + PROTOCOL_FOOTER_LEN))
        {
            /* 設置DMA接收完成標志，在主循環中處理 */
            dmaRxCompleteFlag = 1;
        }
    }
    
    /* 處理其他UART錯誤中斷 */
    if(USART_GetITStatus(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE | USART_IT_ORE) != RESET)
    {
        /* 清除錯誤中斷標誌，避免無限中斷 */
        USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE | USART_IT_ORE);
    }
}

/*********************************************************************
 * @fn      DMA1_Channel6_IRQHandler
 *
 * @brief   DMA1 Channel6（UART2接收）中斷處理函數
 *
 * @return  none
 */
void DMA1_Channel6_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC6))
    {
        /* 清除DMA傳輸完成中斷標誌 */
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        
        /* 設置DMA接收完成標志，在主循環中處理接收到的數據 */
        dmaRxCompleteFlag = 1;
    }
}

/*********************************************************************
 * @fn      UART_Handle_SyncTime
 *
 * @brief   處理同步時間指令
 *
 * @param   data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Handle_SyncTime(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[1];
    
    // 檢查數據長度
    if(dataLen != 7) {
        UART_Send_Error(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // 解析時間數據
    uint16_t year = (data[0] << 8) | data[1];
    uint8_t month = data[2];
    uint8_t day = data[3];
    uint8_t hour = data[4];
    uint8_t minute = data[5];
    uint8_t second = data[6];
    
    // 檢查時間值的合法性
    if(year < 2000 || year > 2099 || 
       month < 1 || month > 12 || 
       day < 1 || day > 31 || 
       hour > 23 || minute > 59 || second > 59) {
        UART_Send_Error(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return;
    }
    
    // 更新系統時間 (存到參數表中)
    ParamCodeTable.Time.YEAR = year - 2000;
    ParamCodeTable.Time.MONTH = month;
    ParamCodeTable.Time.DATE = day;
    ParamCodeTable.Time.HOUR = hour;
    ParamCodeTable.Time.MINUTE = minute;
    ParamCodeTable.Time.SECOND = second;
    
    // 保存時間到Flash，使用PARAM_UpdateDateTime替代PARAM_SaveTime
    FLASH_ParamResult_TypeDef result = PARAM_UpdateDateTime(
        year - 2000, month, day, hour, minute, second);
    
    if(result != PARAM_SUCCESS) {
        UART_Send_Error(CMD_SYNC_TIME, ERR_HARDWARE);
        return;
    }
    
    // 成功響應
    response[0] = 0x00;  // 成功狀態
    UART_Send_Packet(CMD_SYNC_TIME_ACK, response, 1);
}

/*********************************************************************
 * @fn      UART_Handle_ReqStatus
 *
 * @brief   處理請求裝置狀態指令
 *
 * @param   data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Handle_ReqStatus(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[8];
    uint16_t stripType, stripStatus, batteryVoltage, temperature;
    
    // 獲取當前測試項目
    stripType = ParamCodeTable.Display.Strip_Type;
    
    // 獲取試片狀態
    // 這裡應該有硬體檢測試片狀態的代碼
    // 暫時假設試片正常
    stripStatus = 0x0000;
    
    // 獲取電池電壓 (單位: mV)
    // 這裡應該有ADC讀取電池電壓的代碼
    // 暫時使用示例值
    batteryVoltage = 0x0B01;  // 2.817V
    
    // 獲取環境溫度 (單位: 攝氏度 x 10)
    // 這裡應該有讀取溫度感測器的代碼
    // 暫時使用示例值
    temperature = 0x0119;  // 28.1°C
    
    // 檢查電池和溫度狀態
    if(batteryVoltage < 2200) {  // 電池低於2.2V
        stripStatus = 0x0001;  // 電池電量過低
    } else if(temperature > 400) {  // 溫度高於40°C
        stripStatus = 0x0002;  // 溫度過高
    } else if(temperature < 100) {  // 溫度低於10°C
        stripStatus = 0x0003;  // 溫度過低
    }
    
    // 構建回應數據
    response[0] = (stripType >> 8) & 0xFF;
    response[1] = stripType & 0xFF;
    response[2] = (stripStatus >> 8) & 0xFF;
    response[3] = stripStatus & 0xFF;
    response[4] = (batteryVoltage >> 8) & 0xFF;
    response[5] = batteryVoltage & 0xFF;
    response[6] = (temperature >> 8) & 0xFF;
    response[7] = temperature & 0xFF;
    
    // 發送回應
    UART_Send_Packet(CMD_STATUS_ACK, response, 8);
}

/*********************************************************************
 * @fn      UART_Handle_SetCodeEvent
 *
 * @brief   處理設定CODE和EVENT指令
 *
 * @param   data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Handle_SetCodeEvent(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[1];
    
    // 檢查數據長度
    if(dataLen != 3) {
        UART_Send_Error(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // 解析數據
    uint8_t code = data[0];
    uint16_t event = (data[1] << 8) | data[2];
    
    // 檢查EVENT值的合法性
    if(event > EVENT_QC) {
        UART_Send_Error(CMD_SET_CODE_EVENT, ERR_DATA_FORMAT);
        return;
    }
    
    // 更新CODE和EVENT到測試記錄
    Test_Record.Strip_Code = code;
    Test_Record.EVENT = event;
    
    // 成功響應
    response[0] = 0x00;  // 成功狀態
    UART_Send_Packet(CMD_CODE_EVENT_ACK, response, 1);
}

/*********************************************************************
 * @fn      UART_Handle_CheckBlood
 *
 * @brief   處理檢測狀態請求指令
 *
 * @param   data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Handle_CheckBlood(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[1];
    uint8_t bloodDetected = 0;  // 是否檢測到血液
    uint8_t countdownSec = 5;   // 倒數計時秒數
    
    // 檢查是否已經檢測到血液
    // 這裡應該有檢測血液的硬體代碼
    // 假設當前沒有檢測到血液，返回錯誤
    if(!bloodDetected) {
        UART_Send_Error(CMD_CHECK_BLOOD, ERR_TIMEOUT);
        return;
    }
    
    // 構建回應數據
    response[0] = countdownSec;
    
    // 發送回應
    UART_Send_Packet(CMD_BLOOD_DETECTED_ACK, response, 1);
}

/*********************************************************************
 * @fn      UART_Handle_ReqResult
 *
 * @brief   處理請求測試結果指令
 *
 * @param   data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Handle_ReqResult(uint8_t *data, uint8_t dataLen)
{
    uint8_t response[25];
    uint16_t resultStatus, testValue, stripType;
    uint16_t year, month, day, hour, minute, second;
    uint16_t batteryVoltage, temperature;
    
    // 檢查測試是否已完成
    // 這裡應該有檢查測試狀態的代碼
    // 暫時假設測試已完成且成功
    resultStatus = 0x0000;  // 成功
    
    // 獲取測試結果
    // 根據測試項目類型獲取測試值
    stripType = ParamCodeTable.Display.Strip_Type;
    switch(stripType) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            testValue = 123;  // 假設值為12.3 mmol/L
            break;
            
        case STRIP_TYPE_U:
            testValue = 350;  // 假設值為350 umol/L
            break;
            
        case STRIP_TYPE_C:
            testValue = 52;   // 假設值為5.2 mmol/L
            break;
            
        case STRIP_TYPE_TG:
            testValue = 17;   // 假設值為1.7 mmol/L
            break;
            
        default:
            testValue = 0;
            break;
    }
    
    // 獲取時間
    year = 2000 + ParamCodeTable.Time.YEAR;
    month = ParamCodeTable.Time.MONTH;
    day = ParamCodeTable.Time.DATE;
    hour = ParamCodeTable.Time.HOUR;
    minute = ParamCodeTable.Time.MINUTE;
    second = ParamCodeTable.Time.SECOND;
    
    // 獲取電池電壓
    batteryVoltage = 0x0B01;  // 2.817V
    
    // 獲取環境溫度
    temperature = 0x0119;  // 28.1°C
    
    // 構建回應數據
    response[0] = (resultStatus >> 8) & 0xFF;
    response[1] = resultStatus & 0xFF;
    response[2] = (testValue >> 8) & 0xFF;
    response[3] = testValue & 0xFF;
    response[4] = (stripType >> 8) & 0xFF;
    response[5] = stripType & 0xFF;
    response[6] = (Test_Record.EVENT >> 8) & 0xFF;
    response[7] = Test_Record.EVENT & 0xFF;
    response[8] = Test_Record.Strip_Code;
    response[9] = (year >> 8) & 0xFF;
    response[10] = year & 0xFF;
    response[11] = (month >> 8) & 0xFF;
    response[12] = month & 0xFF;
    response[13] = (day >> 8) & 0xFF;
    response[14] = day & 0xFF;
    response[15] = (hour >> 8) & 0xFF;
    response[16] = hour & 0xFF;
    response[17] = (minute >> 8) & 0xFF;
    response[18] = minute & 0xFF;
    response[19] = (second >> 8) & 0xFF;
    response[20] = second & 0xFF;
    response[21] = (batteryVoltage >> 8) & 0xFF;
    response[22] = batteryVoltage & 0xFF;
    response[23] = (temperature >> 8) & 0xFF;
    response[24] = temperature & 0xFF;
    
    // 發送回應
    UART_Send_Packet(CMD_RESULT_ACK, response, 25);
}

/*********************************************************************
 * @fn      UART_Handle_ReqRawData
 *
 * @brief   處理請求RAW DATA指令
 *
 * @param   data    - 資料指針
 *          dataLen - 資料長度
 *
 * @return  none
 */
void UART_Handle_ReqRawData(uint8_t *data, uint8_t dataLen)
{
    // RAW DATA響應包含大量數據，需要分段處理
    // 這裡只展示基本結構，實際應用中需要根據實際的RAW DATA生成
    
    // 示例: 生成最小的RAW DATA響應
    uint8_t response[50];  // 實際應用中可能需要更大的緩衝區
    uint16_t rawDataLen = 48;  // RAW DATA總長度
    
    // 設置RAW DATA長度
    response[0] = (rawDataLen >> 8) & 0xFF;
    response[1] = rawDataLen & 0xFF;
    
    // 這裡填入實際的RAW DATA
    // 實際應用中，這些數據應該從測試過程中獲取
    // 填充示例數據
    memset(&response[2], 0, 48);
    
    // 發送RAW DATA響應
    UART_Send_Packet(CMD_RAW_DATA_ACK, response, 50);
}

/*********************************************************************
 * @fn      UART_Check_DMA_Received_Data
 *
 * @brief   檢查並處理DMA接收到的數據
 *
 * @return  none
 */
void UART_Check_DMA_Received_Data(void)
{
    if(dmaRxCompleteFlag)
    {
        /* 清除DMA接收完成標志 */
        dmaRxCompleteFlag = 0;
        
        /* 計算接收到的數據長度 */
        uint16_t rxCount = PROTOCOL_MAX_PACKET_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);
        
        /* 當接收數據長度大於最小有效數據包長度時，嘗試解析 */
        if(rxCount >= (PROTOCOL_HEADER_LEN + PROTOCOL_FOOTER_LEN))
        {
            /* 在DMA緩衝區中尋找並處理完整數據包 */
            UART_Find_Packet_In_Buffer(rxDMABuffer, rxCount);
        }
    }
}
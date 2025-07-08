/********************************** (C) COPYRIGHT *******************************
 * File Name          : uart_protocol.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/12
 * Description        : 多功能生化測試儀UART通訊協議
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#include "uart_protocol.h"
#include "debug.h"
#include "string.h"
#include "param_table.h"
#include "strip_detect.h"
#include "rtc.h"

/* 全局變數 */
static uint8_t rx_buffer[MAX_PACKET_SIZE];
static uint8_t tx_buffer[MAX_PACKET_SIZE];
static uint16_t rx_index = 0;
static uint8_t packet_received = 0;
uint16_t g_batteryVoltage = 0;  // 全局電池電壓變數

/* 當前測試狀態 */
static uint8_t test_in_progress = 0;
static uint8_t blood_detected = 0;
static uint8_t blood_countdown = 5; // 預設倒數時間為5秒

/* 測試結果暫存區 */
static TestResult_TypeDef current_test_result;
static RawData_TypeDef current_raw_data;

/* 外部聲明 - 這些函數需要在main或其他檔案中實現 */
extern uint8_t Check_Battery_Status(void);
extern uint16_t Get_Battery_Voltage(void);
extern uint16_t Get_Temperature(void);
extern uint8_t Check_Strip_Status(void);
extern uint8_t Check_Blood_Status(void);
extern uint8_t Get_Test_Data(TestResult_TypeDef *result);
extern uint8_t Get_Raw_Test_Data(RawData_TypeDef *rawData);

/*********************************************************************
 * @fn      UART_Protocol_Init
 *
 * @brief   初始化UART協議處理
 *
 * @return  none
 */
void UART_Protocol_Init(void)
{
    /* 初始化接收緩衝區索引 */
    rx_index = 0;
    packet_received = 0;
    
    /* 初始化測試狀態 */
    test_in_progress = 0;
    blood_detected = 0;
    blood_countdown = 5;
    
    /* 初始化結果結構 */
    memset(&current_test_result, 0, sizeof(TestResult_TypeDef));
    memset(&current_raw_data, 0, sizeof(RawData_TypeDef));
    
    printf("UART Protocol initialized\r\n");
}

/*********************************************************************
 * @fn      UART_Protocol_Process
 *
 * @brief   處理接收到的UART資料
 *
 * @return  none
 */
void UART_Protocol_Process(void)
{
    /* 檢查是否有完整封包接收到（這部分需要與外部UART接收機制整合） */
    if(packet_received)
    {
        /* 解析並處理封包 */
        UART_ProcessPacket(rx_buffer, rx_index);
        
        /* 重設接收變數 */
        rx_index = 0;
        packet_received = 0;
    }
    
    /* 如果正在測試中，檢查血液狀態 */
    if(test_in_progress && !blood_detected)
    {
        if(Check_Blood_Status() == 1)
        {
            blood_detected = 1;
            /* 發送血液檢測通知 */
            UART_SendBloodNotify(blood_countdown);
        }
    }
}

/*********************************************************************
 * @fn      UART_CalculateChecksum
 *
 * @brief   計算檢驗和
 *
 * @param   cmdId - 指令ID
 *          length - 資料長度
 *          data - 資料指標
 *
 * @return  計算得到的檢驗和
 */
uint8_t UART_CalculateChecksum(uint8_t cmdId, uint8_t length, uint8_t *data)
{
    uint16_t sum = cmdId + length;
    uint8_t i;
    
    for(i = 0; i < length; i++)
    {
        sum += data[i];
    }
    
    return (uint8_t)(sum % 256);
}

/*********************************************************************
 * @fn      UART_VerifyChecksum
 *
 * @brief   驗證封包檢驗和
 *
 * @param   packet - 封包資料
 *          length - 封包長度
 *
 * @return  1:檢驗通過 0:檢驗失敗
 */
uint8_t UART_VerifyChecksum(uint8_t *packet, uint16_t length)
{
    if(length < 5) // 最小封包長度檢查 (起始標記+指令ID+長度+校驗和+結束標記)
        return 0;
    
    uint8_t cmdId = packet[1];
    uint8_t dataLen = packet[2];
    uint8_t *data = &packet[3];
    uint8_t checksum = packet[3 + dataLen];
    
    uint8_t calculatedChecksum = UART_CalculateChecksum(cmdId, dataLen, data);
    
    return (checksum == calculatedChecksum) ? 1 : 0;
}

/*********************************************************************
 * @fn      UART_SendPacket
 *
 * @brief   發送協議封包
 *
 * @param   cmdId - 指令ID
 *          data - 資料指標
 *          length - 資料長度
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendPacket(uint8_t cmdId, uint8_t *data, uint8_t length)
{
    if(length > MAX_DATA_SIZE)
        return 0;
    
    uint8_t txIndex = 0;
    
    /* 封裝封包 */
    tx_buffer[txIndex++] = PROTOCOL_START_MARK;     // 起始標記
    tx_buffer[txIndex++] = cmdId;                  // 指令ID
    tx_buffer[txIndex++] = length;                 // 資料長度
    
    /* 資料區 */
    if(length > 0 && data != NULL)
    {
        memcpy(&tx_buffer[txIndex], data, length);
        txIndex += length;
    }
    
    /* 計算並加入校驗和 */
    tx_buffer[txIndex++] = UART_CalculateChecksum(cmdId, length, data);
    
    /* 加入結束標記 */
    tx_buffer[txIndex++] = PROTOCOL_END_MARK;
    
    /* 通過UART發送封包(這裡需要與實際UART發送機制整合) */
    for(uint8_t i = 0; i < txIndex; i++)
    {
        USART_SendData(USART2, tx_buffer[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessPacket
 *
 * @brief   處理收到的封包
 *
 * @param   packet - 封包資料
 *          length - 封包長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessPacket(uint8_t *packet, uint16_t length)
{
    /* 檢查是否有完整的封包 */
    if(packet[0] != PROTOCOL_START_MARK || packet[length-1] != PROTOCOL_END_MARK)
    {
        return 0;
    }
    
    /* 驗證校驗和 */
    if(!UART_VerifyChecksum(packet, length))
    {
        /* 校驗錯誤，發送錯誤回應 */
        UART_SendErrorAck(packet[1], ERR_CHECKSUM_ERROR);
        return 0;
    }
    
    /* 解析封包 */
    uint8_t cmdId = packet[1];
    uint8_t dataLen = packet[2];
    uint8_t *data = &packet[3];
    
    /* 根據命令ID處理不同的指令 */
    switch(cmdId)
    {
        case CMD_SYNC_TIME:
            /* 同步時間指令 */
            UART_ProcessSyncTime(data, dataLen);
            break;
        
        case CMD_GET_STATUS:
            /* 請求裝置狀態指令 */
            UART_ProcessGetStatus(data, dataLen);
            break;
        
        case CMD_SET_PARAM:
            /* 設置參數指令 */
            UART_ProcessSetParam(data, dataLen);
            break;
        
        case CMD_CHECK_BLOOD:
            /* 檢測血液狀態指令 */
            UART_ProcessCheckBlood(data, dataLen);
            break;
        
        case CMD_GET_RESULT:
            /* 請求測試結果指令 */
            UART_ProcessGetResult(data, dataLen);
            break;
        
        case CMD_GET_RAW_DATA:
            /* 請求RAW DATA指令 */
            UART_ProcessGetRawData(data, dataLen);
            break;
            
        case CMD_STRIP_INSERTED:
            /* 試片插入通知 */
            UART_ProcessStripInsertedCmd(data, dataLen);
            break;
            
        default:
            /* 不支援的指令 */
            UART_SendErrorAck(cmdId, ERR_COMMAND_NOTSUPP);
            return 0;
    }
}

/*********************************************************************
 * @fn      UART_ProcessSyncTime
 *
 * @brief   處理同步時間指令
 *
 * @param   data - 資料指標
 *          length - 資料長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessSyncTime(uint8_t *data, uint8_t length)
{
    /* 檢查資料長度是否正確 */
    if(length != 12) // 6個16位元的時間資料
    {
        UART_SendErrorAck(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 解析時間資料 */
    uint16_t year = (data[0] << 8) | data[1];
    uint16_t month = (data[2] << 8) | data[3];
    uint16_t date = (data[4] << 8) | data[5];
    uint16_t hour = (data[6] << 8) | data[7];
    uint16_t minute = (data[8] << 8) | data[9];
    uint16_t second = (data[10] << 8) | data[11];
    
    /* 驗證時間資料合理性 */
    if(year < 2000 || year > 2099 || month < 1 || month > 12 || 
       date < 1 || date > 31 || hour > 23 || minute > 59 || second > 59)
    {
        UART_SendErrorAck(CMD_SYNC_TIME, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 設置系統時間到參數表 */
    uint8_t result = PARAM_SetDateTime(year - 2000, month, date, hour, minute, second);
    
    /* 同時更新RTC時間 */
    RTC_SetTime(year - 2000, month, date, hour, minute, second);
    
    /* 發送確認回應 */
    UART_SendSyncTimeAck(result ? 0 : ERR_DATA_FORMAT);
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessGetStatus
 *
 * @brief   處理獲取裝置狀態指令
 *
 * @param   data - 資料指標
 *          length - 資料長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessGetStatus(uint8_t *data, uint8_t length)
{
    /* 檢查資料長度, 此指令無資料 */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_GET_STATUS, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 獲取當前裝置狀態 */
    DeviceStatus_TypeDef status;
    
    status.stripType = (uint16_t)PARAM_GetByte(PARAM_STRIP_TYPE);
    status.stripStatus = (uint16_t)Check_Strip_Status();
    status.batteryVoltage = Get_Battery_Voltage();
    status.temperature = Get_Temperature();
    
    /* 發送狀態回應 */
    UART_SendStatusAck(&status);
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessSetParam
 *
 * @brief   處理設置CODE和EVENT參數指令
 *
 * @param   data - 資料指標
 *          length - 資料長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessSetParam(uint8_t *data, uint8_t length)
{
    /* 檢查資料長度 */
    if(length != 4) // 2個16位元的參數(CODE和EVENT)
    {
        UART_SendErrorAck(CMD_SET_PARAM, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 解析參數 */
    uint16_t code = (data[0] << 8) | data[1];
    uint16_t event = (data[2] << 8) | data[3];
    
    /* 驗證參數合理性 */
    if(event >= EVENT_MAX)
    {
        UART_SendErrorAck(CMD_SET_PARAM, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 設置參數 */
    PARAM_SetWord(PARAM_CODE_TABLE_V, code);
    PARAM_SetByte(PARAM_EVENT, event);
    
    /* 同步更新測試結果結構 */
    current_test_result.eventType = event;
    current_test_result.stripCode = code;
    
    /* 發送確認回應 */
    UART_SendParamAck(0); // 0表示成功
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessCheckBlood
 *
 * @brief   處理檢測血液狀態指令
 *
 * @param   data - 資料指標
 *          length - 資料長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessCheckBlood(uint8_t *data, uint8_t length)
{
    /* 檢查資料長度, 此指令無資料 */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_CHECK_BLOOD, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 檢查試片狀態 */
    uint8_t stripStatus = Check_Strip_Status();
    if(stripStatus != 0)
    {
        UART_SendErrorAck(CMD_CHECK_BLOOD, stripStatus);
        return 0;
    }
    
    /* 啟動血液檢測 */
    test_in_progress = 1;
    blood_detected = 0;
    
    /* 檢查是否已經有血液樣本 */
    if(Check_Blood_Status() == 1)
    {
        blood_detected = 1;
        UART_SendBloodNotify(blood_countdown);
    }
    else
    {
        /* 沒有檢測到血液, 在超時前等待 */
        UART_SendErrorAck(CMD_CHECK_BLOOD, ERR_TEST_TIMEOUT);
    }
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessGetResult
 *
 * @brief   處理獲取測試結果指令
 *
 * @param   data - 資料指標
 *          length - 資料長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessGetResult(uint8_t *data, uint8_t length)
{
    /* 檢查資料長度, 此指令無資料 */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_GET_RESULT, ERR_DATA_FORMAT);
        return 0;
    }
    
    //目前先忽略test_in_progress與blood_detected的檢查

    // /* 檢查是否測試進行中 */
    // if(test_in_progress!=0)
    // {
    //     UART_SendErrorAck(CMD_GET_RESULT, ERR_TEST_TIMEOUT);
    //     return 0;
    // }
    
    // /* 檢查是否已經檢測到血液 */
    // if(!blood_detected)
    // {
    //     UART_SendErrorAck(CMD_GET_RESULT, ERR_BLOOD_NOT_ENOUGH);
    //     return 0;
    // }
    
    /* 獲取測試資料 */
    TestResult_TypeDef result;
    if(!Get_Test_Data(&result))
    {
        UART_SendErrorAck(CMD_GET_RESULT, ERR_HARDWARE_ERROR);
        test_in_progress = 0; // 測試結束
        return 0;
    }
    
    /* 儲存測試結果 */
    memcpy(&current_test_result, &result, sizeof(TestResult_TypeDef));
    
    /* 發送結果回應 */
    UART_SendResultAck(&result);
    
    /* 測試結束 */
    test_in_progress = 0;
    
    return 1;
}

/*********************************************************************
 * @fn      UART_ProcessGetRawData
 *
 * @brief   處理獲取RAW DATA指令
 *
 * @param   data - 資料指標
 *          length - 資料長度
 *
 * @return  處理結果(1:成功 0:失敗)
 */
uint8_t UART_ProcessGetRawData(uint8_t *data, uint8_t length)
{
    /* 檢查資料長度, 此指令無資料 */
    if(length != 0)
    {
        UART_SendErrorAck(CMD_GET_RAW_DATA, ERR_DATA_FORMAT);
        return 0;
    }
    
    /* 獲取原始測試資料 */
    RawData_TypeDef rawData;
    if(!Get_Raw_Test_Data(&rawData))
    {
        UART_SendErrorAck(CMD_GET_RAW_DATA, ERR_HARDWARE_ERROR);
        return 0;
    }
    
    /* 儲存原始資料 */
    memcpy(&current_raw_data, &rawData, sizeof(RawData_TypeDef));
    
    /* 發送原始資料回應 */
    UART_SendRawDataAck(&rawData);
    
    return 1;
}

/*********************************************************************
 * @fn      UART_SendSyncTimeAck
 *
 * @brief   發送時間同步確認回應
 *
 * @param   status - 狀態碼(0成功, 非0錯誤)
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendSyncTimeAck(uint8_t status)
{
    uint8_t data[1];
    data[0] = status;
    
    return UART_SendPacket(CMD_SYNC_TIME_ACK, data, 1);
}

/*********************************************************************
 * @fn      UART_SendStatusAck
 *
 * @brief   發送裝置狀態回應
 *
 * @param   status - 裝置狀態結構指標
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendStatusAck(DeviceStatus_TypeDef *status)
{
    uint8_t data[8]; // 4個16位元數據
    
    /* 封裝資料 */
    data[0] = (status->stripType >> 8) & 0xFF;
    data[1] = status->stripType & 0xFF;
    
    data[2] = (status->stripStatus >> 8) & 0xFF;
    data[3] = status->stripStatus & 0xFF;
    
    data[4] = (status->batteryVoltage >> 8) & 0xFF;
    data[5] = status->batteryVoltage & 0xFF;
    
    data[6] = (status->temperature >> 8) & 0xFF;
    data[7] = status->temperature & 0xFF;
    
    return UART_SendPacket(CMD_STATUS_ACK, data, 8);
}

/*********************************************************************
 * @fn      UART_SendParamAck
 *
 * @brief   發送參數設置確認回應
 *
 * @param   status - 狀態碼(0成功, 非0錯誤)
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendParamAck(uint8_t status)
{
    uint8_t data[1];
    data[0] = status;
    
    return UART_SendPacket(CMD_SET_PARAM_ACK, data, 1);
}

/*********************************************************************
 * @fn      UART_SendBloodNotify
 *
 * @brief   發送血液檢測通知
 *
 * @param   countdown - 倒數秒數
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendBloodNotify(uint8_t countdown)
{
    uint8_t data[1];
    data[0] = countdown;
    
    return UART_SendPacket(CMD_BLOOD_NOTIFY, data, 1);
}

/*********************************************************************
 * @fn      UART_SendResultAck
 *
 * @brief   發送測試結果回應
 *
 * @param   result - 測試結果結構指標
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendResultAck(TestResult_TypeDef *result)
{
    uint8_t data[26]; // 13個16位元數據
    uint8_t index = 0;
    
    /* 封裝資料 */
    data[index++] = (result->resultStatus >> 8) & 0xFF;
    data[index++] = result->resultStatus & 0xFF;
    
    data[index++] = (result->testValue >> 8) & 0xFF;
    data[index++] = result->testValue & 0xFF;
    
    data[index++] = (result->stripType >> 8) & 0xFF;
    data[index++] = result->stripType & 0xFF;
    
    data[index++] = (result->eventType >> 8) & 0xFF;
    data[index++] = result->eventType & 0xFF;
    
    data[index++] = (result->stripCode >> 8) & 0xFF;
    data[index++] = result->stripCode & 0xFF;
    
    data[index++] = (result->year >> 8) & 0xFF;
    data[index++] = result->year & 0xFF;
    
    data[index++] = (result->month >> 8) & 0xFF;
    data[index++] = result->month & 0xFF;
    
    data[index++] = (result->date >> 8) & 0xFF;
    data[index++] = result->date & 0xFF;
    
    data[index++] = (result->hour >> 8) & 0xFF;
    data[index++] = result->hour & 0xFF;
    
    data[index++] = (result->minute >> 8) & 0xFF;
    data[index++] = result->minute & 0xFF;
    
    data[index++] = (result->second >> 8) & 0xFF;
    data[index++] = result->second & 0xFF;
    
    data[index++] = (result->batteryVoltage >> 8) & 0xFF;
    data[index++] = result->batteryVoltage & 0xFF;
    
    data[index++] = (result->temperature >> 8) & 0xFF;
    data[index++] = result->temperature & 0xFF;
    
    return UART_SendPacket(CMD_RESULT_ACK, data, index);
}

/*********************************************************************
 * @fn      UART_SendRawDataAck
 *
 * @brief   發送RAW DATA回應
 *
 * @param   rawData - RAW DATA結構指標
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendRawDataAck(RawData_TypeDef *rawData)
{
    uint8_t data[60]; // RAW DATA結構的封裝
    uint8_t index = 0;
    
    /* 封裝資料 */
    data[index++] = (rawData->resultStatus >> 8) & 0xFF;
    data[index++] = rawData->resultStatus & 0xFF;
    
    data[index++] = (rawData->testValue >> 8) & 0xFF;
    data[index++] = rawData->testValue & 0xFF;
    
    data[index++] = (rawData->year >> 8) & 0xFF;
    data[index++] = rawData->year & 0xFF;
    
    data[index++] = (rawData->month >> 8) & 0xFF;
    data[index++] = rawData->month & 0xFF;
    
    data[index++] = (rawData->date >> 8) & 0xFF;
    data[index++] = rawData->date & 0xFF;
    
    data[index++] = (rawData->hour >> 8) & 0xFF;
    data[index++] = rawData->hour & 0xFF;
    
    data[index++] = (rawData->minute >> 8) & 0xFF;
    data[index++] = rawData->minute & 0xFF;
    
    data[index++] = (rawData->second >> 8) & 0xFF;
    data[index++] = rawData->second & 0xFF;
    
    data[index++] = (rawData->stripType >> 8) & 0xFF;
    data[index++] = rawData->stripType & 0xFF;
    
    data[index++] = (rawData->eventType >> 8) & 0xFF;
    data[index++] = rawData->eventType & 0xFF;
    
    data[index++] = (rawData->batteryVoltage >> 8) & 0xFF;
    data[index++] = rawData->batteryVoltage & 0xFF;
    
    data[index++] = (rawData->temperature >> 8) & 0xFF;
    data[index++] = rawData->temperature & 0xFF;
    
    data[index++] = (rawData->w1Adv >> 8) & 0xFF;
    data[index++] = rawData->w1Adv & 0xFF;
    
    data[index++] = (rawData->w2Adv >> 8) & 0xFF;
    data[index++] = rawData->w2Adv & 0xFF;
    
    data[index++] = (rawData->t1Ampl >> 8) & 0xFF;
    data[index++] = rawData->t1Ampl & 0xFF;
    
    data[index++] = (rawData->t1AcMax >> 8) & 0xFF;
    data[index++] = rawData->t1AcMax & 0xFF;
    
    data[index++] = (rawData->t1AcMin >> 8) & 0xFF;
    data[index++] = rawData->t1AcMin & 0xFF;
    
    data[index++] = (rawData->t3Adc >> 8) & 0xFF;
    data[index++] = rawData->t3Adc & 0xFF;
    
    data[index++] = (rawData->testCount >> 8) & 0xFF;
    data[index++] = rawData->testCount & 0xFF;
    
    data[index++] = (rawData->stripCode >> 8) & 0xFF;
    data[index++] = rawData->stripCode & 0xFF;
    
    /* 加入ASCII編碼的操作者ID和圖表編號 */
    for(int i = 0; i < 10; i++)
    {
        data[index++] = rawData->operatorId[i];
    }
    
    for(int i = 0; i < 10; i++)
    {
        data[index++] = rawData->chartNo[i];
    }
    
    return UART_SendPacket(CMD_RAW_DATA_ACK, data, index);
}

/*********************************************************************
 * @fn      UART_SendErrorAck
 *
 * @brief   發送錯誤回應
 *
 * @param   cmdId - 原始指令ID
 *          errorCode - 錯誤代碼
 *
 * @return  發送結果(1:成功 0:失敗)
 */
uint8_t UART_SendErrorAck(uint8_t cmdId, uint8_t errorCode)
{
    uint8_t data[2];
    data[0] = cmdId;
    data[1] = errorCode;
    
    return UART_SendPacket(CMD_ERROR_ACK, data, 2);
}

/*********************************************************************
 * @fn      UART2_Receive_Byte_ISR
 *
 * @brief   UART接收位元組中斷服務函數
 *          此函數應被整合到UART2的接收中斷處理中
 *
 * @param   byte - 收到的位元組
 *
 * @return  none
 */
void UART2_Receive_Byte_ISR(uint8_t byte)
{
    static uint8_t state = 0;
    static uint8_t dataLen = 0;
    static uint8_t dataCnt = 0;
    
    switch(state)
    {
        case 0: // 等待起始標記
            if(byte == PROTOCOL_START_MARK)
            {
                rx_buffer[0] = byte;
                rx_index = 1;
                state = 1;
            }
            break;
            
        case 1: // 接收指令ID
            rx_buffer[rx_index++] = byte;
            state = 2;
            break;
            
        case 2: // 接收資料長度
            rx_buffer[rx_index++] = byte;
            dataLen = byte;
            dataCnt = 0;
            
            if(dataLen == 0)
                state = 4; // 無資料，直接接收校驗和
            else
                state = 3; // 接收資料
            break;
            
        case 3: // 接收資料
            rx_buffer[rx_index++] = byte;
            dataCnt++;
            
            if(dataCnt >= dataLen)
                state = 4; // 資料接收完畢，接收校驗和
            break;
            
        case 4: // 接收校驗和
            rx_buffer[rx_index++] = byte;
            state = 5;
            break;
            
        case 5: // 接收結束標記
            if(byte == PROTOCOL_END_MARK)
            {
                rx_buffer[rx_index++] = byte;
                packet_received = 1; // 封包接收完成
            }
            else
            {
                // 結束標記錯誤，重置接收
                rx_index = 0;
            }
            
            state = 0; // 重置狀態機
            break;
            
        default:
            state = 0;
            rx_index = 0;
            break;
    }
    
    /* 緩衝區溢出保護 */
    if(rx_index >= MAX_PACKET_SIZE)
    {
        state = 0;
        rx_index = 0;
    }
}

/*********************************************************************
 * @fn      UART_UpdateBloodCountdown
 *
 * @brief   根據試片類型更新血液檢測倒數時間
 *
 * @param   stripType - 試片類型
 *
 * @return  none
 */
void UART_UpdateBloodCountdown(StripType_TypeDef stripType)
{
    uint16_t tpl1, trd1, evWidth1;
    uint16_t tpl2, trd2, evWidth2; // 未使用但參數需要
    
    /* 獲取試片時序參數 */
    if (PARAM_GetTimingParameters(stripType, &tpl1, &trd1, &evWidth1, 1) &&
        PARAM_GetTimingParameters(stripType, &tpl2, &trd2, &evWidth2, 2)) {
        
        /* 計算血液倒數時間：(tpl1 + trd1 + evWidth1) / 1000 */
        uint32_t totalTime = (uint32_t)tpl1 + (uint32_t)trd1 + (uint32_t)evWidth1;
        blood_countdown = (uint8_t)(totalTime / 1000);

        if ((totalTime%1000) > 0) {
            blood_countdown++;
        }
        
        /* 確保至少有1秒的倒數時間 */
        if (blood_countdown < 1) {
            blood_countdown = 1;
        }
        
        printf("Blood countdown updated for strip type %s: %d seconds\r\n", 
               StripType_GetName(stripType), blood_countdown);
    } else {
        /* 無法獲取時序參數，使用預設值 */
        blood_countdown = 5;
        printf("Failed to get timing parameters for strip type %s, using default countdown: %d seconds\r\n", 
               StripType_GetName(stripType), blood_countdown);
    }
}

/*********************************************************************
 * @fn      UART_ProcessStripInsertedCmd
 *
 * @brief   處理試片插入通知命令
 *
 * @param   data - 數據指針
 * @param   length - 數據長度
 *
 * @return  處理結果 (0=失敗, 1=成功)
 */
uint8_t UART_ProcessStripInsertedCmd(uint8_t *data, uint8_t length)
{
    uint16_t batteryVoltage = 0;
    
    /* 讀取試片Pin3和Pin5的狀態及電池電壓（如果有提供） */
    if (length >= 4) {
        uint8_t pin3Status = data[0];
        uint8_t pin5Status = data[1];
        
        /* 讀取電池電壓 */
        batteryVoltage = (uint16_t)((data[2] << 8) | data[3]);
        
        /* 更新系統中的電池電壓變數 */
        extern uint16_t g_batteryVoltage; // 這需要在某個地方定義
        g_batteryVoltage = batteryVoltage;
        
        /* 設置試片腳位狀態 */
        STRIP_DETECT_SetPinStatus(pin3Status, pin5Status);
        
        /* 觸發插入處理 */
        STRIP_DETECT_HandleInsertedEvent();
        
        /* 獲取判斷的試片類型並更新血液倒數時間 */
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
        UART_UpdateBloodCountdown(currentStripType);
        
        printf("Strip inserted notification received. Pin3=%d, Pin5=%d, BatteryVoltage=%dmV\r\n", 
               pin3Status, pin5Status, batteryVoltage);
    } else if (length >= 2) {
        uint8_t pin3Status = data[0];
        uint8_t pin5Status = data[1];
        
        /* 設置試片腳位狀態 */
        STRIP_DETECT_SetPinStatus(pin3Status, pin5Status);
        
        /* 觸發插入處理 */
        STRIP_DETECT_HandleInsertedEvent();
        
        /* 獲取判斷的試片類型並更新血液倒數時間 */
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
        UART_UpdateBloodCountdown(currentStripType);
        
        printf("Strip inserted notification received. Pin3=%d, Pin5=%d\r\n", 
               pin3Status, pin5Status);
    } else {
        printf("Strip inserted notification without pin status\r\n");
        STRIP_DETECT_HandleInsertedEvent();
        
        /* 獲取判斷的試片類型並更新血液倒數時間 */
        StripType_TypeDef currentStripType = STRIP_DETECT_GetStripType();
        UART_UpdateBloodCountdown(currentStripType);
    }
    
    return 1;
}

/*********************************************************************
 * @fn      UART_SendStripTypeAck
 *
 * @brief   發送試片類型回應
 *
 * @param   stripType - 判斷的試片類型
 *
 * @return  是否成功發送
 */
uint8_t UART_SendStripTypeAck(StripType_TypeDef stripType)
{
    uint8_t data[3];
    extern uint16_t g_batteryVoltage;
    
    data[0] = stripType;
    data[1] = (uint8_t)(g_batteryVoltage >> 8);     // 電池電壓高位元組
    data[2] = (uint8_t)(g_batteryVoltage & 0xFF);   // 電池電壓低位元組
    
    printf("Sending strip type ack: %s, Battery: %dmV\r\n", StripType_GetName(stripType), g_batteryVoltage);
    
    return UART_SendPacket(CMD_STRIP_TYPE_ACK, data, 3);
}

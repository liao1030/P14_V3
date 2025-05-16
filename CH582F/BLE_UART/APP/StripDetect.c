/********************************** (C) COPYRIGHT *******************************
 * File Name          : StripDetect.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : 試片插入偵測模組，偵測並判斷試片類型
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "HAL.h"
#include "app_uart.h"
#include "StripDetect.h"
#include "CH58x_common.h"

/*********************************************************************
 * MACROS
 */
#define STRIP_DEBOUNCE_TIME     50    // 除彈跳時間，單位毫秒
#define STRIP_CHECK_INTERVAL    100   // 定期檢查間隔，單位毫秒，縮短間隔以提高響應速度

// 試片插入引腳定義
#define STRIP_DETECT_3_PIN      GPIO_Pin_11
#define STRIP_DETECT_3_PORT     GPIOB
#define STRIP_DETECT_5_PIN      GPIO_Pin_15
#define STRIP_DETECT_5_PORT     GPIOA
#define T3_IN_SEL_PIN           GPIO_Pin_10
#define T3_IN_SEL_PORT          GPIOB
#define V2P5_ENABLE_PIN         GPIO_Pin_14
#define V2P5_ENABLE_PORT        GPIOA

// 通訊協議相關
#define PROTOCOL_STRIP_INSERTED         0x20  // 試片插入通知
#define PROTOCOL_STRIP_TYPE_ACK         0xA0  // 試片類型回應

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct {
    bool isStripInserted;           // 試片插入狀態
    uint8_t stripType;              // 試片類型
    uint8_t pin3Status;             // Strip_Detect_3 狀態
    uint8_t pin5Status;             // Strip_Detect_5 狀態
    uint32_t insertTimeStamp;       // 插入時間戳
    bool isWaitingForMCUResponse;   // 等待MCU回應
    bool isTypeDetected;            // 試片類型已辨識完成
} StripDetectState_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
volatile StripDetectState_t stripState = {
    .isStripInserted = false,
    .stripType = STRIP_TYPE_UNKNOWN,
    .pin3Status = 1,                // 上拉狀態，默認高電平
    .pin5Status = 1,                // 上拉狀態，默認高電平
    .insertTimeStamp = 0,
    .isWaitingForMCUResponse = false,
    .isTypeDetected = false
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern uint32_t tmos_GetSystemClock(void);
extern void send_to_uart_mcu(uint8_t *buf, uint16_t len);

/*********************************************************************
 * LOCAL VARIABLES
 */
static tmosTaskID StripDetect_TaskID = INVALID_TASK_ID;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void StripDetect_SendMessage(uint8_t msgType, uint8_t stripType);
static void StripDetect_SendInsertInfo(uint8_t pin3Status, uint8_t pin5Status);
static void StripDetect_PeriodicCheck(void);

/*********************************************************************
 * @fn      debounce_pin_status
 *
 * @brief   對引腳狀態進行軟件除彈跳處理
 *
 * @param   currentStatus - 當前狀態
 *          lastStatus - 上次狀態
 *          debounceCount - 除彈跳計數器指針
 *          threshold - 穩定閾值
 *
 * @return  穩定後的狀態
 */
static uint8_t debounce_pin_status(uint8_t currentStatus, uint8_t lastStatus, 
                                   uint8_t *debounceCount, uint8_t threshold)
{
    // 如果當前狀態與上次相同，增加計數
    if (currentStatus == lastStatus) {
        if (*debounceCount < threshold) {
            (*debounceCount)++;
        }
    } else {
        // 狀態改變，重置計數
        *debounceCount = 0;
    }
    
    // 如果計數達到閾值，認為狀態穩定
    if (*debounceCount >= threshold) {
        return currentStatus;
    }
    
    // 否則維持上次狀態
    return lastStatus;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      StripDetect_Init
 *
 * @brief   初始化試片偵測模組
 *
 * @param   task_id - 任務ID，在任務系統中使用
 *
 * @return  none
 */
void StripDetect_Init(tmosTaskID task_id)
{
    StripDetect_TaskID = task_id;
    
    // 初始化GPIO引腳
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_Floating);    // 配置Strip_Detect_3為浮空输入(已由外部上拉)
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_Floating);    // 配置Strip_Detect_5為浮空输入(已由外部上拉)
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);    // 配置T3_IN_SEL為推挽輸出
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);  // 配置V2P5_ENABLE為推挽輸出
    
    // 設定初始狀態
    GPIOB_SetBits(T3_IN_SEL_PIN);                // T3_IN_SEL輸出高電平，預設關閉T3電極
    GPIOA_SetBits(V2P5_ENABLE_PIN);              // V2P5_ENABLE輸出高電平，供電給CH32V203
    
    // 不再使用中斷，改為定期輪詢
    // 啟動定期檢查任務，間隔時間縮短以增加響應速度
    tmos_start_task(StripDetect_TaskID, STRIP_PERIODIC_CHECK_EVT, STRIP_CHECK_INTERVAL);
    
    PRINT("Strip Detect Module Initialized (Poll Mode)\n");
}

/*********************************************************************
 * @fn      StripDetect_ProcessEvent
 *
 * @brief   處理試片偵測相關事件
 *
 * @param   task_id  - 任務ID
 * @param   events - 事件標誌
 *
 * @return  未處理的事件
 */
uint16_t StripDetect_ProcessEvent(tmosTaskID task_id, uint16_t events)
{
    if(events & STRIP_PERIODIC_CHECK_EVT)
    {
        // 執行定期檢查
        StripDetect_PeriodicCheck();
        
        // 重新啟動定期檢查
        tmos_start_task(StripDetect_TaskID, STRIP_PERIODIC_CHECK_EVT, STRIP_CHECK_INTERVAL);
        
        return (events ^ STRIP_PERIODIC_CHECK_EVT);
    }
    
    // 返回未處理事件
    return 0;
}

/*********************************************************************
 * @fn      StripDetect_PeriodicCheck
 *
 * @brief   定期檢查試片狀態
 *
 * @param   none
 *
 * @return  none
 */
static void StripDetect_PeriodicCheck(void)
{
    // 靜態變數保存上次狀態以實現軟件除彈跳
    static uint8_t lastPin3Status = 1;
    static uint8_t lastPin5Status = 1;
    static uint8_t debounceCount3 = 0;
    static uint8_t debounceCount5 = 0;
    static uint8_t insertDebounceCount = 0;
    static uint8_t removeDebounceCount = 0;
    
    // 讀取當前狀態
    uint8_t currentPin3Status = GPIOB_ReadPortPin(STRIP_DETECT_3_PIN) ? 1 : 0;
    uint8_t currentPin5Status = GPIOA_ReadPortPin(STRIP_DETECT_5_PIN) ? 1 : 0;
    
    // 除彈跳處理
    uint8_t stablePin3Status = debounce_pin_status(currentPin3Status, lastPin3Status, &debounceCount3, 3);
    uint8_t stablePin5Status = debounce_pin_status(currentPin5Status, lastPin5Status, &debounceCount5, 3);
    
    // 更新上次狀態
    lastPin3Status = currentPin3Status;
    lastPin5Status = currentPin5Status;
    
    // 檢查試片插入情況 - 至少有一個引腳為低電平，試片可能已插入
    if (stablePin3Status == 0 || stablePin5Status == 0)
    {
        // 增加插入除彈跳計數
        if (insertDebounceCount < 5) {
            insertDebounceCount++;
        }
        // 重置拔出除彈跳計數
        removeDebounceCount = 0;
        
        // 確認試片穩定插入
        if (insertDebounceCount >= 3 && !stripState.isStripInserted)
        {
            // 第一次偵測到試片插入
            stripState.isStripInserted = true;
            stripState.pin3Status = stablePin3Status;
            stripState.pin5Status = stablePin5Status;
            
            // 發送試片插入消息給MCU
            StripDetect_SendInsertInfo(stablePin3Status, stablePin5Status);
            
            // 設定等待MCU回應
            stripState.isWaitingForMCUResponse = true;
            
            PRINT("Strip Inserted (Polling)! Pin3: %d, Pin5: %d\n", stablePin3Status, stablePin5Status);
            
            // 根據Pin3和Pin5的狀態預判試片類型
            if(stablePin3Status == 0 && stablePin5Status == 1) {
                // 可能是GLV或U型試片
                PRINT("Possible Strip Type: GLV or U\n");
            } else if(stablePin3Status == 0 && stablePin5Status == 0) {
                // 可能是C型試片
                PRINT("Possible Strip Type: C\n");
            } else if(stablePin3Status == 1 && stablePin5Status == 0) {
                // 可能是TG或GAV型試片
                PRINT("Possible Strip Type: TG or GAV\n");
            }
        }
    }
    else // 兩個引腳都是高電平，試片可能已拔出
    {
        // 增加拔出除彈跳計數
        if (removeDebounceCount < 5) {
            removeDebounceCount++;
        }
        // 重置插入除彈跳計數
        insertDebounceCount = 0;
        
        // 確認試片穩定拔出
        if (removeDebounceCount >= 3 && stripState.isStripInserted) 
        {
            // 試片拔出
            stripState.isStripInserted = false;
            stripState.stripType = STRIP_TYPE_UNKNOWN;
            stripState.isTypeDetected = false;
            stripState.isWaitingForMCUResponse = false;
            PRINT("Strip Removed (Polling)\n");
        }
    }
}

/*********************************************************************
 * @fn      StripDetect_SendMessage
 *
 * @brief   發送試片相關消息到MCU
 *
 * @param   msgType - 消息類型
 * @param   stripType - 試片類型
 *
 * @return  none
 */
static void StripDetect_SendMessage(uint8_t msgType, uint8_t stripType)
{
    uint8_t buf[6];
    
    // 組裝消息包
    buf[0] = 0xAA;                // 起始標記
    buf[1] = msgType;             // 消息類型
    buf[2] = 0x01;                // 長度為1
    buf[3] = stripType;           // 試片類型
    buf[4] = (buf[1] + buf[2] + buf[3]) % 256;  // 校驗和
    buf[5] = 0x55;                // 結束標記
    
    // 發送到MCU
    send_to_uart_mcu(buf, 6);
}

/*********************************************************************
 * @fn      StripDetect_SetStripType
 *
 * @brief   設定試片類型 (由MCU反饋試片類型時調用)
 *
 * @param   type - 試片類型
 *
 * @return  none
 */
void StripDetect_SetStripType(uint8_t type)
{
    if(stripState.isStripInserted && stripState.isWaitingForMCUResponse)
    {
        stripState.stripType = type;
        stripState.isTypeDetected = true;
        stripState.isWaitingForMCUResponse = false;
        
        // 根據試片類型配置相應的測量電路
        switch(type) {
            case STRIP_TYPE_GAV:
                // GAV試片需要開啟T3電極
                GPIOB_ResetBits(T3_IN_SEL_PIN);
                PRINT("GAV Strip Confirmed, T3 Enabled\n");
                break;
                
            default:
                // 其他試片類型使用WE電極，保持T3關閉
                GPIOB_SetBits(T3_IN_SEL_PIN);
                PRINT("Strip Type %d Confirmed, T3 Disabled\n", type);
                break;
        }
        
        // 通知GUI顯示試片類型
        PRINT("Strip Type %d Confirmed\n", type);
    }
}

/*********************************************************************
 * @fn      StripDetect_IsStripInserted
 *
 * @brief   檢查試片是否插入
 *
 * @param   none
 *
 * @return  bool - 試片插入狀態
 */
bool StripDetect_IsStripInserted(void)
{
    return stripState.isStripInserted;
}

/*********************************************************************
 * @fn      StripDetect_GetStripType
 *
 * @brief   獲取試片類型
 *
 * @param   none
 *
 * @return  uint8_t - 試片類型
 */
uint8_t StripDetect_GetStripType(void)
{
    return stripState.stripType;
}

/*********************************************************************
 * @fn      StripDetect_SendInsertInfo
 *
 * @brief   發送試片插入狀態資訊到MCU
 *
 * @param   pin3Status - Strip_Detect_3腳位狀態
 * @param   pin5Status - Strip_Detect_5腳位狀態
 *
 * @return  none
 */
static void StripDetect_SendInsertInfo(uint8_t pin3Status, uint8_t pin5Status)
{
    uint8_t buf[7];
    
    /* 組裝消息包 */
    buf[0] = 0xAA;                // 起始標記
    buf[1] = PROTOCOL_STRIP_INSERTED;  // 試片插入通知命令
    buf[2] = 0x02;                // 長度為2
    buf[3] = pin3Status;          // 第3腳狀態
    buf[4] = pin5Status;          // 第5腳狀態
    buf[5] = (buf[1] + buf[2] + buf[3] + buf[4]) % 256;  // 校驗和
    buf[6] = 0x55;                // 結束標記
    
    /* 發送到MCU */
    send_to_uart_mcu(buf, 7);
    
    PRINT("Strip Insert Info Sent. Pin3=%d, Pin5=%d\n", pin3Status, pin5Status);
}

/*********************************************************************
*********************************************************************/

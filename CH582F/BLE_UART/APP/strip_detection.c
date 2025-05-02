/**************************************************************************************************
 * 檔案名稱: strip_detection.c
 * 功能說明: 試片插入偵測、除彈跳及類型判別機制實現
 **************************************************************************************************/

#include "strip_detection.h"
#include "CH58x_common.h"

// 系統時鐘函數封裝，用於獲取系統毫秒計數
static uint32_t GetSysTickCount(void)
{
    // 使用系統提供的 SYS_GetSysTickCnt 函數，並轉換為毫秒
    return SYS_GetSysTickCnt() / (GetSysClock() / 1000);
}

// 定義與試片偵測相關的常數
#define STRIP_DEBOUNCE_TIME         20      // 除彈跳時間，單位ms
#define STRIP_UART_BAUD             115200  // UART1與CH32V203通訊的波特率

// 定義與試片偵測相關的GPIO腳位
#define STRIP_DETECT_3_PIN          GPIO_Pin_11  // PB11
#define STRIP_DETECT_5_PIN          GPIO_Pin_15  // PA15
#define V2P5_ENABLE_PIN             GPIO_Pin_14  // PA14，供電給CH32V203致能控制

// 定義MCU喚醒命令
#define CMD_WAKEUP_MCU              0xA1    // 喚醒CH32V203的命令
#define CMD_STRIP_INSERTED          0xA2    // 通知試片已插入的命令
#define CMD_REQUEST_STRIP_TYPE      0xA3    // 請求試片類型判別的命令
#define CMD_STRIP_TYPE_RESULT       0xB1    // 試片類型判別結果的命令

// 定義UART中斷標誌位
#define UART_RX_INT_FLAG            RB_IER_RECV_RDY  // UART接收中斷標誌

// 定義試片狀態
static volatile StripStatus_t stripStatus = STRIP_STATUS_NONE;
static volatile StripType_t stripType = STRIP_TYPE_UNKNOWN;

// 除彈跳定時器
static volatile uint32_t stripInsertionTime = 0;
static volatile uint8_t stripDetectionPending = 0;

// 根據UART接收到的數據更新試片類型
static void ProcessStripTypeResponse(uint8_t typeCode) {
    switch(typeCode) {
        case 0x01:
            stripType = STRIP_TYPE_GLV;
            break;
        case 0x02:
            stripType = STRIP_TYPE_U;
            break;
        case 0x03:
            stripType = STRIP_TYPE_C;
            break;
        case 0x04:
            stripType = STRIP_TYPE_TG;
            break;
        case 0x05:
            stripType = STRIP_TYPE_GAV;
            break;
        default:
            stripType = STRIP_TYPE_UNKNOWN;
            break;
    }
    
    if (stripType != STRIP_TYPE_UNKNOWN) {
        stripStatus = STRIP_STATUS_READY;
        // 可在此處觸發事件通知應用層
    }
}

// UART接收中斷處理函數
__HIGH_CODE
void UART1_IRQHandler(void) {
    uint8_t receivedByte;
    
    if (R8_UART1_IIR & RB_IER_RECV_RDY) {
        // 已接收到數據
        receivedByte = UART1_RecvByte();
        
        // 處理從CH32V203接收到的命令
        if (receivedByte == CMD_STRIP_TYPE_RESULT) {
            // 等待接收試片類型結果
            while (!(R8_UART1_LSR & RB_LSR_DATA_RDY));
            receivedByte = UART1_RecvByte();
            ProcessStripTypeResponse(receivedByte);
        }
    }
}

// 檢查試片插入狀態（除彈跳處理）
void Strip_CheckInsertionStatus(void) {
    uint8_t pin3Status, pin5Status;
    
    // 如果已經在處理中，檢查除彈跳時間
    if (stripDetectionPending) {
        if ((GetSysTickCount() - stripInsertionTime) >= STRIP_DEBOUNCE_TIME) {
            stripDetectionPending = 0;
            
            // 再次檢查腳位狀態確認是否為有效插入
            pin3Status = GPIOB_ReadPortPin(STRIP_DETECT_3_PIN);
            pin5Status = GPIOA_ReadPortPin(STRIP_DETECT_5_PIN);
            
            // 檢查是否有任一接點被拉低（表示試片插入）
            if (!pin3Status || !pin5Status) {
                // 試片確實插入，更新狀態
                stripStatus = STRIP_STATUS_INSERTED;
                
                // 喚醒CH32V203
                GPIOA_SetBits(V2P5_ENABLE_PIN);  // 開啟CH32V203電源
                DelayMs(10);  // 等待電源穩定
                
                // 發送喚醒命令給CH32V203
                UART1_SendByte(CMD_WAKEUP_MCU);
                DelayMs(5);
                
                // 通知試片已插入並請求判別類型
                UART1_SendByte(CMD_STRIP_INSERTED);
                DelayMs(5);
                UART1_SendByte(CMD_REQUEST_STRIP_TYPE);
                
                // 進入測試準備狀態，等待CH32V203回應試片類型
            }
        }
    }
}

// 外部中斷處理函數
__HIGH_CODE
void GPIO_IRQHandler(void) {
    // 檢查是否是Strip_Detect_3中斷
    if (GPIOB_ReadITFlagBit(STRIP_DETECT_3_PIN)) {
        GPIOB_ClearITFlagBit(STRIP_DETECT_3_PIN);
        
        // 只處理下降沿（試片插入）
        if (stripStatus == STRIP_STATUS_NONE) {
            // 記錄時間戳，開始除彈跳
            stripInsertionTime = GetSysTickCount();
            stripDetectionPending = 1;
        }
    }
    
    // 檢查是否是Strip_Detect_5中斷
    if (GPIOA_ReadITFlagBit(STRIP_DETECT_5_PIN)) {
        GPIOA_ClearITFlagBit(STRIP_DETECT_5_PIN);
        
        // 只處理下降沿（試片插入）
        if (stripStatus == STRIP_STATUS_NONE) {
            // 記錄時間戳，開始除彈跳
            stripInsertionTime = GetSysTickCount();
            stripDetectionPending = 1;
        }
    }
}

// 初始化試片插入偵測
void Strip_Detection_Init(void) {
    // 配置Strip_Detect_3 (PB11)為上拉輸入，並啟用下降沿中斷
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ(GPIO_B_IRQn);
    
    // 配置Strip_Detect_5 (PA15)為上拉輸入，並啟用下降沿中斷
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ(GPIO_A_IRQn);
    
    // 配置V2P5_ENABLE (PA14)為推挽輸出，初始為低電位
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(V2P5_ENABLE_PIN); // 初始關閉CH32V203電源
    
    // 初始化UART1，用於與CH32V203通訊
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    UART1_BaudRateCfg(STRIP_UART_BAUD);
    
    // 啟用UART1接收中斷
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
    
    // 初始化狀態變數
    stripStatus = STRIP_STATUS_NONE;
    stripType = STRIP_TYPE_UNKNOWN;
    stripDetectionPending = 0;
}

// 獲取試片狀態
StripStatus_t Strip_GetStatus(void) {
    return stripStatus;
}

// 獲取試片類型
StripType_t Strip_GetType(void) {
    return stripType;
}

// 設置試片類型
void Strip_SetType(StripType_t type) {
    stripType = type;
    if (type != STRIP_TYPE_UNKNOWN) {
        stripStatus = STRIP_STATUS_READY;
    }
}

// 測試完成處理函數
void Strip_TestComplete(void) {
    stripStatus = STRIP_STATUS_COMPLETE;
    // 可在此處添加測試完成後的處理邏輯
}
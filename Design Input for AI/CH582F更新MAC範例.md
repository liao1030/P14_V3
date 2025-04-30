# CH582F MAC地址設定與讀取範例程式

本文檔提供了完整的範例程式碼，詳細說明如何在CH582F晶片上設定和讀取MAC地址，適用於低功耗藍牙應用開發。

## 完整範例程式碼

```c
/**************************************************************************************************
 * 檔案名稱: mac_example.c
 * 功能說明: CH582F MAC地址設定與讀取示範
 **************************************************************************************************/

#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"
#include "config.h"

// 任務ID宣告
static uint8_t taskID = INVALID_TASK_ID; // 主應用任務ID

// 事件宣告
#define START_DEVICE_EVT            0x0001
#define PRINT_MAC_EVT               0x0002

// MAC地址配置
static uint8_t customMacAddr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; // 自定義MAC地址

/**
 * @brief 通用應用回調函數
 */
static void appGapRoleCallback(gapRoleEvent_t *pEvent)
{
    switch (pEvent->gap.opcode)
    {
        case GAP_DEVICE_INIT_DONE_EVENT:
        {
            gapDeviceInitDoneEvent_t *pDevInitDone = (gapDeviceInitDoneEvent_t *)(pEvent);
            
            // 印出當前設備MAC地址
            PRINT("系統MAC地址: ");
            for (int i = 0; i < B_ADDR_LEN; i++)
            {
                PRINT("%02X", pDevInitDone->devAddr[i]);
                if (i < B_ADDR_LEN - 1)
                    PRINT(":");
            }
            PRINT("\n");
            
            // 設定一個事件在5秒後再次讀取MAC地址
            tmos_start_task(taskID, PRINT_MAC_EVT, MS1_TO_SYSTEM_TIME(5000));
            break;
        }
        
        default:
            break;
    }
}

/**
 * @brief 設定MAC地址
 */
static void setCustomMacAddress(void)
{
    PRINT("設定自定義MAC地址: ");
    for (int i = 0; i < B_ADDR_LEN; i++)
    {
        PRINT("%02X", customMacAddr[i]);
        if (i < B_ADDR_LEN - 1)
            PRINT(":");
    }
    PRINT("\n");
    
    // 兩種方式設定MAC地址
    
    // 方式1: 使用GAPRole_SetParameter (僅在設備初始化前有效)
    GAPRole_SetParameter(GAPROLE_BD_ADDR, B_ADDR_LEN, customMacAddr);
    
    // 方式2: 直接修改藍牙配置參數 (需在初始化藍牙庫之前設定)
    // 注意: 這應該在主函數中的CH58X_BLEInit()調用之前執行
    /*
    bleConfig_t bleConfig;
    tmos_memcpy(bleConfig.MacAddr, customMacAddr, 6);
    BLE_LibInit(&bleConfig);
    */
}

/**
 * @brief 讀取MAC地址並印出
 */
static void readAndPrintMacAddress(void)
{
    PRINT("讀取當前MAC地址...\n");
    
    // 方式1: 使用GAPRole_GetParameter
    uint8_t macAddr[6];
    GAPRole_GetParameter(GAPROLE_BD_ADDR, macAddr);
    
    PRINT("讀取到的MAC地址: ");
    for (int i = 0; i < B_ADDR_LEN; i++)
    {
        PRINT("%02X", macAddr[i]);
        if (i < B_ADDR_LEN - 1)
            PRINT(":");
    }
    PRINT("\n");
}

/**
 * @brief 應用任務事件處理函數
 */
uint16_t MacApp_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if (events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        
        if ((pMsg = tmos_msg_receive(taskID)) != NULL)
        {
            // 處理消息
            tmos_msg_deallocate(pMsg);
        }
        
        return (events ^ SYS_EVENT_MSG);
    }
    
    if (events & START_DEVICE_EVT)
    {
        // 啟動設備
        PRINT("啟動設備...\n");
        
        // 設定自定義MAC地址
        setCustomMacAddress();
        
        // 註冊回調函數並啟動設備 (以Peripheral角色為例)
        GAPRole_PeripheralStartDevice(taskID, NULL, &appGapRoleCallback);
        
        return (events ^ START_DEVICE_EVT);
    }
    
    if (events & PRINT_MAC_EVT)
    {
        // 再次讀取MAC地址
        readAndPrintMacAddress();
        
        return (events ^ PRINT_MAC_EVT);
    }
    
    return 0;
}

/**
 * @brief 應用任務初始化
 */
void MacApp_Init(void)
{
    // 註冊任務
    taskID = TMOS_ProcessEventRegister(MacApp_ProcessEvent);
    
    // 設定初始事件
    tmos_set_event(taskID, START_DEVICE_EVT);
}

/**
 * @brief 主函數
 */
int main(void)
{
    // 設定系統時鐘
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    // 配置IO口
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    
    // 配置串口
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    
    PRINT("CH582F MAC地址設定與讀取示範\n");
    
    // 初始化藍牙庫 (在此之前可修改bleConfig中的MAC地址)
    CH58X_BLEInit();
    
    // 初始化HAL層
    HAL_Init();
    
    // 初始化GAP角色
    GAPRole_PeripheralInit();
    
    // 初始化自定義應用
    MacApp_Init();
    
    // 主循環
    while(1)
    {
        TMOS_SystemProcess();
    }
}
```

## 程式說明

### 重要函數介紹

1. **setCustomMacAddress()**: 設定MAC地址
   - 使用`GAPRole_SetParameter()`函數設定MAC地址
   - 示範了另一種透過`bleConfig_t`結構設定的方法(註解部分)

2. **readAndPrintMacAddress()**: 讀取並顯示MAC地址
   - 使用`GAPRole_GetParameter()`函數讀取當前MAC地址

3. **appGapRoleCallback()**: GAP角色事件回調
   - 在`GAP_DEVICE_INIT_DONE_EVENT`事件中獲取系統MAC地址

### 設定MAC地址的兩種方式

#### 方式1: 透過GAPRole_SetParameter函數

```c
GAPRole_SetParameter(GAPROLE_BD_ADDR, B_ADDR_LEN, customMacAddr);
```

此方法需在BLE設備初始化前調用才有效。

#### 方式2: 透過藍牙庫初始化參數

```c
bleConfig_t bleConfig;
tmos_memcpy(bleConfig.MacAddr, customMacAddr, 6);
BLE_LibInit(&bleConfig);
```

此方法必須在`CH58X_BLEInit()`調用前執行，能夠更直接地設定MAC地址。

### 讀取MAC地址的兩種方式

#### 方式1: 使用GAPRole_GetParameter函數

```c
uint8_t macAddr[6];
GAPRole_GetParameter(GAPROLE_BD_ADDR, macAddr);
```

#### 方式2: 從初始化完成事件中讀取

```c
case GAP_DEVICE_INIT_DONE_EVENT:
{
    gapDeviceInitDoneEvent_t *pDevInitDone = (gapDeviceInitDoneEvent_t *)(pEvent);
    // pDevInitDone->devAddr 包含MAC地址
    ...
}
```

## 使用注意事項

1. MAC地址設定應在設備初始化前完成，否則設定可能無效
2. 在實際產品中，每個設備應有唯一的MAC地址，避免地址衝突
3. 部分CH582F可能在出廠時已經燒錄了MAC地址，請根據實際情況決定是否覆蓋
4. 使用`GAPROLE_BD_ADDR`參數時，請確保它是BLE協議棧支持的參數(參考WCH_BLE軟件開發參考手冊)

## 應用場景

- 藍牙設備製造時寫入唯一MAC地址
- 開發階段測試不同MAC地址對配對連接的影響
- 需要特定MAC地址前綴的應用(如公司標識)
- 批量燒錄設備時的MAC地址管理

此範例提供了完整的MAC地址操作流程，您可以根據實際需求調整或擴展功能。

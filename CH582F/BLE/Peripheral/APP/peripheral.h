/* 外部變量聲明 */
extern uint8_t ble_device_name[20]; /* 聲明來自main.c的設備名稱 */ 

/* 事件定義 */
#define SBP_START_DEVICE_EVT               0x0001
#define SBP_PERIODIC_EVT                   0x0002
#define SBP_READ_RSSI_EVT                  0x0004
#define SBP_PARAM_UPDATE_EVT               0x0008
#define SBP_PHY_UPDATE_EVT                 0x0010

/* 連接項目類型定義 */
typedef struct
{
    uint16_t connHandle;      // 連接句柄
    uint16_t connInterval;    // 連接間隔
    uint16_t connSlaveLatency;// 從機延遲
    uint16_t connTimeout;     // 連接超時
} peripheralConnItem_t;

/* 函數聲明 */
void Peripheral_Init(void);
uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events); 
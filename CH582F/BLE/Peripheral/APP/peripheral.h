/* �ⲿ׃���� */
extern uint8_t ble_device_name[20]; /* ������main.c���O�����Q */ 

/* �¼����x */
#define SBP_START_DEVICE_EVT               0x0001
#define SBP_PERIODIC_EVT                   0x0002
#define SBP_READ_RSSI_EVT                  0x0004
#define SBP_PARAM_UPDATE_EVT               0x0008
#define SBP_PHY_UPDATE_EVT                 0x0010

/* �B���Ŀ��Ͷ��x */
typedef struct
{
    uint16_t connHandle;      // �B�Ӿ��
    uint16_t connInterval;    // �B���g��
    uint16_t connSlaveLatency;// �ęC���t
    uint16_t connTimeout;     // �B�ӳ��r
} peripheralConnItem_t;

/* ������ */
void Peripheral_Init(void);
uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events); 
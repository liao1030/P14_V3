/********************************** (C) COPYRIGHT *******************************
 * File Name          : StripDetect.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/13
 * Description        : ԇƬ����ɜyģ�M���ɜy�K�Д�ԇƬ���
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
#include "batt_measure.h"

/*********************************************************************
 * MACROS
 */
#define STRIP_DEBOUNCE_TIME     50    // �������r�g����λ����
#define STRIP_CHECK_INTERVAL    500   // ���ڙz���g������λ����

// ԇƬ�������_���x
#define STRIP_DETECT_3_PIN      GPIO_Pin_11
#define STRIP_DETECT_3_PORT     GPIOB
#define STRIP_DETECT_5_PIN      GPIO_Pin_15
#define STRIP_DETECT_5_PORT     GPIOA
#define T3_IN_SEL_PIN           GPIO_Pin_10
#define T3_IN_SEL_PORT          GPIOB
#define V2P5_ENABLE_PIN         GPIO_Pin_14
#define V2P5_ENABLE_PORT        GPIOA

// ͨӍ�f�h���P
#define PROTOCOL_STRIP_INSERTED         0x20  // ԇƬ����֪ͨ
#define PROTOCOL_STRIP_TYPE_ACK         0xA0  // ԇƬ��ͻؑ�
#define PROTOCOL_BATT_DATA_LEN          2     // 늳�늉������L��(2�ֹ�)

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct {
    bool isStripInserted;           // ԇƬ�����B
    uint8_t stripType;              // ԇƬ���
    uint8_t pin3Status;             // Strip_Detect_3 ��B
    uint8_t pin5Status;             // Strip_Detect_5 ��B
    uint32_t insertTimeStamp;       // ����r�g��
    bool isWaitingForMCUResponse;   // �ȴ�MCU�ؑ�
    bool isTypeDetected;            // ԇƬ����ѱ��R���
} StripDetectState_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
volatile StripDetectState_t stripState = {
    .isStripInserted = false,
    .stripType = STRIP_TYPE_UNKNOWN,
    .pin3Status = 1,                // ������B��Ĭ�J���ƽ
    .pin5Status = 1,                // ������B��Ĭ�J���ƽ
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
static uint16_t StripDetect_GetBatteryVoltage(void);

/*********************************************************************
 * @fn      debounce_pin_status
 *
 * @brief   �����_��B�M��ܛ��������̎��
 *
 * @param   currentStatus - ��ǰ��B
 *          lastStatus - �ϴΠ�B
 *          debounceCount - ������Ӌ����ָ�
 *          threshold - �����ֵ
 *
 * @return  ������Ġ�B
 */
static uint8_t debounce_pin_status(uint8_t currentStatus, uint8_t lastStatus, 
                                   uint8_t *debounceCount, uint8_t threshold)
{
    // �����ǰ��B�c�ϴ���ͬ������Ӌ��
    if (currentStatus == lastStatus) {
        if (*debounceCount < threshold) {
            (*debounceCount)++;
        }
    } else {
        // ��B��׃������Ӌ��
        *debounceCount = 0;
    }
    
    // ���Ӌ���_���ֵ���J���B����
    if (*debounceCount >= threshold) {
        return currentStatus;
    }
    
    // ��t�S���ϴΠ�B
    return lastStatus;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      StripDetect_Init
 *
 * @brief   ��ʼ��ԇƬ�ɜyģ�M
 *
 * @param   task_id - �΄�ID�����΄�ϵ�y��ʹ��
 *
 * @return  none
 */
void StripDetect_Init(tmosTaskID task_id)
{
    StripDetect_TaskID = task_id;
    
    // ��ʼ��GPIO���_
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_Floating);    // ����Strip_Detect_3�鸡������(�����ⲿ����)
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_Floating);    // ����Strip_Detect_5�鸡������(�����ⲿ����)
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);    // ����T3_IN_SEL������ݔ��
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);  // ����V2P5_ENABLE������ݔ��
    
    // �O����ʼ��B
    GPIOB_SetBits(T3_IN_SEL_PIN);                // T3_IN_SELݔ�����ƽ���A�O�P�]T3늘O
    GPIOA_ResetBits(V2P5_ENABLE_PIN);              // V2P5_ENABLEݔ�����ƽ������늽oCH32V203
    
    // ��ʼ��늳�늉��y��ģ�M
    Batt_MeasureInit();
    
    // ���Ӷ��ڙz���΄գ�ÿ100ms�z��һ��
    tmos_start_task(StripDetect_TaskID, STRIP_PERIODIC_CHECK_EVT, MS1_TO_SYSTEM_TIME(100));
    
    PRINT("Strip Detect Module Initialized (Polling Mode)\n");
}

/*********************************************************************
 * @fn      StripDetect_ProcessEvent
 *
 * @brief   ̎��ԇƬ�ɜy���P�¼�
 *
 * @param   task_id  - �΄�ID
 * @param   events - �¼����I
 *
 * @return  δ̎����¼�
 */
uint16_t StripDetect_ProcessEvent(tmosTaskID task_id, uint16_t events)
{
    if(events & STRIP_PERIODIC_CHECK_EVT)
    {
        // ���ж��ڙz��
        StripDetect_PeriodicCheck();
        
        return (events ^ STRIP_PERIODIC_CHECK_EVT);
    }
    
    if(events & STRIP_SEND_MESSAGE_EVT)
    {
        // �ڵȴ�1000ms��l��ԇƬ������Ϣ�oMCU
        PRINT("STRIP_SEND_MESSAGE_EVT triggered, isStripInserted=%d\n", stripState.isStripInserted);
        if(stripState.isStripInserted)
        {
            // �l��ԇƬ������Ϣ�oMCU
            StripDetect_SendInsertInfo(stripState.pin3Status, stripState.pin5Status);
            PRINT("Strip Insert Info Sent after Power Up Delay\n");
        }
        
        return (events ^ STRIP_SEND_MESSAGE_EVT);
    }
    
    // ����δ̎���¼�
    return 0;
}

/*********************************************************************
 * @fn      StripDetect_PeriodicCheck
 *
 * @brief   ���ڙz��ԇƬ��B
 *
 * @param   none
 *
 * @return  none
 */
static void StripDetect_PeriodicCheck(void)
{
    // ������̎��
    static uint8_t lastPin3Status = 1;
    static uint8_t lastPin5Status = 1;
    static uint8_t debounceCount3 = 0;
    static uint8_t debounceCount5 = 0;
    
    // �xȡ��ǰ��B
    uint8_t currentPin3Status = GPIOB_ReadPortPin(STRIP_DETECT_3_PIN) ? 1 : 0;
    uint8_t currentPin5Status = GPIOA_ReadPortPin(STRIP_DETECT_5_PIN) ? 1 : 0;
    
    // ������̎��
    uint8_t stablePin3Status = debounce_pin_status(currentPin3Status, lastPin3Status, &debounceCount3, 3);
    uint8_t stablePin5Status = debounce_pin_status(currentPin5Status, lastPin5Status, &debounceCount5, 3);
    
    // �����ϴΠ�B
    lastPin3Status = currentPin3Status;
    lastPin5Status = currentPin5Status;

    // �z��ԇƬ������r - ������һ�����_����ƽ��ԇƬ�����Ѳ���
    if (stablePin3Status == 0 || stablePin5Status == 0)
    {
        if(!stripState.isStripInserted)
        {
            // ��һ�΂ɜy��ԇƬ����
            stripState.isStripInserted = true;
            stripState.pin3Status = stablePin3Status;
            stripState.pin5Status = stablePin5Status;
            stripState.insertTimeStamp = GetSysClock() / 1000;
            
            // ԇƬ����r��V2P5_ENABLEݔ�����ƽ����늽oCH32V203
            GPIOA_SetBits(V2P5_ENABLE_PIN);

            // �������t�΄գ��ȴ�1000ms��l��ԇƬ������Ϣ�oMCU
            tmos_start_task(StripDetect_TaskID, STRIP_SEND_MESSAGE_EVT, MS1_TO_SYSTEM_TIME(1000));
            
            // �O���ȴ�MCU�ؑ�
            stripState.isWaitingForMCUResponse = true;
            
            PRINT("Strip Inserted (Polling)! Pin3: %d, Pin5: %d\n", stablePin3Status, stablePin5Status);
            
            // ����Pin3��Pin5�Ġ�B�A��ԇƬ���
            if(stablePin3Status == 0 && stablePin5Status == 1) {
                // ������GLV��U��ԇƬ
                PRINT("Possible Strip Type: GLV or U\n");
            } else if(stablePin3Status == 0 && stablePin5Status == 0) {
                // ������C��ԇƬ
                PRINT("Possible Strip Type: C\n");
            } else if(stablePin3Status == 1 && stablePin5Status == 0) {
                // ������TG��GAV��ԇƬ
                PRINT("Possible Strip Type: TG or GAV\n");
            }
        }
    }
    // �z��ԇƬ�γ���r - �ɂ����_���Ǹ��ƽ��ԇƬ�����Ѱγ�
    else if (stablePin3Status == 1 && stablePin5Status == 1) 
    {
        if(stripState.isStripInserted) 
        {
            // ԇƬ�γ�
            stripState.isStripInserted = false;
            stripState.stripType = STRIP_TYPE_UNKNOWN;
            stripState.isTypeDetected = false;
            stripState.isWaitingForMCUResponse = false;
            
            // ԇƬ�γ��r��V2P5_ENABLEݔ�����ƽ������늽oCH32V203
            GPIOA_ResetBits(V2P5_ENABLE_PIN);
            
            // ԇƬ�γ��r��T3_IN_SELݔ�����ƽ���P�]T3늘O
            GPIOB_SetBits(T3_IN_SEL_PIN);
            
            PRINT("Strip Removed (Polling)\n");
        }
    }
    
    // �����Ӷ��ڙz��
    tmos_start_task(StripDetect_TaskID, STRIP_PERIODIC_CHECK_EVT, MS1_TO_SYSTEM_TIME(100));
}

/*********************************************************************
 * @fn      StripDetect_SendMessage
 *
 * @brief   �l��ԇƬ���P��Ϣ��MCU
 *
 * @param   msgType - ��Ϣ���
 * @param   stripType - ԇƬ���
 *
 * @return  none
 */
static void StripDetect_SendMessage(uint8_t msgType, uint8_t stripType)
{
    uint8_t buf[6];
    
    // �M�b��Ϣ��
    buf[0] = 0xAA;                // ��ʼ��ӛ
    buf[1] = msgType;             // ��Ϣ���
    buf[2] = 0x01;                // �L�Ȟ�1
    buf[3] = stripType;           // ԇƬ���
    buf[4] = (buf[1] + buf[2] + buf[3]) % 256;  // У��
    buf[5] = 0x55;                // �Y����ӛ
    
    // �l�͵�MCU
    send_to_uart_mcu(buf, 6);
}

/*********************************************************************
 * @fn      StripDetect_SetStripType
 *
 * @brief   �O��ԇƬ��� (��MCU����ԇƬ��͕r�{��)
 *
 * @param   type - ԇƬ���
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
        
        // ����ԇƬ������������Ĝy���·
        switch(type) {
            case STRIP_TYPE_GAV:
                // GAVԇƬ��Ҫ�_��T3늘O
                GPIOB_ResetBits(T3_IN_SEL_PIN);
                PRINT("GAV Strip Confirmed, T3 Enabled\n");
                break;
                
            default:
                // ����ԇƬ���ʹ��WE늘O������T3�P�]
                GPIOB_SetBits(T3_IN_SEL_PIN);
                PRINT("Strip Type %d Confirmed, T3 Disabled\n", type);
                break;
        }
        
        // ֪ͨGUI�@ʾԇƬ���
        PRINT("Strip Type %d Confirmed\n", type);
    }
}

/*********************************************************************
 * @fn      StripDetect_IsStripInserted
 *
 * @brief   �z��ԇƬ�Ƿ����
 *
 * @param   none
 *
 * @return  bool - ԇƬ�����B
 */
bool StripDetect_IsStripInserted(void)
{
    return stripState.isStripInserted;
}

/*********************************************************************
 * @fn      StripDetect_GetStripType
 *
 * @brief   �@ȡԇƬ���
 *
 * @param   none
 *
 * @return  uint8_t - ԇƬ���
 */
uint8_t StripDetect_GetStripType(void)
{
    return stripState.stripType;
}

/*********************************************************************
 * @fn      StripDetect_GetBatteryVoltage
 *
 * @brief   �@ȡ늳�늉�
 *
 * @return  늳�늉� (mV)
 */
static uint16_t StripDetect_GetBatteryVoltage(void)
{
    return Batt_GetVoltage();
}

/*********************************************************************
 * @fn      StripDetect_SendInsertInfo
 *
 * @brief   �l��ԇƬ�����B�YӍ��MCU��ͬ�r����늳�늉���Ϣ
 *
 * @param   pin3Status - Strip_Detect_3�_λ��B
 * @param   pin5Status - Strip_Detect_5�_λ��B
 *
 * @return  none
 */
static void StripDetect_SendInsertInfo(uint8_t pin3Status, uint8_t pin5Status)
{
    uint8_t buf[9];  // ���Ӄɂ��ֹ����늳�늉�
    uint16_t battVoltage;
    
    // �y����ǰ늳�늉�
    battVoltage = StripDetect_GetBatteryVoltage();
    
    /* �M�b��Ϣ�� */
    buf[0] = 0xAA;                // ��ʼ��ӛ
    buf[1] = PROTOCOL_STRIP_INSERTED;  // ԇƬ����֪ͨ����
    buf[2] = 0x04;                // �L�Ȟ�4��2�ֹ��_λ��B + 2�ֹ�늳�늉���
    buf[3] = pin3Status;          // ��3�_��B
    buf[4] = pin5Status;          // ��5�_��B
    buf[5] = battVoltage >> 8;    // 늳�늉����ֹ�
    buf[6] = battVoltage & 0xFF;  // 늳�늉����ֹ�
    buf[7] = (buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6]) % 256;  // У��
    buf[8] = 0x55;                // �Y����ӛ
    
    /* �l�͵�MCU */
    send_to_uart_mcu(buf, 9);
    
    PRINT("Strip Insert Info Sent. Pin3=%d, Pin5=%d, BattVoltage=%dmV\n", 
          pin3Status, pin5Status, battVoltage);
}

/*********************************************************************
*********************************************************************/

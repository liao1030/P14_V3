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

    // �����Д�
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge); // Strip_Detect_3�½����Д�
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge); // Strip_Detect_5�½����Д�    
    
    // �����Д�
    PFIC_EnableIRQ(GPIO_B_IRQn);
    PFIC_EnableIRQ(GPIO_A_IRQn);
    
    // ���Ӷ��ڙz���΄�
    tmos_start_task(StripDetect_TaskID, STRIP_PERIODIC_CHECK_EVT, STRIP_CHECK_INTERVAL);
    
    // �����z��һ��ԇƬ��B���Է�ϵ�y���ӕr�Ѳ���ԇƬ
    tmos_start_task(StripDetect_TaskID, STRIP_DETECT_EVT, 1);
    
    PRINT("Strip Detect Module Initialized\n");
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
    PRINT("StripDetect_ProcessEvent called with events=0x%04X\n", events);
    
    if(events & STRIP_DETECT_EVT)
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
        
        // ��ֹż�l�Ԏ֔_���ٴδ_�JԇƬ�����B
        if(stablePin3Status == 0 || stablePin5Status == 0) 
        {
            // ������һ�����_�����ͣ��_�JԇƬ����
            if(!stripState.isStripInserted) 
            {
                // ��һ�΂ɜy��ԇƬ����
                stripState.isStripInserted = true;
                stripState.pin3Status = stablePin3Status;
                stripState.pin5Status = stablePin5Status;
                
                // ԇƬ����r��V2P5_ENABLEݔ�����ƽ����늽oCH32V203
                GPIOA_SetBits(V2P5_ENABLE_PIN);

                // �������t�΄գ��ȴ�1000ms��l��ԇƬ������Ϣ�oMCU
                // ����tmos_start_task����tmos_start_reload_task
                tmos_start_task(StripDetect_TaskID, STRIP_SEND_MESSAGE_EVT, MS1_TO_SYSTEM_TIME(1000));
                
                // �O���ȴ�MCU�ؑ�
                stripState.isWaitingForMCUResponse = true;
                
                PRINT("Strip Inserted! Pin3: %d, Pin5: %d\n", stablePin3Status, stablePin5Status);
                
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
        else 
        {
            // �ɂ����_���Ǹ��ƽ��ԇƬ�����Ѱγ�
            if(stripState.isStripInserted) 
            {
                // ԇƬ�γ�
                stripState.isStripInserted = false;
                stripState.stripType = STRIP_TYPE_UNKNOWN;
                stripState.isTypeDetected = false;
                stripState.isWaitingForMCUResponse = false;
                
                // ԇƬ�γ��r��V2P5_ENABLEݔ�����ƽ������늽oCH32V203
                GPIOA_ResetBits(V2P5_ENABLE_PIN);
                
                PRINT("Strip Removed\n");
            }
        }
        
        return (events ^ STRIP_DETECT_EVT);
    }
    
    if(events & STRIP_PERIODIC_CHECK_EVT)
    {
        // ���ж��ڙz��
        StripDetect_PeriodicCheck();
        
        return (events ^ STRIP_PERIODIC_CHECK_EVT);
    }
    
    if(events & STRIP_SEND_MESSAGE_EVT)
    {
        // �ڵȴ�100ms��l��ԇƬ������Ϣ�oMCU
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
    // �xȡ��ǰ��B
    uint8_t currentPin3Status = GPIOB_ReadPortPin(STRIP_DETECT_3_PIN) ? 1 : 0;
    uint8_t currentPin5Status = GPIOA_ReadPortPin(STRIP_DETECT_5_PIN) ? 1 : 0;
    
    // �z��ԇƬ�γ���r - �ɂ����_���Ǹ��ƽ��ԇƬ�����Ѱγ�
    if (currentPin3Status == 1 && currentPin5Status == 1) 
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
            
            PRINT("Strip Removed (Periodic Check)\n");
        }
    }
    // �z��ԇƬ������r - ������һ�����_����ƽ��ԇƬ�����Ѳ���
    else if (currentPin3Status == 0 || currentPin5Status == 0)
    {
        if(!stripState.isStripInserted)
        {
            // ��һ�΂ɜy��ԇƬ����
            stripState.isStripInserted = true;
            stripState.pin3Status = currentPin3Status;
            stripState.pin5Status = currentPin5Status;
            
            // ԇƬ����r��V2P5_ENABLEݔ�����ƽ����늽oCH32V203
            GPIOA_SetBits(V2P5_ENABLE_PIN);

            // �������t�΄գ��ȴ�1000ms��l��ԇƬ������Ϣ�oMCU
            // ����tmos_start_task����tmos_start_reload_task
            tmos_start_task(StripDetect_TaskID, STRIP_SEND_MESSAGE_EVT, MS1_TO_SYSTEM_TIME(1000));
            
            // �O���ȴ�MCU�ؑ�
            stripState.isWaitingForMCUResponse = true;
            
            PRINT("Strip Inserted (Periodic Check)! Pin3: %d, Pin5: %d\n", currentPin3Status, currentPin5Status);
            
            // ����Pin3��Pin5�Ġ�B�A��ԇƬ���
            if(currentPin3Status == 0 && currentPin5Status == 1) {
                // ������GLV��U��ԇƬ
                PRINT("Possible Strip Type: GLV or U\n");
            } else if(currentPin3Status == 0 && currentPin5Status == 0) {
                // ������C��ԇƬ
                PRINT("Possible Strip Type: C\n");
            } else if(currentPin3Status == 1 && currentPin5Status == 0) {
                // ������TG��GAV��ԇƬ
                PRINT("Possible Strip Type: TG or GAV\n");
            }
        }
    }
    
    // �����Ӷ��ڙz��
    tmos_start_task(StripDetect_TaskID, STRIP_PERIODIC_CHECK_EVT, STRIP_CHECK_INTERVAL);
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
 * @fn      StripDetect_SendInsertInfo
 *
 * @brief   �l��ԇƬ�����B�YӍ��MCU
 *
 * @param   pin3Status - Strip_Detect_3�_λ��B
 * @param   pin5Status - Strip_Detect_5�_λ��B
 *
 * @return  none
 */
static void StripDetect_SendInsertInfo(uint8_t pin3Status, uint8_t pin5Status)
{
    uint8_t buf[7];
    
    /* �M�b��Ϣ�� */
    buf[0] = 0xAA;                // ��ʼ��ӛ
    buf[1] = PROTOCOL_STRIP_INSERTED;  // ԇƬ����֪ͨ����
    buf[2] = 0x02;                // �L�Ȟ�2
    buf[3] = pin3Status;          // ��3�_��B
    buf[4] = pin5Status;          // ��5�_��B
    buf[5] = (buf[1] + buf[2] + buf[3] + buf[4]) % 256;  // У��
    buf[6] = 0x55;                // �Y����ӛ
    
    /* �l�͵�MCU */
    send_to_uart_mcu(buf, 7);
    
    PRINT("Strip Insert Info Sent. Pin3=%d, Pin5=%d\n", pin3Status, pin5Status);
}

/*********************************************************************
 * @fn      GPIOB_IRQHandler
 *
 * @brief   GPIOB�Д�̎����
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void)
{
    // �z���Ƿ��Strip_Detect_3���_�|�l���Д�
    if(GPIOB_ReadITFlagBit(STRIP_DETECT_3_PIN))
    {
        // ����Д����I
        GPIOB_ClearITFlagBit(STRIP_DETECT_3_PIN);
        
        // ӛ䛕r�g��
        stripState.insertTimeStamp = GetSysClock() / 1000;
        
        // ���ӳ������΄�
        tmos_start_task(StripDetect_TaskID, STRIP_DETECT_EVT, STRIP_DEBOUNCE_TIME);
    }
}

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA�Д�̎����
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    // �z���Ƿ��Strip_Detect_5���_�|�l���Д�
    if(GPIOA_ReadITFlagBit(STRIP_DETECT_5_PIN))
    {
        // ����Д����I
        GPIOA_ClearITFlagBit(STRIP_DETECT_5_PIN);
        
        // ӛ䛕r�g��
        stripState.insertTimeStamp = GetSysClock() / 1000;
        
        // ���ӳ������΄�
        tmos_start_task(StripDetect_TaskID, STRIP_DETECT_EVT, STRIP_DEBOUNCE_TIME);
    }
}

/*********************************************************************
*********************************************************************/

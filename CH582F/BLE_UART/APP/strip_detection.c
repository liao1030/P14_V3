/**************************************************************************************************
 * �n�����Q: strip_detection.c
 * �����f��: ԇƬ����ɜy��������������Єe�C�ƌ��F
 **************************************************************************************************/

#include "strip_detection.h"
#include "CH58x_common.h"

// ϵ�y�r犺������b����춫@ȡϵ�y����Ӌ��
static uint32_t GetSysTickCount(void)
{
    // ʹ��ϵ�y�ṩ�� SYS_GetSysTickCnt �������K�D�Q�����
    return SYS_GetSysTickCnt() / (GetSysClock() / 1000);
}

// ���x�cԇƬ�ɜy���P�ĳ���
#define STRIP_DEBOUNCE_TIME         20      // �������r�g����λms
#define STRIP_UART_BAUD             115200  // UART1�cCH32V203ͨӍ�Ĳ�����

// ���x�cԇƬ�ɜy���P��GPIO�_λ
#define STRIP_DETECT_3_PIN          GPIO_Pin_11  // PB11
#define STRIP_DETECT_5_PIN          GPIO_Pin_15  // PA15
#define V2P5_ENABLE_PIN             GPIO_Pin_14  // PA14����늽oCH32V203���ܿ���

// ���xMCU��������
#define CMD_WAKEUP_MCU              0xA1    // ����CH32V203������
#define CMD_STRIP_INSERTED          0xA2    // ֪ͨԇƬ�Ѳ��������
#define CMD_REQUEST_STRIP_TYPE      0xA3    // Ո��ԇƬ����Єe������
#define CMD_STRIP_TYPE_RESULT       0xB1    // ԇƬ����Єe�Y��������

// ���xUART�Д����Iλ
#define UART_RX_INT_FLAG            RB_IER_RECV_RDY  // UART�����Д����I

// ���xԇƬ��B
static volatile StripStatus_t stripStatus = STRIP_STATUS_NONE;
static volatile StripType_t stripType = STRIP_TYPE_UNKNOWN;

// ���������r��
static volatile uint32_t stripInsertionTime = 0;
static volatile uint8_t stripDetectionPending = 0;

// ����UART���յ��Ĕ�������ԇƬ���
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
        // ���ڴ�̎�|�l�¼�֪ͨ���Ì�
    }
}

// UART�����Д�̎����
__HIGH_CODE
void UART1_IRQHandler(void) {
    uint8_t receivedByte;
    
    if (R8_UART1_IIR & RB_IER_RECV_RDY) {
        // �ѽ��յ�����
        receivedByte = UART1_RecvByte();
        
        // ̎���CH32V203���յ�������
        if (receivedByte == CMD_STRIP_TYPE_RESULT) {
            // �ȴ�����ԇƬ��ͽY��
            while (!(R8_UART1_LSR & RB_LSR_DATA_RDY));
            receivedByte = UART1_RecvByte();
            ProcessStripTypeResponse(receivedByte);
        }
    }
}

// �z��ԇƬ�����B��������̎��
void Strip_CheckInsertionStatus(void) {
    uint8_t pin3Status, pin5Status;
    
    // ����ѽ���̎���У��z��������r�g
    if (stripDetectionPending) {
        if ((GetSysTickCount() - stripInsertionTime) >= STRIP_DEBOUNCE_TIME) {
            stripDetectionPending = 0;
            
            // �ٴΙz���_λ��B�_�J�Ƿ����Ч����
            pin3Status = GPIOB_ReadPortPin(STRIP_DETECT_3_PIN);
            pin5Status = GPIOA_ReadPortPin(STRIP_DETECT_5_PIN);
            
            // �z���Ƿ�����һ���c�����ͣ���ʾԇƬ���룩
            if (!pin3Status || !pin5Status) {
                // ԇƬ�_�����룬��� �B
                stripStatus = STRIP_STATUS_INSERTED;
                
                // ����CH32V203
                GPIOA_SetBits(V2P5_ENABLE_PIN);  // �_��CH32V203�Դ
                DelayMs(10);  // �ȴ��Դ����
                
                // �l�͆�������oCH32V203
                UART1_SendByte(CMD_WAKEUP_MCU);
                DelayMs(5);
                
                // ֪ͨԇƬ�Ѳ���KՈ���Єe���
                UART1_SendByte(CMD_STRIP_INSERTED);
                DelayMs(5);
                UART1_SendByte(CMD_REQUEST_STRIP_TYPE);
                
                // �M��yԇ�ʂ��B���ȴ�CH32V203�ؑ�ԇƬ���
            }
        }
    }
}

// �ⲿ�Д�̎����
__HIGH_CODE
void GPIO_IRQHandler(void) {
    // �z���Ƿ���Strip_Detect_3�Д�
    if (GPIOB_ReadITFlagBit(STRIP_DETECT_3_PIN)) {
        GPIOB_ClearITFlagBit(STRIP_DETECT_3_PIN);
        
        // ֻ̎���½��أ�ԇƬ���룩
        if (stripStatus == STRIP_STATUS_NONE) {
            // ӛ䛕r�g�����_ʼ������
            stripInsertionTime = GetSysTickCount();
            stripDetectionPending = 1;
        }
    }
    
    // �z���Ƿ���Strip_Detect_5�Д�
    if (GPIOA_ReadITFlagBit(STRIP_DETECT_5_PIN)) {
        GPIOA_ClearITFlagBit(STRIP_DETECT_5_PIN);
        
        // ֻ̎���½��أ�ԇƬ���룩
        if (stripStatus == STRIP_STATUS_NONE) {
            // ӛ䛕r�g�����_ʼ������
            stripInsertionTime = GetSysTickCount();
            stripDetectionPending = 1;
        }
    }
}

// ��ʼ��ԇƬ����ɜy
void Strip_Detection_Init(void) {
    // ����Strip_Detect_3 (PB11)������ݔ�룬�K�����½����Д�
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ(GPIO_B_IRQn);
    
    // ����Strip_Detect_5 (PA15)������ݔ�룬�K�����½����Д�
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ(GPIO_A_IRQn);
    
    // ����V2P5_ENABLE (PA14)������ݔ������ʼ����λ
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(V2P5_ENABLE_PIN); // ��ʼ�P�]CH32V203�Դ
    
    // ��ʼ��UART1������cCH32V203ͨӍ
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    UART1_BaudRateCfg(STRIP_UART_BAUD);
    
    // ����UART1�����Д�
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
    
    // ��ʼ����B׃��
    stripStatus = STRIP_STATUS_NONE;
    stripType = STRIP_TYPE_UNKNOWN;
    stripDetectionPending = 0;
}

// �@ȡԇƬ��B
StripStatus_t Strip_GetStatus(void) {
    return stripStatus;
}

// �@ȡԇƬ���
StripType_t Strip_GetType(void) {
    return stripType;
}

// �O��ԇƬ���
void Strip_SetType(StripType_t type) {
    stripType = type;
    if (type != STRIP_TYPE_UNKNOWN) {
        stripStatus = STRIP_STATUS_READY;
    }
}

// �yԇ���̎����
void Strip_TestComplete(void) {
    stripStatus = STRIP_STATUS_COMPLETE;
    // ���ڴ�̎��Ӝyԇ������̎��߉݋
}
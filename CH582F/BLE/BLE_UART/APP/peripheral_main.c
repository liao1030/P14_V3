/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.1
 * Date               : 2024/08/10
 * Description        : P14�Ŀ����ʽ������ԇƬ����ɜy�c����Єe����
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

/******************************************************************************/
/* �^�ļ����� */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "app_uart.h"
#include "P14_Init.h"
#include "P14_BLE_Protocol.h"  // ����{���f�h�^�ļ�

/* �ⲿ������ */
uint32_t millis(void);  // �@ȡϵ�y�r�g(����)

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/* ȫ��׃�� */
static volatile uint32_t g_systemTicks = 0;  // ϵ�y�r�gӋ��(����)
static volatile uint32_t g_lastStripCheckTime = 0;

/* �������� */
void BLE_Protocol_Init(void);
static void RegisterBLECallback(void);
void P14_StripDetection_Init(void);
void P14_StripDetection_Process(void);
void SysTick_Init(void);

/**
 * @brief �@ȡϵ�y�r�g(����)
 * 
 * @return uint32_t ϵ�y�\�Еr�g(����)
 */
uint32_t millis(void)
{
    return g_systemTicks;
}

/**
 * @brief SysTick�Д�̎����
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
void SysTick_Handler(void)
{
    g_systemTicks++;
    
    /* ����Д����I */
    SysTick->SR = 0;
}

/**
 * @brief ��ʼ��SysTick���r��
 */
void SysTick_Init(void)
{
    /* �P�]SysTick���r�� */
    SysTick->CTLR = 0;
    
    /* ���Ӌ�������Д����I */
    SysTick->CNT = 0;
    SysTick->SR = 0;
    
    /* �O�����dֵ��1ms�Д� */
    SysTick->CMP = FREQ_SYS / 1000;  // ʹ��CH58x�ж��x��ϵ�y�l��
    
    /* ʹ��SysTick�Д� */
    PFIC_EnableIRQ(SysTick_IRQn);
    
    /* ʹ��SysTick���r����ʹ��ϵ�y�r犡��Ԅ����d��ʹ���Дࡢʹ��Ӌ���� */
    SysTick->CTLR = SysTick_CTLR_INIT | SysTick_CTLR_STRE | SysTick_CTLR_STCLK | SysTick_CTLR_STIE | SysTick_CTLR_STE;
    
    PRINT("SysTick Timer Initialized at 1ms interval\r\n");
}

/**
 * @brief �]���{�����{����
 */
static void RegisterBLECallback(void)
{
    // �]���{��UART���{����
    ble_uart_set_callback(P14_BLE_UartCallback);
}

/**
 * @brief ��ʼ���{���f�h̎��ģ�K
 */
void BLE_Protocol_Init(void)
{
    // ��ʼ���{���f�h��B�C
    // �����fޒ�{��
    P14_BLE_ProtocolInit();
}

/**
 * @brief ��ʼ��ԇƬ�ɜy����
 */
void P14_StripDetection_Init(void)
{
    /* ��ʼ��ԇƬ����ɜy */
    P14_StripDetectInit();
    
    /* ��ʼ��Ӌ�r�� */
    g_lastStripCheckTime = 0;
    
    PRINT("Strip Detection Initialized\r\n");
}

/**
 * @brief ԇƬ����̎������
 */
void P14_StripDetection_Process(void)
{
    uint32_t currentTime = millis();
    static uint8_t processingFlag = 0;  // ����̎��ԇƬ�ɜy�Ę��I
    static uint32_t lastFailTime = 0;   // �ϴ΂ɜyʧ���ĕr�g
    static uint8_t failCount = 0;       // �B�mʧ���Δ�
    static uint8_t retryCount = 0;      // ��ԇ�Δ�
    
    /* �z���Ƿ��ѵ��z���g���r�g����δ̎�̎���Р�B */
    if ((currentTime - g_lastStripCheckTime >= STRIP_DETECT_INTERVAL) && !processingFlag) {
        g_lastStripCheckTime = currentTime;
        
        /* �z���Ƿ���ԇƬ���� */
        if (P14_CheckStripInsertion()) {
            /* �O��̎���И��I���������}�|�l */
            processingFlag = 1;
            
            PRINT("ԇƬ����ɜy�ɹ���\r\n");
            
            /* �Д�ԇƬ���ǰ�_��LED��ʾ */
            GPIOA_SetBits(GPIO_Pin_12);  // ���OPA12��LEDָʾ��
            
            /* �Д�ԇƬ��� */
            StripType_TypeDef type = P14_IdentifyStripType();
            
            /* �P�]LED��ʾ */
            GPIOA_ResetBits(GPIO_Pin_12);
            
            /* �@ʾԇƬ��� */
            PRINT("�R�e����ԇƬ���: ");
            switch (type) {
                case STRIP_TYPE_GLV:
                    PRINT("GLV (Ѫ��)\r\n");
                    break;
                case STRIP_TYPE_U:
                    PRINT("U (����)\r\n");
                    break;
                case STRIP_TYPE_C:
                    PRINT("C (��đ�̴�)\r\n");
                    break;
                case STRIP_TYPE_TG:
                    PRINT("TG (�������֬)\r\n");
                    break;
                case STRIP_TYPE_GAV:
                    PRINT("GAV (Ѫ��V2)\r\n");
                    break;
                default:
                    PRINT("δ֪\r\n");
                    break;
            }
            
            /* �z���Ƿ�ɹ��R�eԇƬ��� */
            if (type != STRIP_TYPE_UNKNOWN) {
                /* ֪ͨCH32V203ԇƬ��� */
                P14_NotifyStripInserted(type);
                PRINT("��֪ͨCH32V203ԇƬ���\r\n");
                
                /* ���O̎���И��I���e�`Ӌ�� */
                processingFlag = 0;
                failCount = 0;
                retryCount = 0;
            } else {
                /* ԇƬ����R�eʧ�� */
                lastFailTime = currentTime;
                
                /* ����ʧ��Ӌ�� */
                failCount++;
                
                /* �Д��Ƿ���Ҫ��ԇ */
                if (retryCount < 3) {
                    PRINT("ԇƬ����R�eʧ������%d����ԇ��...\r\n", retryCount + 1);
                    retryCount++;
                    
                    /* �̕����t���ٴ·Lԇ */
                    DelayMs(200);
                    
                    /* ����̎���Р�B�������S�����R�e */
                    processingFlag = 0;
                } else {
                    /* ���^�����ԇ�Δ� */
                    PRINT("ԇƬ����R�eʧ����Ո���²���ԇƬ\r\n");
                    
                    /* ���OԇƬ��B */
                    P14_StripStateReset();
                    
                    /* ���O��ԇӋ�� */
                    retryCount = 0;
                    processingFlag = 0;
                    
                    /* ����B�mʧ���Δ��^�࣬��ʾ������Ӳ�w���} */
                    if (failCount > 5) {
                        PRINT("���棺�B�m����R�eʧ����Ո�z��x����ԇƬ\r\n");
                    }
                }
            }
        }
    }
    
    /* ���ڙz���Ƿ���Ҫ���̎���И��I */
    if (processingFlag && (currentTime - lastFailTime > 5000)) {
        /* 5�����Ԅ����̎���И��I����ֹ���� */
        processingFlag = 0;
        retryCount = 0;
        PRINT("ԇƬ�R�e�������r���Ԅ����O\r\n");
        P14_StripStateReset();
    }
    
    /* ������B�mʧ�����^10�Σ��g����һ�c����ʾ���ܵ�Ӳ�w���} */
    if (failCount > 10 && (currentTime % 60000) < 1000) {  // ÿ�����ʾһ��
        PRINT("ϵ�y���棺ԇƬ�R�e�B�mʧ���Δ��^�࣬Ո�z���O���B��\r\n");
    }
}

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : ��ѭ�h
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
        app_uart_process();
        
        /* ԇƬ����ɜy̎�� */
        P14_StripDetection_Process();
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : ������
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    /* P14 V2.1 ϵ�y��ʼ�� */
    P14_CH582F_System_Init();
    
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    PRINT("P14 V2.1 System Starting...\r\n");
    
    /* ��ʼ��SysTick���r�� */
    SysTick_Init();
    
    /* BLE���P��ʼ�� */
    CH58X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    
    /* ��ʼ��P14�{���f�h */
    BLE_Protocol_Init();
    PRINT("P14 BLE Protocol Initialized\r\n");
    
    /* ��ʼ��UART */
    app_uart_init();
    PRINT("UART Initialized\r\n");
    
    /* ��ʼ��ԇƬ�ɜy */
    P14_StripDetection_Init();
    
    /* �]��BLE���{���� */
    RegisterBLECallback();
    
    PRINT("P14 V2.1 System Ready\r\n");
    
    /* �M����ѭ�h */
    Main_Circulation();
}

/******************************** endfile @ main ******************************/

/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : HMD
 * Version            : V1.1.0
 * Date               : 2025/04/28
 * Description        : CH582F����ʽ
 *********************************************************************************
 * Copyright (c) 2025 HMD Corporation.
 *******************************************************************************/

/******************************************************************************/
/* �^�ļ����� */
#include "CH58x_common.h"
#include "CH58xBLE_LIB.h"
#include "ble_uart_protocol.h"

/* ȫ�ֶ��x */
#define STRIP_DETECT_3_PIN     GPIO_Pin_11      /* PB11 ԇƬ����z�y */
#define T3_IN_SEL_PIN          GPIO_Pin_10      /* PB10 T3늘O���y���ܿ��� */
#define STRIP_DETECT_5_PIN     GPIO_Pin_15      /* PA15 ԇƬ����z�y */
#define V2P5_ENABLE_PIN        GPIO_Pin_14      /* PA14 ��늽oCH32V203���ܿ��� */
#define V_BACK_C_PIN           GPIO_Pin_13      /* PA13 �ⲿUART�O��Rx Level Shift�Դ�x����� */
#define VBUS_GET_PIN           GPIO_Pin_12      /* PA12 ���y�ⲿ�O���Ƿ�� */

/* UART���P���n�^ */
#define UART_RX_BUF_SIZE       128
static uint8_t uart1_rx_buf[UART_RX_BUF_SIZE];
static uint16_t uart1_rx_len = 0;

/* BLE���P���� */
#define BLE_DEVICE_NAME_BASE   "P14-"
static uint8_t ble_mac[6];
static uint8_t ble_device_name[20];

/* ������ */
void SystemInit(void);
void GPIO_Init(void);
void GPIO_INT_Init(void);
void UART_Init(void);
void BLE_Init(void);
void BLE_ProcessEvent(void);
void Timer_Init(void);
void Process_UART_Data(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main(void)
{
    /* ϵ�y������ʼ�� */
    SystemInit();
    
    /* GPIO��ʼ�� */
    GPIO_Init();
    
    /* GPIO�Д��ʼ�� */
    GPIO_INT_Init();
    
    /* UART��ʼ�� */
    UART_Init();
    
    /* �f�h��ʼ�� */
    BLE_Protocol_Init();
    
    /* BLE��ʼ�� */
    BLE_Init();
    
    /* ���r����ʼ�� */
    Timer_Init();
    
    /* �����yԇݔ�� */
    PRINT("P14_V2.1 BLE ��ʼ�����\n");
    PRINT("ϵ�y�r�: %d MHz\n", GetSysClock() / 1000000);
    PRINT("�O�����Q: %s\n", ble_device_name);
    
    /* ��ѭ�h */
    while(1)
    {
        /* ̎��BLE�¼� */
        BLE_ProcessEvent();
        
        /* ̎��UART���Ք��� */
        Process_UART_Data();
        
        /* ϵ�y���� */
        __WFI();
    }
}

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   ϵ�y�r犳�ʼ��
 *
 * @return  none
 */
void SystemInit(void)
{
    /* ������O�}λ */
    PERIPHERAL_RESET();
    
    /* ϵ�y�r犳�ʼ�� */
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    /* ϵ�y�Д��ʼ�� */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* �_���r� */
    GPIOA_ModeCfg(PIN_WAKEUP_GPIO_ALL, Pin_ModeIN_PD);
    LowPower_Idle();
}

/*********************************************************************
 * @fn      GPIO_Init
 *
 * @brief   GPIO��ʼ������
 *
 * @return  none
 */
void GPIO_Init(void)
{
    /* ԇƬ����z�y���_ - PB11 */
    GPIOB_ModeCfg(STRIP_DETECT_3_PIN, GPIO_ModeIN_PU);
    
    /* T3늘O���y���ܿ��� - PB10 (���ƽ����) */
    GPIOB_ModeCfg(T3_IN_SEL_PIN, GPIO_ModeOut_PP_5mA);
    
    /* ԇƬ����z�y���_ - PA15 */
    GPIOA_ModeCfg(STRIP_DETECT_5_PIN, GPIO_ModeIN_PU);
    
    /* ��늽oCH32V203���ܿ��� - PA14 (���ƽ����) */
    GPIOA_ModeCfg(V2P5_ENABLE_PIN, GPIO_ModeOut_PP_5mA);
    
    /* �ⲿUART�O��Rx Level Shift�Դ�x����� - PA13 */
    GPIOA_ModeCfg(V_BACK_C_PIN, GPIO_ModeOut_PP_5mA);
    
    /* ���y�ⲿ�O���Ƿ�� - PA12 */
    GPIOA_ModeCfg(VBUS_GET_PIN, GPIO_ModeIN_Floating);
    
    /* ��ʼ�O��ݔ����B */
    GPIOB_ResetBits(T3_IN_SEL_PIN);     /* T3늘O���y����(���ƽ) */
    GPIOA_SetBits(V2P5_ENABLE_PIN);     /* ��늽oCH32V203����(���ƽ) */
    GPIOA_ResetBits(V_BACK_C_PIN);      /* UART Level Shift�Դ�x��(���ƽ) */
}

/*********************************************************************
 * @fn      GPIO_INT_Init
 *
 * @brief   GPIO�Д��ʼ������
 *
 * @return  none
 */
void GPIO_INT_Init(void)
{
    /* ����ԇƬ����z�y���_�Д� - PB11 */
    GPIOB_ITModeCfg(STRIP_DETECT_3_PIN, GPIO_ITMode_FallEdge);
    
    /* ����ԇƬ����z�y���_�Д� - PA15 */
    GPIOA_ITModeCfg(STRIP_DETECT_5_PIN, GPIO_ITMode_FallEdge);
    
    /* ����GPIO�Д� */
    PFIC_EnableIRQ(GPIO_IRQn);
}

/*********************************************************************
 * @fn      UART_Init
 *
 * @brief   UART��ʼ������
 *
 * @return  none
 */
void UART_Init(void)
{
    /* UART1��ʼ�� - �cCH32V203ͨӍ */
    GPIOA_SetBits(GPIO_Pin_8);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);         /* RX (PA8) */
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);    /* TX (PA9) */
    
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    
    /* ��ս��վ��n�^ */
    uart1_rx_len = 0;
    
    /* ����UART1�Д���� */
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY);
    PFIC_EnableIRQ(UART1_IRQn);
}

/*********************************************************************
 * @fn      Timer_Init
 *
 * @brief   ���r����ʼ��
 *
 * @return  none
 */
void Timer_Init(void)
{
    /* ��ʼ�����r��3, 10ms�Д� */
    TMR3_TimerInit(GetSysClock() / 100);
    TMR3_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR3_IRQn);
}

/*********************************************************************
 * @fn      Process_UART_Data
 *
 * @brief   ̎��UART���յ��Ĕ���
 *
 * @return  none
 */
void Process_UART_Data(void)
{
    /* ����Д�����Ҫ̎�� */
    if (uart1_rx_len > 0) {
        /* ͨ�^�f�h̎�� */
        BLE_Protocol_Parse(uart1_rx_buf, uart1_rx_len);
        
        /* ��վ��n�^ */
        uart1_rx_len = 0;
    }
}

/*********************************************************************
 * @fn      BLE_Init
 *
 * @brief   BLE��ʼ��
 *
 * @return  none
 */
void BLE_Init(void)
{
    uint8_t i;
    
    /* �@ȡMAC��ַ */
    GetMACAddress(ble_mac);
    
    /* �����O�����Q P14-XXXX (MAC��ַ����3�ֹ�) */
    sprintf((char*)ble_device_name, "%s%02X%02X%02X", BLE_DEVICE_NAME_BASE, 
            ble_mac[3], ble_mac[4], ble_mac[5]);
    
    /* BLE���ó�ʼ�� */
    BLE_LibInit();
    
    /* �V����ʼ�� */
    GAPRole_PeripheralInit();
    
    /* �O���O�����Q */
    GAP_SetDeviceName((uint8_t *)ble_device_name);
    
    /* �O�ÏV������ */
    {
        uint8_t adv_data[31];
        uint8_t adv_len = 0;
        
        adv_data[adv_len++] = 0x02;     // �L��
        adv_data[adv_len++] = 0x01;     // ���: Flags
        adv_data[adv_len++] = 0x06;     // ֵ: LE General Discoverable
        
        adv_data[adv_len++] = strlen((char*)ble_device_name) + 1; // �L��
        adv_data[adv_len++] = 0x09;     // ���: Complete Local Name
        for (i = 0; i < strlen((char*)ble_device_name); i++) {
            adv_data[adv_len++] = ble_device_name[i];
        }
        
        adv_data[adv_len++] = 0x03;     // �L��
        adv_data[adv_len++] = 0x02;     // ���: UUID List
        adv_data[adv_len++] = BLE_SERVICE_UUID & 0xFF;
        adv_data[adv_len++] = (BLE_SERVICE_UUID >> 8) & 0xFF;
        
        /* �O�ÏV������ */
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, adv_len, adv_data);
    }
    
    /* �O�ÏV���g�� */
    {
        uint16_t adv_interval = 160;    // 100ms (160 * 0.625ms)
        GAPRole_SetParameter(GAPROLE_ADV_EVENT_INTERVAL, sizeof(adv_interval), &adv_interval);
    }
    
    /* �O�÷��� */
    {
        /* �Զ��x���� */
        uint8_t service_uuid[2] = {BLE_SERVICE_UUID & 0xFF, (BLE_SERVICE_UUID >> 8) & 0xFF};
        uint8_t rx_uuid[2] = {BLE_CHAR_RX_UUID & 0xFF, (BLE_CHAR_RX_UUID >> 8) & 0xFF};
        uint8_t tx_uuid[2] = {BLE_CHAR_TX_UUID & 0xFF, (BLE_CHAR_TX_UUID >> 8) & 0xFF};
        
        /* ע���Զ��x���� */
        BLE_RegisterService(service_uuid);
        
        /* ע������ */
        BLE_RegisterCharacteristic(rx_uuid, GATT_PERMIT_WRITE);
        BLE_RegisterCharacteristic(tx_uuid, GATT_PERMIT_READ | GATT_PROP_NOTIFY);
    }
    
    /* �_ʼ�V�� */
    GAPRole_PeripheralStartDiscovery();
    
    PRINT("BLE initialized: %s\n", ble_device_name);
}

/*********************************************************************
 * @fn      BLE_ProcessEvent
 *
 * @brief   ̎��BLE�¼�
 *
 * @return  none
 */
void BLE_ProcessEvent(void)
{
    /* ̎��BLE�¼� */
    BLE_LibProcessEvents();
}

/*********************************************************************
 * @fn      TMR3_IRQHandler
 *
 * @brief   TMR3�Д�̎���� (10ms)
 *
 * @return  none
 */
__INTERRUPT void TMR3_IRQHandler(void)
{
    /* ����Д����I */
    TMR3_ClearITFlag(TMR0_3_IT_CYC_END);
    
    /* ̎���؂����r�ȶ��r�΄� */
    static uint16_t retry_counter = 0;
    
    retry_counter++;
    
    /* �z�y10ms�΄� */
    
    /* �z�y100ms�΄� */
    if (retry_counter % 10 == 0) {
        
    }
    
    /* �z�y1000ms�΄� */
    if (retry_counter % 100 == 0) {
        /* ÿ���W�qLED�����������΄� */
    }
}

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1�Д�̎����
 *
 * @return  none
 */
__INTERRUPT void UART1_IRQHandler(void)
{
    uint8_t rx_data;
    
    /* �����Д� */
    if (UART1_GetITFlag(RB_IER_RECV_RDY)) {
        /* �xȡ���Ք��� */
        rx_data = UART1_RecvByte();
        
        /* ���뾏�n�^ */
        if (uart1_rx_len < UART_RX_BUF_SIZE) {
            uart1_rx_buf[uart1_rx_len++] = rx_data;
        }
        
        /* ����Д����I */
        UART1_ClearITFlag(RB_IER_RECV_RDY);
    }
}

/*********************************************************************
 * @fn      GPIO_IRQHandler
 *
 * @brief   GPIO�Д�̎����
 *
 * @return  none
 */
__INTERRUPT void GPIO_IRQHandler(void)
{
    /* �z�yԇƬ���� - PB11 */
    if (GPIOB_ReadITFlagBit(STRIP_DETECT_3_PIN)) {
        PRINT("Strip detected on PB11!\n");
        
        /* ����Д����I */
        GPIOB_ClearITFlagBit(STRIP_DETECT_3_PIN);
    }
    
    /* �z�yԇƬ���� - PA15 */
    if (GPIOA_ReadITFlagBit(STRIP_DETECT_5_PIN)) {
        PRINT("Strip detected on PA15!\n");
        
        /* ����Д����I */
        GPIOA_ClearITFlagBit(STRIP_DETECT_5_PIN);
    }
}

/*********************************************************************
 * @fn      BLE_GAPRole_EventCallBack
 *
 * @brief   BLE GAP��ɫ�¼����{
 *
 * @param   event - �¼����
 * @param   data - �¼�����
 *
 * @return  none
 */
void BLE_GAPRole_EventCallBack(uint8_t event, uint8_t *data)
{
    switch (event) {
        case GAPROLE_CONNECTED:
            /* BLE���B�� */
            PRINT("BLE Connected\n");
            BLE_SetConnectedState(1);
            break;
            
        case GAPROLE_DISCONNECTED:
            /* BLE�є��_ */
            PRINT("BLE Disconnected\n");
            BLE_SetConnectedState(0);
            
            /* �����_ʼ�V�� */
            GAPRole_PeripheralStartDiscovery();
            break;
            
        default:
            break;
    }
}

/*********************************************************************
 * @fn      BLE_Characteristic_EventCallBack
 *
 * @brief   BLE�����¼����{
 *
 * @param   char_uuid - ����UUID
 * @param   event - �¼����
 * @param   data - �¼�����
 * @param   length - �����L��
 *
 * @return  none
 */
void BLE_Characteristic_EventCallBack(uint16_t char_uuid, uint8_t event, uint8_t *data, uint16_t length)
{
    switch (char_uuid) {
        case BLE_CHAR_RX_UUID:
            if (event == GATT_CHAR_WRITE_EVENT) {
                /* ���յ�BLE���딵�� */
                PRINT("BLE RX Data: %d bytes\n", length);
                
                /* �������� */
                BLE_Protocol_Parse(data, length);
            }
            break;
            
        default:
            break;
    }
} 
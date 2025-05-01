/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *P14 V2.1 ����ʽ:
 *�@������չʾ��P14 V2.1�Ŀ������ʽ��
 */

#include "debug.h"
#include "P14_Init.h"

/* Global typedef */

/* Global define */
#define KEY1_PRESSED() (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
#define KEY2_PRESSED() (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)

/* ���I�������� */
#define KEY_DEBOUNCE_MS       20      // �����ӕr�r�g(����)
#define KEY_LONG_PRESS_MS     1000    // �L���|�l�r�g(����)

/* ���I��Bö�e */
typedef enum {
    KEY_STATE_IDLE,           // ���f��B
    KEY_STATE_DEBOUNCE,       // ������B
    KEY_STATE_SHORT_PRESS,    // �̰��_�J
    KEY_STATE_LONG_PRESS      // �L���_�J
} KeyState_TypeDef;

/* ���I�Y���w */
typedef struct {
    KeyState_TypeDef state;   // ���I��ǰ��B
    uint32_t timestamp;       // ��B�ГQ�r�g��
    uint8_t pressed;          // ��ǰ�Ƿ���
    uint8_t longPressed;      // �Ƿ��|�l�L��
} Key_TypeDef;

/* Global Variable */
volatile uint32_t g_systicks = 0;  /* ��ȫ��׃������volatile���Ա����Д�̎��������޸� */
Key_TypeDef g_key1 = {KEY_STATE_IDLE, 0, 0, 0};
Key_TypeDef g_key2 = {KEY_STATE_IDLE, 0, 0, 0};

/*********************************************************************
 * @fn      TestParameterTable
 *
 * @brief   �yԇ��������
 *
 * @return  none
 */
void TestParameterTable(void)
{
    BasicSystemBlock basicParams;
    
    /* �xȡ��ǰ����ϵ�y���� */
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("�xȡ����ϵ�y����ʧ��\r\n");
        return;
    }
    
    /* �ГQ�yԇ�Ŀ��� */
    StripType_TypeDef currentType = (StripType_TypeDef)basicParams.stripType;
    StripType_TypeDef newType = (currentType + 1) % 5; // ѭ�h�ГQ0-4֮�g��ֵ
    
    /* ������ */
    basicParams.stripType = (uint8_t)newType;
    
    /* ���������ą��� */
    if (!PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        printf("���»���ϵ�y����ʧ��\r\n");
        return;
    }
    
    /* �@ʾ�ГQ����Ŀ */
    printf("�yԇ�Ŀ�ГQ��: %s\r\n", P14_ParamTable_GetStripTypeName(newType));
    
    /* �yԇӛ䛃��湦�� */
    uint16_t randomValue = g_systicks % 300 + 100;  // ģ�M�y��ֵ (100-399)
    if (PARAM_SaveTestRecord(randomValue, 0, basicParams.defaultEvent, 0, 3000, 250)) {
        printf("����yԇӛ䛳ɹ����y��ֵ: %d\r\n", randomValue);
    } else {
        printf("����yԇӛ�ʧ��\r\n");
    }
    
    /* �@ʾ��ǰ�yԇ�Δ� */
    uint16_t testCount = PARAM_GetTestCount();
    printf("��ǰ�yԇ�Δ�: %d\r\n", testCount);
    
    /* �@ʾ�yԇӛ䛿��� */
    uint16_t recordCount = PARAM_GetTestRecordCount();
    printf("�yԇӛ䛿���: %d\r\n", recordCount);
    
    /* ����Мyԇӛ䛣��@ʾ���µ�һ�l */
    if (recordCount > 0) {
        TestRecord record;
        if (PARAM_GetTestRecord(0, &record)) {
            printf("���yԇӛ�: ���=%d, �Y��=%d, �¼�=%d, �r�g=%02d-%02d-%02d %02d:%02d:%02d\r\n",
                   record.testType, record.resultValue, record.event,
                   record.year, record.month, record.date,
                   record.hour, record.minute, record.second);
        }
    }
}

/*********************************************************************
 * @fn      InitSysTick
 *
 * @brief   ��ʼ��ϵ�yӋ�r��
 *
 * @return  none
 */
void InitSysTick(void)
{
    /* �O��SysTick��1ms�Д� */
    SysTick->CTLR = 0;
    SysTick->SR = 0;
    SysTick->CNT = 0;
    SysTick->CMP = SystemCoreClock / 1000;  /* 1ms�Д�һ�� */
    
    /* ʹ��Ӌ�������Д� */
    SysTick->CTLR = 0x0F;  /* ʹ��Ӌ�������Д࣬�Kʹ��ϵ�y�r� */
}

/*********************************************************************
 * @fn      UpdateKeyState
 *
 * @brief   ���°��I��B�C
 *
 * @param   key - ���I�Y���wָ�
 * @param   isPressed - ��ǰ�Ƿ���
 *
 * @return  1: ���I�l����B׃��, 0: �o׃��
 */
uint8_t UpdateKeyState(Key_TypeDef *key, uint8_t isPressed)
{
    uint8_t stateChanged = 0;
    uint32_t currentTime = g_systicks;
    
    switch (key->state) {
        case KEY_STATE_IDLE:
            if (isPressed) {
                key->state = KEY_STATE_DEBOUNCE;
                key->timestamp = currentTime;
            }
            break;
            
        case KEY_STATE_DEBOUNCE:
            if (isPressed) {
                if (currentTime - key->timestamp >= KEY_DEBOUNCE_MS) {
                    key->state = KEY_STATE_SHORT_PRESS;
                    key->pressed = 1;
                    stateChanged = 1;
                }
            } else {
                key->state = KEY_STATE_IDLE;
            }
            break;
            
        case KEY_STATE_SHORT_PRESS:
            if (isPressed) {
                if (currentTime - key->timestamp >= KEY_LONG_PRESS_MS) {
                    key->state = KEY_STATE_LONG_PRESS;
                    key->longPressed = 1;
                    stateChanged = 1;
                }
            } else {
                key->state = KEY_STATE_IDLE;
                key->pressed = 0;
                stateChanged = 1;
            }
            break;
            
        case KEY_STATE_LONG_PRESS:
            if (!isPressed) {
                key->state = KEY_STATE_IDLE;
                key->pressed = 0;
                key->longPressed = 0;
                stateChanged = 1;
            }
            break;
            
        default:
            key->state = KEY_STATE_IDLE;
            break;
    }
    
    return stateChanged;
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    uint8_t key1Changed = 0;
    uint8_t key2Changed = 0;
    
    /* ϵ�y��ʼ�� */
    P14_CH32V203_System_Init();
    
    /* ��ʼ��ϵ�yӋ�r�� */
    InitSysTick();
    
    printf("P14 V2.1 ϵ�y�ц���\r\n");
    printf("SystemClk: %d Hz\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());
    
    /* �_���GɫLED */
    GPIO_SetBits(GPIOB, GPIO_Pin_7);
    
    /* ��ѭ�h */
    while(1)
    {
        /* ���°��I��B */
        key1Changed = UpdateKeyState(&g_key1, KEY1_PRESSED());
        key2Changed = UpdateKeyState(&g_key2, KEY2_PRESSED());
        
        /* ̎��KEY1���I���� */
        if (key1Changed && g_key1.pressed) {
            printf("�z�y��KEY1���I����\r\n");
            
            /* �ГQ�tɫLED��B */
            GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_6)));
        }
        
        if (key1Changed && g_key1.longPressed) {
            printf("�z�y��KEY1�L��\r\n");
            /* �L��̎��߉݋���Է����@�e */
        }
        
        /* ̎��KEY2���I���� */
        if (key2Changed && g_key2.pressed) {
            printf("�z�y��KEY2���I����\r\n");
            
            /* �yԇ�������� */
            TestParameterTable();
        }
        
        if (key2Changed && g_key2.longPressed) {
            printf("�z�y��KEY2�L��\r\n");
            /* �L��̎��߉݋���Է����@�e */
        }
        
        /* ϵ�y�ӕr */
        Delay_Ms(10);
        
        /* �GɫLED�W�q����ϵ�y�\��ָʾ */
        if ((g_systicks / 1000) % 2 == 0) {
            GPIO_SetBits(GPIOB, GPIO_Pin_7);
        } else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
        }
    }
}

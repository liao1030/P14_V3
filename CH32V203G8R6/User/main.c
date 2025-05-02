/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH / HMD
 * Version            : V1.0.0
 * Date               : 2024/09/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Copyright (c) 2024 HMD Biomedical Inc.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *�๦�������yԇ�xFlash�������淽����ʽ:
 *��ʾ����ʽ��ʾ��γ�ʼ�����xȡ�ͱ���๦�������yԇ�x�ą����͜yԇӛ䛡�
 *USART1_Tx(PA9) - ���ݔ���{ԇ��Ϣ
 *
 */

#include "debug.h"
#include "param_storage.h"
#include <stdbool.h>

/* ȫ��׃�����x */
static BasicSystemBlock g_basicParams;
static HardwareCalibBlock g_calibParams;
static BloodGlucoseBlock g_bgParams;
static TestRecord g_testRecord;

/*********************************************************************
 * @fn      printBasicSystemInfo
 *
 * @brief   ��ӡ����ϵ�y������Ϣ
 *
 * @param   basicParams - ����ϵ�y����ָ�
 *
 * @return  none
 */
void printBasicSystemInfo(const BasicSystemBlock *basicParams)
{
    printf("\r\n====== �๦�������yԇ�x����������Ϣ ======\r\n");
    printf("�g�w�汾: V%d.%d\r\n", basicParams->fwNo / 10, basicParams->fwNo % 10);
    printf("���a��汾: %d\r\n", basicParams->codeTableVer);
    printf("�yԇ�Δ�: %d\r\n", basicParams->testCount);
    printf("����ģʽ: %s\r\n", (basicParams->factory == 0) ? "ʹ����ģʽ" : "���Sģʽ");
    
    printf("\r\n-- ���ڕr�g�O�� --\r\n");
    printf("����: 20%02d/%02d/%02d\r\n", basicParams->year, basicParams->month, basicParams->date);
    printf("�r�g: %02d:%02d:%02d\r\n", basicParams->hour, basicParams->minute, basicParams->second);
    
    printf("\r\n-- �yԇ�h������ --\r\n");
    printf("�����ضȹ���: %d��C ~ %d��C\r\n", basicParams->tempLowLimit, basicParams->tempHighLimit);
    
    char *unit_str = "δ֪";
    switch(basicParams->measureUnit) {
        case UNIT_MMOL_L: unit_str = "mmol/L"; break;
        case UNIT_MG_DL: unit_str = "mg/dL"; break;
        case UNIT_GM_DL: unit_str = "gm/dl"; break;
    }
    printf("�y����λ: %s\r\n", unit_str);
    
    char *strip_type_str = "δ֪";
    switch(basicParams->stripType) {
        case STRIP_TYPE_GLV: strip_type_str = "Ѫ��(GLV)"; break;
        case STRIP_TYPE_UA: strip_type_str = "����(U)"; break;
        case STRIP_TYPE_CHOL: strip_type_str = "��đ�̴�(C)"; break;
        case STRIP_TYPE_TG: strip_type_str = "�������֬(TG)"; break;
        case STRIP_TYPE_GAV: strip_type_str = "Ѫ��(GAV)"; break;
    }
    printf("�A�O�yԇ�Ŀ: %s\r\n", strip_type_str);
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      printHardwareCalibInfo
 *
 * @brief   ��ӡӲ�wУ�ʅ�����Ϣ
 *
 * @param   calibParams - Ӳ�wУ�ʅ���ָ�
 *
 * @return  none
 */
void printHardwareCalibInfo(const HardwareCalibBlock *calibParams)
{
    printf("\r\n====== Ӳ�wУ�ʅ�����Ϣ ======\r\n");
    printf("EV_T3�|�l늉�: %d mV\r\n", calibParams->evT3Trigger);
    printf("�y������늘O늉�: %d\r\n", calibParams->evWorking);
    printf("ѪҺ�z�y늘O늉�: %d\r\n", calibParams->evT3);
    printf("OPAУ��б��: %.4f\r\n", calibParams->ops);
    printf("OPAУ�ʽؾ�: %.4f\r\n", calibParams->opi);
    printf("�ض�У��ƫ��: %.1f��C\r\n", (float)calibParams->tempOffset / 10.0f);
    printf("늳�У��ƫ��: %d mV\r\n", calibParams->batteryOffset * 10);
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      printBloodGlucoseInfo
 *
 * @brief   ��ӡѪ�ǅ�����Ϣ
 *
 * @param   bgParams - Ѫ�ǅ���ָ�
 *
 * @return  none
 */
void printBloodGlucoseInfo(const BloodGlucoseBlock *bgParams)
{
    printf("\r\n====== Ѫ�ǅ�����Ϣ ======\r\n");
    printf("��̖: %s\r\n", bgParams->bgStripLot);
    printf("��ԇƬ�z�yˮƽ: %d\r\n", bgParams->bgNdl);
    printf("��ʹ��ԇƬ�z�yˮƽ: %d\r\n", bgParams->bgUdl);
    printf("ѪҺ�z�yˮƽ: %d\r\n", bgParams->bgBloodIn);
    printf("�y������: %d - %d mg/dL\r\n", bgParams->bgL * 10, bgParams->bgH * 10);
    
    printf("\r\n-- �yԇ�r�򅢔� --\r\n");
    printf("��һ�M: TPL=%d, TRD=%d, EVWidth=%d\r\n", 
           bgParams->bgTPL1, bgParams->bgTRD1, bgParams->bgEVWidth1);
    printf("�ڶ��M: TPL=%d, TRD=%d, EVWidth=%d\r\n", 
           bgParams->bgTPL2, bgParams->bgTRD2, bgParams->bgEVWidth2);
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      simulateTest
 *
 * @brief   ģ�Mһ�Μyԇ�K����ӛ�
 *
 * @param   none
 *
 * @return  none
 */
void simulateTest(void)
{
    static uint16_t testValue = 120; // ��ʼ�yԇֵ(mg/dL)
    
    // �@ȡ��ǰ�r�g
    uint8_t year, month, date, hour, minute, second;
    PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second);
    
    // �@ȡ��ǰ�yԇ�Ŀ
    BasicSystemBlock basicParams;
    PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
    
    // �ʂ�yԇӛ�
    g_testRecord.resultStatus = 0; // �ɹ�
    g_testRecord.resultValue = testValue;
    g_testRecord.testType = basicParams.stripType;
    g_testRecord.eventType = basicParams.defaultEvent;
    g_testRecord.stripCode = 1; // ���O��ԇƬУ���a
    g_testRecord.year = 2000 + year;
    g_testRecord.month = month;
    g_testRecord.date = date;
    g_testRecord.hour = hour;
    g_testRecord.minute = minute;
    g_testRecord.second = second;
    g_testRecord.batteryVoltage = 320; // ģ�Mֵ: 3.2V
    g_testRecord.temperature = 250; // ģ�Mֵ: 25.0��C
    
    // ����yԇӛ�
    if (PARAM_SaveTestRecord(&g_testRecord)) {
        printf("\r\n�ѱ���yԇӛ�: ���=%d, ֵ=%d mg/dL, �r�g=20%02d/%02d/%02d %02d:%02d:%02d\r\n", 
               g_testRecord.testType, g_testRecord.resultValue,
               year, month, date, hour, minute, second);
    } else {
        printf("\r\n����yԇӛ�ʧ��!\r\n");
    }
    
    // ���Ӝyԇ�Δ�
    PARAM_IncreaseTestCount();
    
    // ��׃�´Μyԇֵ
    testValue += 5;
    if (testValue > 350) testValue = 80;
}

/*********************************************************************
 * @fn      printTestRecords
 *
 * @brief   ��ӡ�yԇӛ�
 *
 * @param   none
 *
 * @return  none
 */
void printTestRecords(void)
{
    uint16_t count = PARAM_GetTestRecordCount();
    
    printf("\r\n====== �yԇӛ� (��%d�P) ======\r\n", count);
    
    if (count > 0) {
        // ����@ʾ��10�Pӛ�
        uint16_t maxShow = (count > 10) ? 10 : count;
        uint16_t startIdx = (count > 10) ? (count - 10) : 0;
        TestRecord records[10];
        uint16_t actualCount = 0;
        
        if (PARAM_ReadTestRecords(records, startIdx, maxShow, &actualCount)) {
            for (uint16_t i = 0; i < actualCount; i++) {
                TestRecord *rec = &records[i];
                
                char *typeStr = "δ֪";
                switch (rec->testType) {
                    case STRIP_TYPE_GLV: typeStr = "Ѫ��(GLV)"; break;
                    case STRIP_TYPE_UA: typeStr = "����(U)"; break;
                    case STRIP_TYPE_CHOL: typeStr = "��đ�̴�(C)"; break;
                    case STRIP_TYPE_TG: typeStr = "�������֬(TG)"; break;
                    case STRIP_TYPE_GAV: typeStr = "Ѫ��(GAV)"; break;
                }
                
                char *eventStr = "δ֪";
                switch (rec->eventType) {
                    case EVENT_QC: eventStr = "QC"; break;
                    case EVENT_AC: eventStr = "AC"; break;
                    case EVENT_PC: eventStr = "PC"; break;
                }
                
                printf("%d. [%s] %d mg/dL (%s) %04d/%02d/%02d %02d:%02d:%02d\r\n",
                       startIdx + i + 1, typeStr, rec->resultValue, eventStr,
                       rec->year, rec->month, rec->date, rec->hour, rec->minute, rec->second);
            }
        } else {
            printf("�xȡ�yԇӛ�ʧ��!\r\n");
        }
    } else {
        printf("�]�Мyԇӛ�\r\n");
    }
    
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("ϵ�y�r��l��: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("��ƬID: %08x\r\n", DBGMCU_GetCHIPID());
    printf("�๦�������yԇ�xFlash�������淽����ʽ\r\n");
    
    /* ��ʼ����������ϵ�y */
    PARAM_Init();
    printf("��������ϵ�y��ʼ�����\r\n");
    
    /* �xȡ�������^�K */
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &g_basicParams, sizeof(BasicSystemBlock))) {
        printBasicSystemInfo(&g_basicParams);
    } else {
        printf("�xȡ����ϵ�y����ʧ��!\r\n");
    }
    
    if (PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &g_calibParams, sizeof(HardwareCalibBlock))) {
        printHardwareCalibInfo(&g_calibParams);
    } else {
        printf("�xȡӲ�wУ�ʅ���ʧ��!\r\n");
    }
    
    if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &g_bgParams, sizeof(BloodGlucoseBlock))) {
        printBloodGlucoseInfo(&g_bgParams);
    } else {
        printf("�xȡѪ�ǅ���ʧ��!\r\n");
    }
    
    /* ģ�M�yԇ���� */
    for (int i = 0; i < 3; i++) {
        simulateTest();
        Delay_Ms(1000);
    }
    
    /* ��ӡ�yԇӛ� */
    printTestRecords();
    
    /* ��ѭ�h */
    while(1)
    {
        Delay_Ms(1000);
    }
}

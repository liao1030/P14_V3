/********************************** (C) COPYRIGHT *******************************
 * File Name          : Calculation.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Blood glucose calculation functions.
 *******************************************************************************/

#include "Calculation.h"
#include "param_table.h"
#include <stdio.h>
#include "strip_detect.h"  // ����ԇƬ�z�y���P����

/* Ѫ��ֵ׃�� */
uint16_t wGlucose = 0;

/*********************************************************************
 * @fn      CalGlucose
 *
 * @brief   ��ADCֵ�D�Q��Ѫ��ֵ
 *
 * @param   adcValue - ݔ���ADCֵ
 *
 * @return  none (�Y��������ȫ��׃��wGlucose��)
 */
void CalGlucose(uint16_t adcValue)
{
    // Ѫ��Ӌ�ㅢ��
    float Sr = 1.039081335f;
    float Ir = 16.76f;
    float S3 = 165.0f;
    float I2 = 0.0f;
    float S2 = 16.0f;
    float I3 = -60.0f;
    float Tf = 100.0f;
    float To = 0.0f;
    float BG_Offset = 0.0f;    
    
    // ����ԇƬ��͛Q������ֵ
    uint16_t EV_BACKGROUND;
    
    // ����ʹ��ԇƬ�z�yģ�M�Į�ǰ��ͣ�����oЧ�tʹ�Å������е�ֵ
    StripType_TypeDef stripType = STRIP_DETECT_GetStripType();
    if (stripType >= STRIP_TYPE_MAX) {
        stripType = (StripType_TypeDef)PARAM_GetByte(PARAM_STRIP_TYPE);
    }
    
    printf("Using strip type: %s (%d)\r\n", StripType_GetName(stripType), stripType);
    
    switch(stripType) {
        case STRIP_TYPE_GLV:  // ԇƬGLV
            EV_BACKGROUND = 625;
            break;
        case STRIP_TYPE_U:    // ԇƬU
            EV_BACKGROUND = 466;
            break;
        case STRIP_TYPE_C:    // ԇƬC
            EV_BACKGROUND = 156;
            break;
        case STRIP_TYPE_TG:   // ԇƬTG
            EV_BACKGROUND = 156;
            break;
        case STRIP_TYPE_GAV:  // ԇƬGAV
            EV_BACKGROUND = 625;
            break;
        default:
            EV_BACKGROUND = 625; // �A�Oֵ
            break;
    }
    
    float fTmp;
    
    // Ѫ��Ӌ�㹫ʽ
    // Step 1: fTmp = adc_value*Sr+Ir
    fTmp = (float)adcValue * Sr + Ir;
    
    // Step 2: fTmp = fTmp-EV_BACKGROUND (����ԇƬ��̈́ӑB�Q��)
    fTmp = fTmp - (float)EV_BACKGROUND;
    
    // Step 3: fTmp = fTmp*4/S2-I2
    fTmp = fTmp * 4.0f / S2 - I2;
    
    // Step 4: fTmp = fTmp*Tf/100
    fTmp = fTmp * Tf / 100.0f;
    
    // Step 5: fTmp = fTmp*S3/100+I3+To
    fTmp = fTmp * S3 / 100.0f + I3 + To;
    
    // Step 6: fTmp=fTmp+BG_Offset
    fTmp = fTmp + BG_Offset;
    
    // Step 7: glucose=fTmp
    float glucose = fTmp;
    
    // Step 8: ����Ѫ��ֵ���� 0<=glucose<=999
    if (glucose < 0.0f) glucose = 0.0f;
    if (glucose > 999.0f) glucose = 999.0f;
    
    // ���Y�����浽ȫ��׃�� (�Ē����뵽����)
    wGlucose = (uint16_t)(glucose + 0.5f);
    
    printf("Blood Glucose Calculation:\r\n");
    printf("  ADC Value: %d\r\n", adcValue);
    printf("  Strip Type: %d, EV_BACKGROUND: %d\r\n", stripType, EV_BACKGROUND);
    printf("  Calculated fTmp steps:\r\n");
    printf("    Step 1: %.3f\r\n", (float)adcValue * Sr + Ir);
    printf("    Step 2: %.3f\r\n", ((float)adcValue * Sr + Ir) - (float)EV_BACKGROUND);
    printf("    Final glucose: %d\r\n", wGlucose);
} 
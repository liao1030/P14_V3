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
#include "strip_detect.h"  // 包含試片檢測相關函數

/* 血糖值變數 */
uint16_t wGlucose = 0;

/*********************************************************************
 * @fn      CalGlucose
 *
 * @brief   將ADC值轉換為血糖值
 *
 * @param   adcValue - 輸入的ADC值
 *
 * @return  none (結果儲存在全域變數wGlucose中)
 */
void CalGlucose(uint16_t adcValue)
{
    // 血糖計算參數
    float Sr = 1.039081335f;
    float Ir = 16.76f;
    float S3 = 165.0f;
    float I2 = 0.0f;
    float S2 = 16.0f;
    float I3 = -60.0f;
    float Tf = 100.0f;
    float To = 0.0f;
    float BG_Offset = 0.0f;    
    
    // 根據試片類型決定背景值
    uint16_t EV_BACKGROUND;
    
    // 優先使用試片檢測模組的當前類型，如果無效則使用參數表中的值
    StripType_TypeDef stripType = STRIP_DETECT_GetStripType();
    if (stripType >= STRIP_TYPE_MAX) {
        stripType = (StripType_TypeDef)PARAM_GetByte(PARAM_STRIP_TYPE);
    }
    
    printf("Using strip type: %s (%d)\r\n", StripType_GetName(stripType), stripType);
    
    switch(stripType) {
        case STRIP_TYPE_GLV:  // 試片GLV
            EV_BACKGROUND = 625;
            break;
        case STRIP_TYPE_U:    // 試片U
            EV_BACKGROUND = 466;
            break;
        case STRIP_TYPE_C:    // 試片C
            EV_BACKGROUND = 156;
            break;
        case STRIP_TYPE_TG:   // 試片TG
            EV_BACKGROUND = 156;
            break;
        case STRIP_TYPE_GAV:  // 試片GAV
            EV_BACKGROUND = 625;
            break;
        default:
            EV_BACKGROUND = 625; // 預設值
            break;
    }
    
    float fTmp;
    
    // 血糖計算公式
    // Step 1: fTmp = adc_value*Sr+Ir
    fTmp = (float)adcValue * Sr + Ir;
    
    // Step 2: fTmp = fTmp-EV_BACKGROUND (根據試片類型動態決定)
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
    
    // Step 8: 限制血糖值範圍 0<=glucose<=999
    if (glucose < 0.0f) glucose = 0.0f;
    if (glucose > 999.0f) glucose = 999.0f;
    
    // 將結果儲存到全域變數 (四捨五入到整數)
    wGlucose = (uint16_t)(glucose + 0.5f);
    
    printf("Blood Glucose Calculation:\r\n");
    printf("  ADC Value: %d\r\n", adcValue);
    printf("  Strip Type: %d, EV_BACKGROUND: %d\r\n", stripType, EV_BACKGROUND);
    printf("  Calculated fTmp steps:\r\n");
    printf("    Step 1: %.3f\r\n", (float)adcValue * Sr + Ir);
    printf("    Step 2: %.3f\r\n", ((float)adcValue * Sr + Ir) - (float)EV_BACKGROUND);
    printf("    Final glucose: %d\r\n", wGlucose);
} 
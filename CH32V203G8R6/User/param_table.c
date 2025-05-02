/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/09/18
 * Description        : 多功能生化測試儀參數代碼表實現文件
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_table.h"
#include "string.h"
#include "ch32v20x_flash.h"

/* 定義參數表在Flash中的存儲位置 */
#define PARAM_TABLE_FLASH_ADDR   0x08010000  // 假設在Flash的64KB處保存參數表

/*********************************************************************
 * @fn      ParamTable_Init
 *
 * @brief   初始化參數表，設定各參數的預設值
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  none
 */
void ParamTable_Init(ParamTable_TypeDef *param_table)
{
    /* 清除所有參數，設為0 */
    memset(param_table, 0, sizeof(ParamTable_TypeDef));

    /* 1. 初始化系統基本參數 */
    param_table->LBT = 58;                // 2.58V (電池圖示閃爍)
    param_table->OBT = 55;                // 2.55V (電池圖示常亮)
    param_table->FACTORY = MODE_USER;     // 使用者模式
    param_table->MODEL_NO = 1;            // 產品型號 (根據實際設置)
    param_table->FW_NO = 10;              // 韌體版本號 V1.0
    param_table->NOT = 0;                 // 測試次數從0開始
    param_table->Code_Table_V = 1;        // 代碼表版本編號

    /* 2. 初始化時間設定參數 (設置為當前時間的預設值) */
    param_table->YEAR = 24;       // 2024年
    param_table->MONTH = 9;       // 9月
    param_table->DATE = 18;       // 18日
    param_table->HOUR = 12;       // 12時
    param_table->MINUTE = 0;      // 0分
    param_table->SECOND = 0;      // 0秒

    /* 3. 初始化測試環境參數 */
    param_table->TLL = 10;               // 操作溫度範圍下限 10.0°C
    param_table->TLH = 40;               // 操作溫度範圍上限 40.0°C
    param_table->MGDL = UNIT_MG_DL;      // 預設單位 mg/dL
    param_table->EVENT = EVENT_AC;       // 預設事件類型 餐前測試
    param_table->Strip_Type = STRIP_TYPE_GLV; // 預設測試項目 血糖

    /* 4. 初始化硬體校準參數 */
    param_table->EV_T3_TRG = 180;         // EV_T3觸發電壓 1800mV
    param_table->EV_WORKING = 164;        // 測量工作電極電壓
    param_table->EV_T3 = 164;             // 血液檢測電極電壓
    param_table->DACO = 0;                // DAC偏移補償
    param_table->DACDO = 0;               // DAC校正管理參數
    param_table->CC211NoDone = 0;         // CC211未完成旗標
    param_table->CAL_TOL = 8;             // OPS/OPI校準容差 0.8%
    param_table->OPS = 1.0f;              // OPA校準斜率
    param_table->OPI = 0.0f;              // OPA校準截距
    param_table->QCT = 0;                 // QCT校準測試低位元組
    param_table->TOFFSET = 0;             // 溫度校準偏移
    param_table->BOFFSET = 0;             // 電池校準偏移

    /* 初始化各項測試參數的默認值 */
    /* 5. 血糖專用參數 */
    param_table->BG_CSU_TOL = 10;         // 試片檢查容差
    param_table->BG_NDL = 100;            // 新試片檢測水平
    param_table->BG_UDL = 90;             // 已使用試片檢測水平
    param_table->BG_BLOOD_IN = 200;       // 血液檢測水平
    /* 設置默認批號 */
    strcpy((char *)param_table->BG_Strip_Lot, "GLV20240918");
    
    param_table->BG_L = 20;               // 血糖值下限 20 mg/dL
    param_table->BG_H = 600;              // 血糖值上限 600 mg/dL
    param_table->BG_T3_E37 = 2000;        // T3 ADV錯誤37閾值

    /* 設置測試時序參數 */
    param_table->BG_TPL_1 = 10;           // 1秒
    param_table->BG_TRD_1 = 50;           // 5秒
    param_table->BG_EV_Width_1 = 2;       // 0.2秒
    param_table->BG_TPL_2 = 10;           // 1秒
    param_table->BG_TRD_2 = 30;           // 3秒
    param_table->BG_EV_Width_2 = 2;       // 0.2秒

    /* 設置品管液參數 */
    param_table->BG_Sq = 100;
    param_table->BG_Iq = 0;
    param_table->BG_CVq = 100;            // 關閉QC優化
    param_table->BG_Aq = 5;
    param_table->BG_Bq = 5;
    param_table->BG_Aq5 = 5;
    param_table->BG_Bq5 = 5;
    
    /* 設置計算式補償參數 */
    param_table->BG_S2 = 100;
    param_table->BG_I2 = 0;
    param_table->BG_Sr = 1.0f;
    param_table->BG_Ir = 0.0f;

    /* 6. 尿酸專用參數 */
    param_table->U_CSU_TOL = 10;
    param_table->U_NDL = 100;
    param_table->U_UDL = 90;
    param_table->U_BLOOD_IN = 200;
    strcpy((char *)param_table->U_Strip_Lot, "UA20240918");
    
    param_table->U_L = 3;
    param_table->U_H = 20;
    param_table->U_T3_E37 = 2000;

    param_table->U_TPL_1 = 10;
    param_table->U_TRD_1 = 50;
    param_table->U_EV_Width_1 = 2;
    param_table->U_TPL_2 = 10;
    param_table->U_TRD_2 = 30;
    param_table->U_EV_Width_2 = 2;

    param_table->U_Sq = 100;
    param_table->U_Iq = 0;
    param_table->U_CVq = 100;
    param_table->U_Aq = 5;
    param_table->U_Bq = 5;
    
    param_table->U_S2 = 100;
    param_table->U_I2 = 0;
    param_table->U_Sr = 1.0f;
    param_table->U_Ir = 0.0f;

    /* 7. 總膽固醇專用參數 */
    param_table->C_CSU_TOL = 10;
    param_table->C_NDL = 100;
    param_table->C_UDL = 90;
    param_table->C_BLOOD_IN = 200;
    strcpy((char *)param_table->C_Strip_Lot, "CHOL20240918");
    
    param_table->C_L = 100;
    param_table->C_H = 400;
    param_table->C_T3_E37 = 2000;

    param_table->C_TPL_1 = 10;
    param_table->C_TRD_1 = 50;
    param_table->C_EV_Width_1 = 2;
    param_table->C_TPL_2 = 10;
    param_table->C_TRD_2 = 30;
    param_table->C_EV_Width_2 = 2;

    param_table->C_Sq = 100;
    param_table->C_Iq = 0;
    param_table->C_CVq = 100;
    param_table->C_Aq = 5;
    param_table->C_Bq = 5;
    
    param_table->C_S2 = 100;
    param_table->C_I2 = 0;
    param_table->C_Sr = 1.0f;
    param_table->C_Ir = 0.0f;

    /* 8. 三酸甘油脂專用參數 */
    param_table->TG_CSU_TOL = 10;
    param_table->TG_NDL = 100;
    param_table->TG_UDL = 90;
    param_table->TG_BLOOD_IN = 200;
    strcpy((char *)param_table->TG_Strip_Lot, "TG20240918");
    
    param_table->TG_L = 50;
    param_table->TG_H = 500;
    param_table->TG_T3_E37 = 2000;

    param_table->TG_TPL_1 = 10;
    param_table->TG_TRD_1 = 50;
    param_table->TG_EV_Width_1 = 2;
    param_table->TG_TPL_2 = 10;
    param_table->TG_TRD_2 = 30;
    param_table->TG_EV_Width_2 = 2;

    param_table->TG_Sq = 100;
    param_table->TG_Iq = 0;
    param_table->TG_CVq = 100;
    param_table->TG_Aq = 5;
    param_table->TG_Bq = 5;
    
    param_table->TG_S2 = 100;
    param_table->TG_I2 = 0;
    param_table->TG_Sr = 1.0f;
    param_table->TG_Ir = 0.0f;

    /* 計算校驗和與CRC */
    uint16_t checksum = ParamTable_CalculateChecksum(param_table);
    param_table->SUM_L = (uint8_t)(checksum & 0xFF);
    param_table->SUM_H = (uint8_t)((checksum >> 8) & 0xFF);
    param_table->CRC16 = ParamTable_CalculateCRC16(param_table);
}

/*********************************************************************
 * @fn      ParamTable_CalculateChecksum
 *
 * @brief   計算參數表校驗和(地址0~674)
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  校驗和值
 */
uint16_t ParamTable_CalculateChecksum(ParamTable_TypeDef *param_table)
{
    uint16_t checksum = 0;
    uint8_t *ptr = (uint8_t *)param_table;
    
    /* 計算地址0~674的校驗和 */
    for(uint16_t i = 0; i < PARAM_CHECKSUM_OFFSET; i++)
    {
        checksum += ptr[i];
    }
    
    return checksum;
}

/*********************************************************************
 * @fn      ParamTable_CalculateCRC16
 *
 * @brief   計算參數表CRC16(完整表)
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  CRC16校驗值
 */
uint8_t ParamTable_CalculateCRC16(ParamTable_TypeDef *param_table)
{
    /* 簡單實現，實際上應使用標準CRC16算法 */
    uint8_t crc = 0;
    uint8_t *ptr = (uint8_t *)param_table;
    
    /* 計算完整表的XOR作為CRC (簡化版) */
    for(uint16_t i = 0; i < PARAM_TABLE_SIZE - 1; i++)
    {
        crc ^= ptr[i];
    }
    
    return crc;
}

/*********************************************************************
 * @fn      ParamTable_Verify
 *
 * @brief   驗證參數表的校驗和和CRC是否正確
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  0: 校驗成功, 1: 校驗失敗
 */
uint8_t ParamTable_Verify(ParamTable_TypeDef *param_table)
{
    uint16_t calculated_checksum = ParamTable_CalculateChecksum(param_table);
    uint16_t stored_checksum = ((uint16_t)param_table->SUM_H << 8) | param_table->SUM_L;
    
    /* 檢查校驗和 */
    if(calculated_checksum != stored_checksum)
    {
        return 1; // 校驗失敗
    }
    
    /* 檢查CRC16 */
    uint8_t calculated_crc = ParamTable_CalculateCRC16(param_table);
    if(calculated_crc != param_table->CRC16)
    {
        return 1; // 校驗失敗
    }
    
    return 0; // 校驗成功
}

/*********************************************************************
 * @fn      ParamTable_SaveToFlash
 *
 * @brief   將參數表保存到Flash存儲器
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  none
 */
void ParamTable_SaveToFlash(ParamTable_TypeDef *param_table)
{
    /* 重新計算校驗和與CRC */
    uint16_t checksum = ParamTable_CalculateChecksum(param_table);
    param_table->SUM_L = (uint8_t)(checksum & 0xFF);
    param_table->SUM_H = (uint8_t)((checksum >> 8) & 0xFF);
    param_table->CRC16 = ParamTable_CalculateCRC16(param_table);
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 擦除目標Flash扇區 */
    FLASH_ErasePage(PARAM_TABLE_FLASH_ADDR);
    
    /* 將參數表寫入Flash */
    uint32_t *source = (uint32_t *)param_table;
    uint32_t address = PARAM_TABLE_FLASH_ADDR;
    
    for(uint16_t i = 0; i < (sizeof(ParamTable_TypeDef) + 3) / 4; i++)
    {
        FLASH_ProgramWord(address, source[i]);
        address += 4;
    }
    
    /* 鎖定Flash */
    FLASH_Lock();
}

/*********************************************************************
 * @fn      ParamTable_LoadFromFlash
 *
 * @brief   從Flash存儲器加載參數表
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  0: 加載成功, 1: 校驗失敗，已初始化為默認值
 */
uint8_t ParamTable_LoadFromFlash(ParamTable_TypeDef *param_table)
{
    /* 從Flash讀取參數表 */
    memcpy(param_table, (void *)PARAM_TABLE_FLASH_ADDR, sizeof(ParamTable_TypeDef));
    
    /* 驗證參數表 */
    if(ParamTable_Verify(param_table) != 0)
    {
        /* 校驗失敗，初始化為默認值 */
        ParamTable_Init(param_table);
        return 1;
    }
    
    return 0;
} 
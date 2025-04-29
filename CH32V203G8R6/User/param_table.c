/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/04/29
 * Description        : 多功能生化測試儀參數代碼表實現文件
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#include "param_table.h"
#include "string.h"
#include "debug.h"
#include "ch32v20x_flash.h"

/* Flash參數表起始地址 */
#define PARAM_TABLE_FLASH_ADDR  0x0800F000  // 使用Flash最後一個頁面存儲參數表

/* 定義全局參數表變量 */
Param_Table_TypeDef g_ParamTable;

/* 參數表默認值設定 */
static const Param_Table_TypeDef DefaultParamTable = {
    /* 1. 系統基本參數 */
    .LBT = 58,                    // 低電池閾值: 2.58V
    .OBT = 55,                    // 電池耗竭閾值: 2.55V
    .FACTORY = 0,                 // 儀器操作模式: 使用者模式
    .MODEL_NO = 10,               // 產品型號: 10
    .FW_NO = 1,                   // 韌體版本號: 1
    .NOT = 0,                     // 測試次數: 0
    .Code_Table_V = 1,            // 代碼表版本號: 1
    
    /* 2. 時間設定參數 */
    .YEAR = 25,                   // 年份設定: 2025年
    .MONTH = 4,                   // 月份設定: 4月
    .DATE = 29,                   // 日期設定: 29日
    .HOUR = 12,                   // 小時設定: 12時
    .MINUTE = 0,                  // 分鐘設定: 0分
    .SECOND = 0,                  // 秒數設定: 0秒
    
    /* 3. 測試環境參數 */
    .TLL = 1,                     // 操作溫度範圍下限: 1.0°C
    .TLH = 40,                    // 操作溫度範圍上限: 40.0°C
    .MGDL = UNIT_MMOL_L,          // 濃度單位設定: mmol/L
    .EVENT = EVENT_AC,            // 設置事件類型: 空腹
    .Strip_Type = STRIP_TYPE_GLV, // 測試項目: 血糖(GLV)
    
    /* 4. 硬體校正參數 */
    .EV_T3_TRG = 120,             // EV_T3觸發電壓: 1200mV
    .EV_WORKING = 164,            // 測量工作電壓電壓
    .EV_T3 = 164,                 // 血液滴入電極電壓
    .DACO = 0,                    // DAC偏移補償
    .DACDO = 0,                   // DAC校正管理參數
    .CC211NoDone = 0,             // CC211未完成旗標 (正常)
    .CAL_TOL = 8,                 // OPS/OPI校正容差 (0.8%)
    .OPS = 1.0f,                  // OPA校正斜率
    .OPI = 0.0f,                  // OPA校正截距
    .QCT = 0,                     // QCT校正測試低位元組
    .TOFFSET = 0,                 // 溫度校正偏移
    .BOFFSET = 0,                 // 電池校正偏移
    
    /* 5. 血糖(GLV/GAV)專用參數 */
    /* 5.1 試片參數 */
    .BG_CSU_TOL = 10,             // 試片檢查容差 (3%)
    .BG_NDL = 1000,               // 新試片水平
    .BG_UDL = 800,                // 已使用試片水平
    .BG_BLOOD_IN = 1500,          // 血液水平
    .BG_Strip_Lot = "BG2025042900001",  // 試片批號
    
    /* 5.2 測量範圍參數 */
    .BG_L = 10,                   // 血糖值下限 (1.0 mmol/L或10 mg/dl)
    .BG_H = 250,                  // 血糖值上限 (修正為250，原為333超出uint8_t範圍)
    .BG_T3_E37 = 2000,            // T3 ADV閾E37閾值
    
    /* 5.3 測試時序參數 (時間單位: 100ms) */
    .BG_TPL_1 = 10,               // 時間脈衝寬度 (第一組): 1000ms
    .BG_TRD_1 = 20,               // 原始數據時間 (第一組): 2000ms
    .BG_EV_Width_1 = 5,           // 脈衝時間 (第一組): 500ms
    .BG_TPL_2 = 10,               // 時間脈衝寬度 (第二組): 1000ms
    .BG_TRD_2 = 35,               // 原始數據時間 (第二組): 3500ms
    .BG_EV_Width_2 = 10,          // 脈衝時間 (第二組): 1000ms
    
    /* 預設其他參數值... */
};

/**
 * @brief  初始化參數表
 * @note   根據Flash中是否存在有效參數表決定初始化方式
 * @param  None
 * @retval None
 */
void PARAM_TABLE_Init(void)
{
    if (PARAM_TABLE_Verify() == 0) {
        // Flash中沒有有效參數表，設置默認值
        PARAM_TABLE_SetDefaults();
        PARAM_TABLE_SaveToFlash();
        printf("初始化參數表: 使用默認值\r\n");
    } else {
        // 從Flash中載入參數表
        PARAM_TABLE_LoadFromFlash();
        printf("初始化參數表: 從Flash中載入\r\n");
    }
}

/**
 * @brief  設置參數表為默認值
 * @param  None
 * @retval None
 */
void PARAM_TABLE_SetDefaults(void)
{
    // 複製默認參數表到全局變數
    memcpy(&g_ParamTable, &DefaultParamTable, sizeof(Param_Table_TypeDef));
    
    // 計算校驗和
    uint16_t checksum = PARAM_TABLE_CalculateChecksum();
    g_ParamTable.SUM_L = checksum & 0xFF;
    g_ParamTable.SUM_H = (checksum >> 8) & 0xFF;
    g_ParamTable.CRC16 = 1;  // 簡單CRC，實際應計算CRC16
}

/**
 * @brief  將參數表保存到Flash
 * @param  None
 * @retval None
 */
void PARAM_TABLE_SaveToFlash(void)
{
    uint32_t flashAddr = PARAM_TABLE_FLASH_ADDR;
    uint32_t *src = (uint32_t *)&g_ParamTable;
    uint16_t len = (sizeof(Param_Table_TypeDef) + 3) / 4;  // 以4字節為單位
    uint32_t i;
    
    FLASH_Unlock();
    
    // 修正：使用FLASH_ErasePage代替不存在的FLASH_ErasePages
    FLASH_ErasePage(PARAM_TABLE_FLASH_ADDR);  // 擦除一頁Flash
    
    // 按4字節為單位寫入Flash
    for (i = 0; i < len; i++) {
        FLASH_ProgramWord(flashAddr, *src);
        flashAddr += 4;
        src++;
    }
    
    FLASH_Lock();
    
    printf("參數表已保存到Flash\r\n");
}

/**
 * @brief  從Flash載入參數表
 * @param  None
 * @retval None
 */
void PARAM_TABLE_LoadFromFlash(void)
{
    memcpy(&g_ParamTable, (void *)PARAM_TABLE_FLASH_ADDR, sizeof(Param_Table_TypeDef));
    printf("參數表已從Flash載入\r\n");
}

/**
 * @brief  計算參數表校驗和 (0-674位置)
 * @param  None
 * @retval 校驗和
 */
uint16_t PARAM_TABLE_CalculateChecksum(void)
{
    uint8_t *tableBytes = (uint8_t *)&g_ParamTable;
    uint16_t sum = 0;
    uint16_t i;
    
    for (i = 0; i < PARAM_TABLE_CHECK_SIZE; i++) {
        sum += tableBytes[i];
    }
    
    return sum;
}

/**
 * @brief  驗證參數表是否有效
 * @param  None
 * @retval 1: 有效; 0: 無效
 */
uint8_t PARAM_TABLE_Verify(void)
{
    Param_Table_TypeDef *flashParamTable = (Param_Table_TypeDef *)PARAM_TABLE_FLASH_ADDR;
    uint8_t *tableBytes = (uint8_t *)flashParamTable;
    uint16_t sum = 0;
    uint16_t i;
    uint16_t storedChecksum;
    
    // 檢查Flash中是否存在參數表 (簡單檢查某些關鍵值)
    if (flashParamTable->MODEL_NO == 0xFF && flashParamTable->FW_NO == 0xFF) {
        return 0;  // Flash未初始化
    }
    
    // 計算校驗和
    for (i = 0; i < PARAM_TABLE_CHECK_SIZE; i++) {
        sum += tableBytes[i];
    }
    
    // 檢查校驗和
    storedChecksum = (flashParamTable->SUM_H << 8) | flashParamTable->SUM_L;
    
    return (sum == storedChecksum) ? 1 : 0;
}

/**
 * @brief  打印參數表信息
 * @param  None
 * @retval None
 */
void PARAM_TABLE_PrintInfo(void)
{
    printf("\r\n==== 多功能生化測試儀參數表信息 ====\r\n");
    printf("產品型號: %d\r\n", g_ParamTable.MODEL_NO);
    printf("韌體版本: %d\r\n", g_ParamTable.FW_NO);
    printf("參數表版本: %d\r\n", g_ParamTable.Code_Table_V);
    printf("測試次數: %d\r\n", g_ParamTable.NOT);
    
    printf("\r\n-- 系統設定 --\r\n");
    printf("操作模式: %s\r\n", (g_ParamTable.FACTORY == FACTORY_FACTORY_MODE) ? "工廠模式" : "使用者模式");
    printf("低電池閾值: %d (約 %d.%02d V)\r\n", g_ParamTable.LBT, g_ParamTable.LBT/100, g_ParamTable.LBT%100);
    printf("電池耗竭閾值: %d (約 %d.%02d V)\r\n", g_ParamTable.OBT, g_ParamTable.OBT/100, g_ParamTable.OBT%100);
    
    printf("\r\n-- 日期時間 --\r\n");
    printf("日期時間: 20%02d/%02d/%02d %02d:%02d:%02d\r\n", 
           g_ParamTable.YEAR, g_ParamTable.MONTH, g_ParamTable.DATE, 
           g_ParamTable.HOUR, g_ParamTable.MINUTE, g_ParamTable.SECOND);
    
    printf("\r\n-- 測試環境 --\r\n");
    printf("操作溫度範圍: %d.0°C ~ %d.0°C\r\n", g_ParamTable.TLL, g_ParamTable.TLH);
    printf("濃度單位: %s\r\n", 
           (g_ParamTable.MGDL == UNIT_MMOL_L) ? "mmol/L" : 
           (g_ParamTable.MGDL == UNIT_MG_DL) ? "mg/dL" : "gm/dl");
    
    const char* stripTypes[] = {"血糖(GLV)", "尿酸(U)", "總膽固醇(C)", "三酸甘油脂(TG)", "血糖(GAV)"};
    if (g_ParamTable.Strip_Type < 5) {
        printf("測試項目: %s\r\n", stripTypes[g_ParamTable.Strip_Type]);
    } else {
        printf("測試項目: 未知(%d)\r\n", g_ParamTable.Strip_Type);
    }
    
    const char* eventTypes[] = {"品管液(QC)", "空腹(AC)", "餐後(PC)"};
    if (g_ParamTable.EVENT < 3) {
        printf("事件類型: %s\r\n", eventTypes[g_ParamTable.EVENT]);
    } else {
        printf("事件類型: 未知(%d)\r\n", g_ParamTable.EVENT);
    }
    
    printf("\r\n==== 校驗信息 ====\r\n");
    uint16_t calcChecksum = PARAM_TABLE_CalculateChecksum();
    uint16_t storedChecksum = (g_ParamTable.SUM_H << 8) | g_ParamTable.SUM_L;
    printf("計算校驗和: 0x%04X\r\n", calcChecksum);
    printf("存儲校驗和: 0x%04X\r\n", storedChecksum);
    printf("校驗結果: %s\r\n", (calcChecksum == storedChecksum) ? "有效" : "無效");
    
    printf("===================================\r\n\r\n");
}
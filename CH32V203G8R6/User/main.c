/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH / HMD
 * Version            : V1.0.0
 * Date               : 2024/09/18
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Copyright (c) 2024 HMD Biomedical Inc.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *多功能生化測試儀參數代碼表程式:
 *此示例程式演示如何初始化、讀取和保存多功能生化測試儀的參數代碼表。
 *USART1_Tx(PA9) - 用於輸出調試信息
 *
 */

#include "debug.h"
#include "param_table.h"

/* 全局變數定義 */
ParamTable_TypeDef g_ParamTable;

/*********************************************************************
 * @fn      printParamTableInfo
 *
 * @brief   打印參數表基本信息
 *
 * @param   param_table - 參數表結構體指針
 *
 * @return  none
 */
void printParamTableInfo(ParamTable_TypeDef *param_table)
{
    printf("\r\n====== 多功能生化測試儀參數表信息 ======\r\n");
    printf("韌體版本: V%d.%d\r\n", param_table->FW_NO / 10, param_table->FW_NO % 10);
    printf("參數表版本: %d\r\n", param_table->Code_Table_V);
    printf("測試次數: %d\r\n", param_table->NOT);
    printf("操作模式: %s\r\n", (param_table->FACTORY == MODE_USER) ? "使用者模式" : "工廠模式");
    
    printf("\r\n-- 日期時間設定 --\r\n");
    printf("日期: 20%02d/%02d/%02d\r\n", param_table->YEAR, param_table->MONTH, param_table->DATE);
    printf("時間: %02d:%02d:%02d\r\n", param_table->HOUR, param_table->MINUTE, param_table->SECOND);
    
    printf("\r\n-- 測試環境參數 --\r\n");
    printf("操作溫度範圍: %.1f°C ~ %.1f°C\r\n", (float)param_table->TLL / 10, (float)param_table->TLH / 10);
    
    char *unit_str = "未知";
    switch(param_table->MGDL) {
        case UNIT_MMOL_L: unit_str = "mmol/L"; break;
        case UNIT_MG_DL: unit_str = "mg/dL"; break;
        case UNIT_GM_DL: unit_str = "gm/dl"; break;
    }
    printf("測量單位: %s\r\n", unit_str);
    
    char *strip_type_str = "未知";
    switch(param_table->Strip_Type) {
        case STRIP_TYPE_GLV: strip_type_str = "血糖(GLV)"; break;
        case STRIP_TYPE_UA: strip_type_str = "尿酸(U)"; break;
        case STRIP_TYPE_CHOL: strip_type_str = "總膽固醇(C)"; break;
        case STRIP_TYPE_TG: strip_type_str = "三酸甘油脂(TG)"; break;
        case STRIP_TYPE_GAV: strip_type_str = "血糖(GAV)"; break;
    }
    printf("預設測試項目: %s\r\n", strip_type_str);
    
    printf("\r\n-- 當前測試項目試片信息 --\r\n");
    switch(param_table->Strip_Type) {
        case STRIP_TYPE_GLV:
        case STRIP_TYPE_GAV:
            printf("批號: %s\r\n", param_table->BG_Strip_Lot);
            printf("測量範圍: %d - %d mg/dL\r\n", param_table->BG_L, param_table->BG_H);
            break;
        case STRIP_TYPE_UA:
            printf("批號: %s\r\n", param_table->U_Strip_Lot);
            printf("測量範圍: %d - %d mg/dL\r\n", param_table->U_L, param_table->U_H);
            break;
        case STRIP_TYPE_CHOL:
            printf("批號: %s\r\n", param_table->C_Strip_Lot);
            printf("測量範圍: %d - %d mg/dL\r\n", param_table->C_L, param_table->C_H);
            break;
        case STRIP_TYPE_TG:
            printf("批號: %s\r\n", param_table->TG_Strip_Lot);
            printf("測量範圍: %d - %d mg/dL\r\n", param_table->TG_L, param_table->TG_H);
            break;
    }
    
    printf("\r\n-- 校驗信息 --\r\n");
    uint16_t stored_checksum = ((uint16_t)param_table->SUM_H << 8) | param_table->SUM_L;
    printf("校驗和: 0x%04X\r\n", stored_checksum);
    printf("CRC16: 0x%02X\r\n", param_table->CRC16);
    
    printf("======================================\r\n");
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主程序
 *
 * @return  none
 */
int main(void)
{
    uint8_t load_result;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("系統時鐘頻率: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("晶片ID: %08x\r\n", DBGMCU_GetCHIPID());
    printf("多功能生化測試儀參數代碼表程式\r\n");
    
    /* 從Flash加載參數表，如果校驗失敗則初始化為默認值 */
    load_result = ParamTable_LoadFromFlash(&g_ParamTable);
    
    if(load_result != 0) {
        printf("參數表校驗失敗，已初始化為默認值\r\n");
        /* 初始化後保存到Flash */
        ParamTable_SaveToFlash(&g_ParamTable);
    } else {
        printf("成功從Flash加載參數表\r\n");
    }
    
    /* 更新測試次數 */
    g_ParamTable.NOT++;
    
    /* 保存更新後的參數表 */
    ParamTable_SaveToFlash(&g_ParamTable);
    
    /* 打印參數表信息 */
    printParamTableInfo(&g_ParamTable);
    
    /* 主循環 */
    while(1)
    {
    }
}

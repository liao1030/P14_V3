/********************************** (C) COPYRIGHT  *******************************
* File Name          : param_code_table.c
* Author             : WCH / HMD
* Version            : V1.0.0
* Date               : 2025/05/06
* Description        : 多功能生化測試儀參數代碼表實現
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#include "param_code_table.h"
#include <string.h>

/* 定義SUCCESS與ERROR常量 (如果沒有定義) */
#ifndef SUCCESS
#define SUCCESS     0
#endif

#ifndef ERROR
#define ERROR       1
#endif

/* 定義參數表存儲區 */
ParamCodeTable_TypeDef ParamCodeTable;

/* Flash存儲相關定義 */
#define FLASH_PAGE_SIZE         0x400   /* 1K Bytes */
#define PARAM_TABLE_FLASH_ADDR  0x0800F000 /* 設定參數表存放的Flash地址 */

/* 參數代碼表的默認初始值 */
static const uint8_t DefaultSystemParams[] = {
    /* 系統基本參數 */
    58,     /* LBT: 低電池閾值 (2.58V) */
    55,     /* OBT: 電池耗盡閾值 (2.55V) */
    0,      /* FACTORY: 使用者模式 */
    1,      /* MODEL_NO: 產品型號 */
    1,      /* FW_NO: 韌體版本號 */
    0, 0,   /* NOT: 測試次數 */
    1, 0,   /* Code_Table_V: 代碼表版本號 */
    
    /* 時間設定參數 */
    25,     /* YEAR: 2025年 */
    5,      /* MONTH: 5月 */
    6,      /* DATE: 6日 */
    12,     /* HOUR: 12點 */
    0,      /* MINUTE: 0分 */
    0,      /* SECOND: 0秒 */
    
    /* 測試顯示參數 */
    1,      /* TLL: 操作溫度範圍下限 (1°C) */
    40,     /* TLH: 操作溫度範圍上限 (40°C) */
    0,      /* MGDL: 濃度單位設定 (mmol/L) */
    0,      /* EVENT: 選項事件類型 (QC) */
    0       /* Strip_Type: 測試項目 (GLV) */
};

/* 參數代碼表初始化 */
void ParamCodeTable_Init(void)
{
    /* 嘗試從Flash中載入參數表 */
    if (ParamCodeTable_Load() != SUCCESS)
    {
        /* 如果載入失敗，則重置為默認參數 */
        ParamCodeTable_Reset();
        
        /* 保存默認參數到Flash */
        ParamCodeTable_Save();
    }
}

/* 載入參數代碼表 */
ErrorStatus ParamCodeTable_Load(void)
{
    uint8_t* flashPtr = (uint8_t*)PARAM_TABLE_FLASH_ADDR;
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* 從Flash中複製參數表到RAM */
    memcpy(tablePtr, flashPtr, sizeof(ParamCodeTable_TypeDef));
    
    /* 驗證校驗和 */
    if (!ParamCodeTable_VerifyChecksum())
    {
        return ERROR;
    }
    
    return SUCCESS;
}

/* 保存參數代碼表 */
ErrorStatus ParamCodeTable_Save(void)
{
    uint16_t checksum;
    uint32_t addr = PARAM_TABLE_FLASH_ADDR;
    uint32_t pageAddr;
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* 計算校驗和 */
    checksum = ParamCodeTable_CalculateChecksum();
    ParamCodeTable.SUM_L = (uint8_t)(checksum & 0xFF);
    ParamCodeTable.SUM_H = (uint8_t)(checksum >> 8);
    
    /* 解鎖Flash */
    FLASH_Unlock();
    
    /* 擦除存放參數表的頁 */
    pageAddr = PARAM_TABLE_FLASH_ADDR & ~(FLASH_PAGE_SIZE - 1);
    FLASH_ErasePage(pageAddr);
    
    /* 將參數表寫入Flash */
    for (int i = 0; i < sizeof(ParamCodeTable_TypeDef); i += 4)
    {
        uint32_t data = *(uint32_t*)(tablePtr + i);
        FLASH_ProgramWord(addr + i, data);
    }
    
    /* 鎖定Flash */
    FLASH_Lock();
    
    /* 驗證寫入是否正確 */
    for (int i = 0; i < sizeof(ParamCodeTable_TypeDef); i++)
    {
        if (*(uint8_t*)(addr + i) != tablePtr[i])
        {
            return ERROR;
        }
    }
    
    return SUCCESS;
}

/* 重置參數代碼表為默認值 */
ErrorStatus ParamCodeTable_Reset(void)
{
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* 清零整個參數表 */
    memset(tablePtr, 0, sizeof(ParamCodeTable_TypeDef));
    
    /* 設置系統基本參數的默認值 */
    memcpy(tablePtr, DefaultSystemParams, sizeof(DefaultSystemParams));
    
    /* 設置各項參數的默認值 */
    
    /* 血糖(GLV/GAV)測試參數 */
    ParamCodeTable.BG.CSU_TOL = 10;
    ParamCodeTable.BG.L = 20;     /* 2.0 mmol/L */
    ParamCodeTable.BG.H = 94;     /* 9.4 mmol/L (修正溢出問題) */
    
    /* 尿酸(U)測試參數 */
    ParamCodeTable.U.CSU_TOL = 10;
    ParamCodeTable.U.L = 10;      /* 100 umol/L */
    ParamCodeTable.U.H = 120;     /* 1200 umol/L */
    
    /* 總膽固醇(C)測試參數 */
    ParamCodeTable.C.CSU_TOL = 10;
    ParamCodeTable.C.L = 25;      /* 2.5 mmol/L */
    ParamCodeTable.C.H = 125;     /* 12.5 mmol/L */
    
    /* 三酸甘油脂(TG)測試參數 */
    ParamCodeTable.TG.CSU_TOL = 10;
    ParamCodeTable.TG.L = 5;      /* 0.5 mmol/L */
    ParamCodeTable.TG.H = 65;     /* 6.5 mmol/L */
    
    /* 設置硬體校正參數默認值 */
    ParamCodeTable.Hardware.EV_T3_TRG = 1600;  /* 1600 mV */
    ParamCodeTable.Hardware.EV_WORKING = 164;
    ParamCodeTable.Hardware.EV_T3 = 164;
    ParamCodeTable.Hardware.CAL_TOL = 8;       /* 0.8% */
    
    /* 設置測試時序參數 */
    /* 血糖 */
    ParamCodeTable.BG.TPL_1 = 50;      /* 5秒 */
    ParamCodeTable.BG.TRD_1 = 45;      /* 4.5秒 */
    ParamCodeTable.BG.EV_Width_1 = 10; /* 1秒 */
    ParamCodeTable.BG.TPL_2 = 95;      /* 9.5秒 */
    ParamCodeTable.BG.TRD_2 = 90;      /* 9秒 */
    ParamCodeTable.BG.EV_Width_2 = 10; /* 1秒 */
    
    /* 尿酸 */
    ParamCodeTable.U.TPL_1 = 75;       /* 7.5秒 */
    ParamCodeTable.U.TRD_1 = 70;       /* 7秒 */
    ParamCodeTable.U.EV_Width_1 = 10;  /* 1秒 */
    ParamCodeTable.U.TPL_2 = 120;      /* 12秒 */
    ParamCodeTable.U.TRD_2 = 115;      /* 11.5秒 */
    ParamCodeTable.U.EV_Width_2 = 10;  /* 1秒 */
    
    /* 總膽固醇 */
    ParamCodeTable.C.TPL_1 = 120;      /* 12秒 */
    ParamCodeTable.C.TRD_1 = 115;      /* 11.5秒 */
    ParamCodeTable.C.EV_Width_1 = 10;  /* 1秒 */
    ParamCodeTable.C.TPL_2 = 180;      /* 18秒 */
    ParamCodeTable.C.TRD_2 = 175;      /* 17.5秒 */
    ParamCodeTable.C.EV_Width_2 = 10;  /* 1秒 */
    
    /* 三酸甘油脂 */
    ParamCodeTable.TG.TPL_1 = 160;     /* 16秒 */
    ParamCodeTable.TG.TRD_1 = 155;     /* 15.5秒 */
    ParamCodeTable.TG.EV_Width_1 = 10; /* 1秒 */
    ParamCodeTable.TG.TPL_2 = 240;     /* 24秒 */
    ParamCodeTable.TG.TRD_2 = 235;     /* 23.5秒 */
    ParamCodeTable.TG.EV_Width_2 = 10; /* 1秒 */
    
    /* 計算校驗和 */
    uint16_t checksum = ParamCodeTable_CalculateChecksum();
    ParamCodeTable.SUM_L = (uint8_t)(checksum & 0xFF);
    ParamCodeTable.SUM_H = (uint8_t)(checksum >> 8);
    
    return SUCCESS;
}

/* 計算參數代碼表校驗和 */
uint16_t ParamCodeTable_CalculateChecksum(void)
{
    uint16_t sum = 0;
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* 計算地址0~674的校驗和 */
    for (int i = 0; i < 675; i++)
    {
        sum += tablePtr[i];
    }
    
    return sum;
}

/* 驗證參數代碼表校驗和 */
ErrorStatus ParamCodeTable_VerifyChecksum(void)
{
    uint16_t checksum = ParamCodeTable_CalculateChecksum();
    uint16_t storedChecksum = (ParamCodeTable.SUM_H << 8) | ParamCodeTable.SUM_L;
    
    if (checksum == storedChecksum)
    {
        return SUCCESS;
    }
    else
    {
        return ERROR;
    }
}

/* 獲取系統參數 */
uint8_t GetSystemParam(uint16_t paramAddress)
{
    if (paramAddress >= sizeof(ParamCodeTable_TypeDef))
    {
        return 0; /* 參數地址超出範圍 */
    }
    
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    return tablePtr[paramAddress];
}

/* 設置系統參數 */
ErrorStatus SetSystemParam(uint16_t paramAddress, uint8_t value)
{
    if (paramAddress >= sizeof(ParamCodeTable_TypeDef))
    {
        return ERROR; /* 參數地址超出範圍 */
    }
    
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    tablePtr[paramAddress] = value;
    
    return SUCCESS;
}

/* 獲取當前試片類型 */
uint8_t GetStripType(void)
{
    return ParamCodeTable.Display.Strip_Type;
}

/* 獲取試片類型名稱 */
const char* GetStripTypeName(uint8_t stripType)
{
    switch (stripType)
    {
        case 0: return "GLV";
        case 1: return "U";
        case 2: return "C";
        case 3: return "TG";
        case 4: return "GAV";
        default: return "Unknown";
    }
}

/* 計算CRC16 */
uint8_t CalculateCRC16(uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;
    
    while (length--)
    {
        crc ^= *data++;
        
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    
    return (uint8_t)(crc & 0xFF);
}
/********************************** (C) COPYRIGHT  *******************************
* File Name          : param_code_table.c
* Author             : WCH / HMD
* Version            : V1.0.0
* Date               : 2025/05/06
* Description        : �๦�������yԇ�x�������a�팍�F
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical Inc.
*******************************************************************************/

#include "param_code_table.h"
#include <string.h>

/* ���xSUCCESS�cERROR���� (����]�ж��x) */
#ifndef SUCCESS
#define SUCCESS     0
#endif

#ifndef ERROR
#define ERROR       1
#endif

/* ���x������惦�^ */
ParamCodeTable_TypeDef ParamCodeTable;

/* Flash�惦���P���x */
#define FLASH_PAGE_SIZE         0x400   /* 1K Bytes */
#define PARAM_TABLE_FLASH_ADDR  0x0800F000 /* �O���������ŵ�Flash��ַ */

/* �������a���Ĭ�J��ʼֵ */
static const uint8_t DefaultSystemParams[] = {
    /* ϵ�y�������� */
    58,     /* LBT: ��늳��ֵ (2.58V) */
    55,     /* OBT: 늳غıM�ֵ (2.55V) */
    0,      /* FACTORY: ʹ����ģʽ */
    1,      /* MODEL_NO: �aƷ��̖ */
    1,      /* FW_NO: �g�w�汾̖ */
    0, 0,   /* NOT: �yԇ�Δ� */
    1, 0,   /* Code_Table_V: ���a��汾̖ */
    
    /* �r�g�O������ */
    25,     /* YEAR: 2025�� */
    5,      /* MONTH: 5�� */
    6,      /* DATE: 6�� */
    12,     /* HOUR: 12�c */
    0,      /* MINUTE: 0�� */
    0,      /* SECOND: 0�� */
    
    /* �yԇ�@ʾ���� */
    1,      /* TLL: �����ضȹ������� (1��C) */
    40,     /* TLH: �����ضȹ������� (40��C) */
    0,      /* MGDL: ��Ȇ�λ�O�� (mmol/L) */
    0,      /* EVENT: �x��¼���� (QC) */
    0       /* Strip_Type: �yԇ�Ŀ (GLV) */
};

/* �������a���ʼ�� */
void ParamCodeTable_Init(void)
{
    /* �Lԇ��Flash���d�녢���� */
    if (ParamCodeTable_Load() != SUCCESS)
    {
        /* ����d��ʧ�����t���Þ�Ĭ�J���� */
        ParamCodeTable_Reset();
        
        /* ����Ĭ�J������Flash */
        ParamCodeTable_Save();
    }
}

/* �d�녢�����a�� */
ErrorStatus ParamCodeTable_Load(void)
{
    uint8_t* flashPtr = (uint8_t*)PARAM_TABLE_FLASH_ADDR;
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* ��Flash���}�u������RAM */
    memcpy(tablePtr, flashPtr, sizeof(ParamCodeTable_TypeDef));
    
    /* ��CУ�� */
    if (!ParamCodeTable_VerifyChecksum())
    {
        return ERROR;
    }
    
    return SUCCESS;
}

/* ���慢�����a�� */
ErrorStatus ParamCodeTable_Save(void)
{
    uint16_t checksum;
    uint32_t addr = PARAM_TABLE_FLASH_ADDR;
    uint32_t pageAddr;
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* Ӌ��У�� */
    checksum = ParamCodeTable_CalculateChecksum();
    ParamCodeTable.SUM_L = (uint8_t)(checksum & 0xFF);
    ParamCodeTable.SUM_H = (uint8_t)(checksum >> 8);
    
    /* ���iFlash */
    FLASH_Unlock();
    
    /* ������Ņ������� */
    pageAddr = PARAM_TABLE_FLASH_ADDR & ~(FLASH_PAGE_SIZE - 1);
    FLASH_ErasePage(pageAddr);
    
    /* �������팑��Flash */
    for (int i = 0; i < sizeof(ParamCodeTable_TypeDef); i += 4)
    {
        uint32_t data = *(uint32_t*)(tablePtr + i);
        FLASH_ProgramWord(addr + i, data);
    }
    
    /* �i��Flash */
    FLASH_Lock();
    
    /* ��C�����Ƿ����_ */
    for (int i = 0; i < sizeof(ParamCodeTable_TypeDef); i++)
    {
        if (*(uint8_t*)(addr + i) != tablePtr[i])
        {
            return ERROR;
        }
    }
    
    return SUCCESS;
}

/* ���Å������a���Ĭ�Jֵ */
ErrorStatus ParamCodeTable_Reset(void)
{
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* �������������� */
    memset(tablePtr, 0, sizeof(ParamCodeTable_TypeDef));
    
    /* �O��ϵ�y����������Ĭ�Jֵ */
    memcpy(tablePtr, DefaultSystemParams, sizeof(DefaultSystemParams));
    
    /* �O�ø�헅�����Ĭ�Jֵ */
    
    /* Ѫ��(GLV/GAV)�yԇ���� */
    ParamCodeTable.BG.CSU_TOL = 10;
    ParamCodeTable.BG.L = 20;     /* 2.0 mmol/L */
    ParamCodeTable.BG.H = 94;     /* 9.4 mmol/L (����������}) */
    
    /* ����(U)�yԇ���� */
    ParamCodeTable.U.CSU_TOL = 10;
    ParamCodeTable.U.L = 10;      /* 100 umol/L */
    ParamCodeTable.U.H = 120;     /* 1200 umol/L */
    
    /* ��đ�̴�(C)�yԇ���� */
    ParamCodeTable.C.CSU_TOL = 10;
    ParamCodeTable.C.L = 25;      /* 2.5 mmol/L */
    ParamCodeTable.C.H = 125;     /* 12.5 mmol/L */
    
    /* �������֬(TG)�yԇ���� */
    ParamCodeTable.TG.CSU_TOL = 10;
    ParamCodeTable.TG.L = 5;      /* 0.5 mmol/L */
    ParamCodeTable.TG.H = 65;     /* 6.5 mmol/L */
    
    /* �O��Ӳ�wУ������Ĭ�Jֵ */
    ParamCodeTable.Hardware.EV_T3_TRG = 1600;  /* 1600 mV */
    ParamCodeTable.Hardware.EV_WORKING = 164;
    ParamCodeTable.Hardware.EV_T3 = 164;
    ParamCodeTable.Hardware.CAL_TOL = 8;       /* 0.8% */
    
    /* �O�Üyԇ�r�򅢔� */
    /* Ѫ�� */
    ParamCodeTable.BG.TPL_1 = 50;      /* 5�� */
    ParamCodeTable.BG.TRD_1 = 45;      /* 4.5�� */
    ParamCodeTable.BG.EV_Width_1 = 10; /* 1�� */
    ParamCodeTable.BG.TPL_2 = 95;      /* 9.5�� */
    ParamCodeTable.BG.TRD_2 = 90;      /* 9�� */
    ParamCodeTable.BG.EV_Width_2 = 10; /* 1�� */
    
    /* ���� */
    ParamCodeTable.U.TPL_1 = 75;       /* 7.5�� */
    ParamCodeTable.U.TRD_1 = 70;       /* 7�� */
    ParamCodeTable.U.EV_Width_1 = 10;  /* 1�� */
    ParamCodeTable.U.TPL_2 = 120;      /* 12�� */
    ParamCodeTable.U.TRD_2 = 115;      /* 11.5�� */
    ParamCodeTable.U.EV_Width_2 = 10;  /* 1�� */
    
    /* ��đ�̴� */
    ParamCodeTable.C.TPL_1 = 120;      /* 12�� */
    ParamCodeTable.C.TRD_1 = 115;      /* 11.5�� */
    ParamCodeTable.C.EV_Width_1 = 10;  /* 1�� */
    ParamCodeTable.C.TPL_2 = 180;      /* 18�� */
    ParamCodeTable.C.TRD_2 = 175;      /* 17.5�� */
    ParamCodeTable.C.EV_Width_2 = 10;  /* 1�� */
    
    /* �������֬ */
    ParamCodeTable.TG.TPL_1 = 160;     /* 16�� */
    ParamCodeTable.TG.TRD_1 = 155;     /* 15.5�� */
    ParamCodeTable.TG.EV_Width_1 = 10; /* 1�� */
    ParamCodeTable.TG.TPL_2 = 240;     /* 24�� */
    ParamCodeTable.TG.TRD_2 = 235;     /* 23.5�� */
    ParamCodeTable.TG.EV_Width_2 = 10; /* 1�� */
    
    /* Ӌ��У�� */
    uint16_t checksum = ParamCodeTable_CalculateChecksum();
    ParamCodeTable.SUM_L = (uint8_t)(checksum & 0xFF);
    ParamCodeTable.SUM_H = (uint8_t)(checksum >> 8);
    
    return SUCCESS;
}

/* Ӌ�ㅢ�����a��У�� */
uint16_t ParamCodeTable_CalculateChecksum(void)
{
    uint16_t sum = 0;
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    
    /* Ӌ���ַ0~674��У�� */
    for (int i = 0; i < 675; i++)
    {
        sum += tablePtr[i];
    }
    
    return sum;
}

/* ��C�������a��У�� */
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

/* �@ȡϵ�y���� */
uint8_t GetSystemParam(uint16_t paramAddress)
{
    if (paramAddress >= sizeof(ParamCodeTable_TypeDef))
    {
        return 0; /* ������ַ�������� */
    }
    
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    return tablePtr[paramAddress];
}

/* �O��ϵ�y���� */
ErrorStatus SetSystemParam(uint16_t paramAddress, uint8_t value)
{
    if (paramAddress >= sizeof(ParamCodeTable_TypeDef))
    {
        return ERROR; /* ������ַ�������� */
    }
    
    uint8_t* tablePtr = (uint8_t*)&ParamCodeTable;
    tablePtr[paramAddress] = value;
    
    return SUCCESS;
}

/* �@ȡ��ǰԇƬ��� */
uint8_t GetStripType(void)
{
    return ParamCodeTable.Display.Strip_Type;
}

/* �@ȡԇƬ������Q */
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

/* Ӌ��CRC16 */
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
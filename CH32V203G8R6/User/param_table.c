/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_table.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Parameter code table implementation for biochemical analyzer.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_table.h"
#include "debug.h"
#include "string.h"

/* Global parameter table structure */
ParamTable_TypeDef gParamTable;

/* CRC table for CRC16 calculation */
static const uint16_t CRC16_Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/* Local Function Prototypes */
static uint8_t Param_ReadFromFlash(void);
static uint8_t Param_WriteToFlash(void);
static uint16_t Param_CRC16_Calculate(uint8_t *data, uint16_t length);

/*********************************************************************
 * @fn      Param_Init
 *
 * @brief   Initialize parameter table from flash or load defaults if invalid
 *
 * @return  Status (0: Success, 1: Error)
 */
uint8_t Param_Init(void)
{
    printf("Parameter table initialization...\r\n");
    
    /* Try to read from flash */
    if (Param_ReadFromFlash() == 0) {
        /* Verify the parameter table */
        if (Param_Verify() == 0) {
            printf("Parameter table loaded successfully.\r\n");
            return 0;
        }
        printf("Parameter table verification failed.\r\n");
    } else {
        printf("Failed to read parameter table from flash.\r\n");
    }
    
    /* Load defaults if reading from flash failed or verification failed */
    if (Param_LoadDefault() == 0) {
        printf("Default parameter table loaded.\r\n");
        return 0;
    }
    
    printf("Parameter table initialization failed.\r\n");
    return 1;
}

/*********************************************************************
 * @fn      Param_LoadDefault
 *
 * @brief   Load default values to the parameter table
 *
 * @return  Status (0: Success, 1: Error)
 */
uint8_t Param_LoadDefault(void)
{
    /* Clear all data to zeros */
    memset(&gParamTable, 0, sizeof(ParamTable_TypeDef));
    
    /* 1. System basic parameters */
    gParamTable.LBT = 58;              /* 2.58V */
    gParamTable.OBT = 55;              /* 2.55V */
    gParamTable.FACTORY = USER_MODE;   /* User mode */
    gParamTable.MODEL_NO = 1;          /* Default model number */
    gParamTable.FW_NO = 1;             /* Default firmware version */
    gParamTable.NOT = 0;               /* No tests performed */
    gParamTable.Code_Table_V = 1;      /* Version 1 of code table */
    
    /* 2. Time settings (will be set properly by RTC) */
    gParamTable.YEAR = 24;             /* 2024 */
    gParamTable.MONTH = 6;             /* June */
    gParamTable.DATE = 12;             /* 12th */
    gParamTable.HOUR = 12;             /* 12:00 */
    gParamTable.MINUTE = 0;
    gParamTable.SECOND = 0;
    
    /* 3. Measurement display parameters */
    gParamTable.TLL = 10;              /* 10.0¡ãC */
    gParamTable.TLH = 40;              /* 40.0¡ãC */
    gParamTable.MGDL = UNIT_MMOL_L;    /* Default unit: mmol/L */
    gParamTable.EVENT = EVENT_AC;      /* Default event: AC */
    gParamTable.Strip_Type = STRIP_TYPE_GLV; /* Default strip: GLV */
    
    /* 4. Hardware calibration parameters */
    gParamTable.EV_T3_TRG = 120;       /* 1200mV */
    gParamTable.EV_WORKING = 164;
    gParamTable.EV_T3 = 164;
    gParamTable.DACO = 0;
    gParamTable.DACDO = 0;
    gParamTable.CC211NoDone = 0;
    gParamTable.CAL_TOL = 8;           /* 0.8% */
    gParamTable.OPS = 1.0f;
    gParamTable.OPI = 0.0f;
    gParamTable.QCT = 0;
    gParamTable.TOFFSET = 0;
    gParamTable.BOFFSET = 0;
    
    /* 5. Blood Glucose (GLV/GAV) specific parameters */
    gParamTable.BG_CSU_TOL = 10;
    gParamTable.BG_NDL = 1000;
    gParamTable.BG_UDL = 800;
    gParamTable.BG_BLOOD_IN = 1500;
    /* Blood glucose reference values */
    gParamTable.BG_L = 20;             /* Display "LO" if below 2.0 mmol/L */
    gParamTable.BG_H = 350;            /* Display "HI" if above 35.0 mmol/L */
    gParamTable.BG_T3_E37 = 2000;
    /* Timing sequence */
    gParamTable.BG_TPL_1 = 50;         /* 5 seconds */
    gParamTable.BG_TRD_1 = 40;         /* 4 seconds */
    gParamTable.BG_EV_Width_1 = 30;    /* 3 seconds */
    gParamTable.BG_TPL_2 = 60;         /* 6 seconds */
    gParamTable.BG_TRD_2 = 50;         /* 5 seconds */
    gParamTable.BG_EV_Width_2 = 40;    /* 4 seconds */
    
    /* 6. Uric Acid (U) specific parameters */
    gParamTable.U_CSU_TOL = 10;
    gParamTable.U_NDL = 1000;
    gParamTable.U_UDL = 800;
    gParamTable.U_BLOOD_IN = 1500;
    /* Uric acid reference values */
    gParamTable.U_L = 20;              /* Display "LO" if below 2.0 mg/dL */
    gParamTable.U_H = 200;             /* Display "HI" if above 20.0 mg/dL */
    gParamTable.U_T3_E37 = 2000;
    /* Timing sequence */
    gParamTable.U_TPL_1 = 60;          /* 6 seconds */
    gParamTable.U_TRD_1 = 50;          /* 5 seconds */
    gParamTable.U_EV_Width_1 = 40;     /* 4 seconds */
    gParamTable.U_TPL_2 = 70;          /* 7 seconds */
    gParamTable.U_TRD_2 = 60;          /* 6 seconds */
    gParamTable.U_EV_Width_2 = 50;     /* 5 seconds */
    
    /* 7. Total Cholesterol (C) specific parameters */
    gParamTable.C_CSU_TOL = 10;
    gParamTable.C_NDL = 1000;
    gParamTable.C_UDL = 800;
    gParamTable.C_BLOOD_IN = 1500;
    /* Cholesterol reference values */
    gParamTable.C_L = 50;              /* Display "LO" if below 50 mg/dL */
    gParamTable.C_H = 500;             /* Display "HI" if above 500 mg/dL */
    gParamTable.C_T3_E37 = 2000;
    /* Timing sequence */
    gParamTable.C_TPL_1 = 150;         /* 15 seconds */
    gParamTable.C_TRD_1 = 140;         /* 14 seconds */
    gParamTable.C_EV_Width_1 = 130;    /* 13 seconds */
    gParamTable.C_TPL_2 = 160;         /* 16 seconds */
    gParamTable.C_TRD_2 = 150;         /* 15 seconds */
    gParamTable.C_EV_Width_2 = 140;    /* 14 seconds */
    
    /* 8. Triglycerides (TG) specific parameters */
    gParamTable.TG_CSU_TOL = 10;
    gParamTable.TG_NDL = 1000;
    gParamTable.TG_UDL = 800;
    gParamTable.TG_BLOOD_IN = 1500;
    /* Triglycerides reference values */
    gParamTable.TG_L = 50;             /* Display "LO" if below 50 mg/dL */
    gParamTable.TG_H = 500;            /* Display "HI" if above 500 mg/dL */
    gParamTable.TG_T3_E37 = 2000;
    /* Timing sequence */
    gParamTable.TG_TPL_1 = 150;        /* 15 seconds */
    gParamTable.TG_TRD_1 = 140;        /* 14 seconds */
    gParamTable.TG_EV_Width_1 = 130;   /* 13 seconds */
    gParamTable.TG_TPL_2 = 160;        /* 16 seconds */
    gParamTable.TG_TRD_2 = 150;        /* 15 seconds */
    gParamTable.TG_EV_Width_2 = 140;   /* 14 seconds */
    
    /* Calculate and store checksums */
    uint16_t checksum = Param_CalculateChecksum();
    gParamTable.SUM_L = checksum & 0xFF;
    gParamTable.SUM_H = (checksum >> 8) & 0xFF;
    
    /* Calculate and store CRC */
    gParamTable.CRC16 = Param_CalculateCRC16();
    
    return 0;
}

/*********************************************************************
 * @fn      Param_Save
 *
 * @brief   Save parameter table to flash
 *
 * @return  Status (0: Success, 1: Error)
 */
uint8_t Param_Save(void)
{
    /* Update checksums before saving */
    uint16_t checksum = Param_CalculateChecksum();
    gParamTable.SUM_L = checksum & 0xFF;
    gParamTable.SUM_H = (checksum >> 8) & 0xFF;
    gParamTable.CRC16 = Param_CalculateCRC16();
    
    /* Write to flash */
    return Param_WriteToFlash();
}

/*********************************************************************
 * @fn      Param_Verify
 *
 * @brief   Verify parameter table checksums
 *
 * @return  Status (0: Valid, 1: Invalid)
 */
uint8_t Param_Verify(void)
{
    /* Check checksum */
    uint16_t calculatedChecksum = Param_CalculateChecksum();
    uint16_t storedChecksum = (gParamTable.SUM_H << 8) | gParamTable.SUM_L;
    
    if (calculatedChecksum != storedChecksum) {
        printf("Checksum mismatch: stored=%04X, calculated=%04X\r\n", 
               storedChecksum, calculatedChecksum);
        return 1;
    }
    
    /* Check CRC */
    uint8_t calculatedCRC = Param_CalculateCRC16();
    if (calculatedCRC != gParamTable.CRC16) {
        printf("CRC mismatch: stored=%02X, calculated=%02X\r\n", 
               gParamTable.CRC16, calculatedCRC);
        return 1;
    }
    
    return 0; /* Valid */
}

/*********************************************************************
 * @fn      Param_CalculateChecksum
 *
 * @brief   Calculate checksum for addresses 0-674
 *
 * @return  Calculated checksum (16-bit)
 */
uint16_t Param_CalculateChecksum(void)
{
    uint16_t sum = 0;
    uint8_t *p = (uint8_t *)&gParamTable;
    
    /* Calculate sum of bytes from address 0 to 674 */
    for (int i = 0; i < 675; i++) {
        sum += p[i];
    }
    
    return sum;
}

/*********************************************************************
 * @fn      Param_CalculateCRC16
 *
 * @brief   Calculate CRC16 for the entire parameter table
 *
 * @return  Calculated CRC16 (8-bit)
 */
uint8_t Param_CalculateCRC16(void)
{
    uint8_t *data = (uint8_t *)&gParamTable;
    uint16_t crc = Param_CRC16_Calculate(data, PARAM_TABLE_SIZE - 1);
    
    /* Return the lower 8 bits as the CRC value */
    return (uint8_t)(crc & 0xFF);
}

/*********************************************************************
 * @fn      Param_ReadFromFlash
 *
 * @brief   Read parameter table from flash
 *
 * @return  Status (0: Success, 1: Error)
 */
static uint8_t Param_ReadFromFlash(void)
{
    uint32_t flash_addr = PARAM_TABLE_BASE_ADDR;
    uint32_t *p_dest = (uint32_t *)&gParamTable;
    uint32_t size_in_words = (PARAM_TABLE_SIZE + 3) / 4; /* Round up to nearest word */
    
    /* Read data from flash */
    for (uint32_t i = 0; i < size_in_words; i++) {
        *p_dest++ = *(uint32_t *)flash_addr;
        flash_addr += 4;
    }
    
    return 0;
}

/*********************************************************************
 * @fn      Param_WriteToFlash
 *
 * @brief   Write parameter table to flash
 *
 * @return  Status (0: Success, 1: Error)
 */
static uint8_t Param_WriteToFlash(void)
{
    uint32_t flash_addr = PARAM_TABLE_BASE_ADDR;
    uint32_t *p_src = (uint32_t *)&gParamTable;
    uint32_t size_in_words = (PARAM_TABLE_SIZE + 3) / 4; /* Round up to nearest word */
    uint8_t result = 0;
    
    /* Unlock flash */
    FLASH_Unlock();
    
    /* Erase the flash page */
    FLASH_Status status = FLASH_ErasePage(flash_addr);
    if (status != FLASH_COMPLETE) {
        printf("Flash erase failed: %d\r\n", status);
        FLASH_Lock();
        return 1;
    }
    
    /* Write data to flash word by word */
    for (uint32_t i = 0; i < size_in_words; i++) {
        status = FLASH_ProgramWord(flash_addr, *p_src);
        if (status != FLASH_COMPLETE) {
            printf("Flash write failed at address 0x%08X: %d\r\n", flash_addr, status);
            result = 1;
            break;
        }
        flash_addr += 4;
        p_src++;
    }
    
    /* Lock flash */
    FLASH_Lock();
    
    return result;
}

/*********************************************************************
 * @fn      Param_CRC16_Calculate
 *
 * @brief   Calculate CRC16 value for given data
 *
 * @param   data - Pointer to the data
 * @param   length - Data length in bytes
 *
 * @return  CRC16 value
 */
static uint16_t Param_CRC16_Calculate(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0;
    
    for (uint16_t i = 0; i < length; i++) {
        crc = (crc << 8) ^ CRC16_Table[(crc >> 8) ^ data[i]];
    }
    
    return crc;
}

/*********************************************************************
 * @fn      Param_GetValue
 *
 * @brief   Get value of a parameter by ID
 *
 * @param   id - Parameter ID
 *
 * @return  Parameter value (or -1 if invalid ID)
 */
int32_t Param_GetValue(Param_ID_TypeDef id)
{
    switch (id) {
        /* System basic parameters */
        case PARAM_LBT:                return gParamTable.LBT;
        case PARAM_OBT:                return gParamTable.OBT;
        case PARAM_FACTORY:            return gParamTable.FACTORY;
        case PARAM_MODEL_NO:           return gParamTable.MODEL_NO;
        case PARAM_FW_NO:              return gParamTable.FW_NO;
        case PARAM_NOT:                return gParamTable.NOT;
        case PARAM_CODE_TABLE_V:       return gParamTable.Code_Table_V;
        
        /* Time setting parameters */
        case PARAM_YEAR:               return gParamTable.YEAR;
        case PARAM_MONTH:              return gParamTable.MONTH;
        case PARAM_DATE:               return gParamTable.DATE;
        case PARAM_HOUR:               return gParamTable.HOUR;
        case PARAM_MINUTE:             return gParamTable.MINUTE;
        case PARAM_SECOND:             return gParamTable.SECOND;
        
        /* Measurement display parameters */
        case PARAM_TLL:                return gParamTable.TLL;
        case PARAM_TLH:                return gParamTable.TLH;
        case PARAM_MGDL:               return gParamTable.MGDL;
        case PARAM_EVENT:              return gParamTable.EVENT;
        case PARAM_STRIP_TYPE:         return gParamTable.Strip_Type;
        
        /* Hardware calibration parameters */
        case PARAM_EV_T3_TRG:          return gParamTable.EV_T3_TRG;
        case PARAM_EV_WORKING:         return gParamTable.EV_WORKING;
        case PARAM_EV_T3:              return gParamTable.EV_T3;
        case PARAM_DACO:               return gParamTable.DACO;
        case PARAM_DACDO:              return gParamTable.DACDO;
        case PARAM_CC211_NO_DONE:      return gParamTable.CC211NoDone;
        case PARAM_CAL_TOL:            return gParamTable.CAL_TOL;
        case PARAM_TOFFSET:            return gParamTable.TOFFSET;
        case PARAM_BOFFSET:            return gParamTable.BOFFSET;
        
        /* Test-specific parameters */
        case PARAM_BG_L:               return gParamTable.BG_L;
        case PARAM_BG_H:               return gParamTable.BG_H;
        case PARAM_U_L:                return gParamTable.U_L;
        case PARAM_U_H:                return gParamTable.U_H;
        case PARAM_C_L:                return gParamTable.C_L;
        case PARAM_C_H:                return gParamTable.C_H;
        case PARAM_TG_L:               return gParamTable.TG_L;
        case PARAM_TG_H:               return gParamTable.TG_H;
        
        default:
            return -1; /* Invalid parameter ID */
    }
}

/*********************************************************************
 * @fn      Param_SetValue
 *
 * @brief   Set value of a parameter by ID
 *
 * @param   id - Parameter ID
 * @param   value - Parameter value to set
 *
 * @return  Status (0: Success, 1: Error)
 */
uint8_t Param_SetValue(Param_ID_TypeDef id, int32_t value)
{
    switch (id) {
        /* System basic parameters */
        case PARAM_LBT:                gParamTable.LBT = value; break;
        case PARAM_OBT:                gParamTable.OBT = value; break;
        case PARAM_FACTORY:            gParamTable.FACTORY = value; break;
        case PARAM_MODEL_NO:           gParamTable.MODEL_NO = value; break;
        case PARAM_FW_NO:              gParamTable.FW_NO = value; break;
        case PARAM_NOT:                gParamTable.NOT = value; break;
        case PARAM_CODE_TABLE_V:       gParamTable.Code_Table_V = value; break;
        
        /* Time setting parameters */
        case PARAM_YEAR:               gParamTable.YEAR = value; break;
        case PARAM_MONTH:              gParamTable.MONTH = value; break;
        case PARAM_DATE:               gParamTable.DATE = value; break;
        case PARAM_HOUR:               gParamTable.HOUR = value; break;
        case PARAM_MINUTE:             gParamTable.MINUTE = value; break;
        case PARAM_SECOND:             gParamTable.SECOND = value; break;
        
        /* Measurement display parameters */
        case PARAM_TLL:                gParamTable.TLL = value; break;
        case PARAM_TLH:                gParamTable.TLH = value; break;
        case PARAM_MGDL:               gParamTable.MGDL = value; break;
        case PARAM_EVENT:              gParamTable.EVENT = value; break;
        case PARAM_STRIP_TYPE:         gParamTable.Strip_Type = value; break;
        
        /* Hardware calibration parameters */
        case PARAM_EV_T3_TRG:          gParamTable.EV_T3_TRG = value; break;
        case PARAM_EV_WORKING:         gParamTable.EV_WORKING = value; break;
        case PARAM_EV_T3:              gParamTable.EV_T3 = value; break;
        case PARAM_DACO:               gParamTable.DACO = value; break;
        case PARAM_DACDO:              gParamTable.DACDO = value; break;
        case PARAM_CC211_NO_DONE:      gParamTable.CC211NoDone = value; break;
        case PARAM_CAL_TOL:            gParamTable.CAL_TOL = value; break;
        case PARAM_TOFFSET:            gParamTable.TOFFSET = value; break;
        case PARAM_BOFFSET:            gParamTable.BOFFSET = value; break;
        
        /* Test-specific parameters */
        case PARAM_BG_L:               gParamTable.BG_L = value; break;
        case PARAM_BG_H:               gParamTable.BG_H = value; break;
        case PARAM_U_L:                gParamTable.U_L = value; break;
        case PARAM_U_H:                gParamTable.U_H = value; break;
        case PARAM_C_L:                gParamTable.C_L = value; break;
        case PARAM_C_H:                gParamTable.C_H = value; break;
        case PARAM_TG_L:               gParamTable.TG_L = value; break;
        case PARAM_TG_H:               gParamTable.TG_H = value; break;
        
        default:
            return 1; /* Invalid parameter ID */
    }
    
    return 0; /* Success */
}

/*********************************************************************
 * @fn      Param_GetStripTypeName
 *
 * @brief   Get strip type name as a string
 *
 * @param   type - Strip type value
 *
 * @return  String representation of strip type
 */
const char* Param_GetStripTypeName(uint8_t type)
{
    switch (type) {
        case STRIP_TYPE_GLV: return "GLV";  /* ÑªÌÇ */
        case STRIP_TYPE_U:   return "U";    /* ÄòËá */
        case STRIP_TYPE_C:   return "C";    /* ¿‚Ä‘¹Ì´¼ */
        case STRIP_TYPE_TG:  return "TG";   /* ÈýËá¸ÊÓÍÖ¬ */
        case STRIP_TYPE_GAV: return "GAV";  /* ÑªÌÇ */
        default:            return "?";    /* Î´Öª */
    }
}

/*********************************************************************
 * @fn      Param_GetEventTypeName
 *
 * @brief   Get event type name as a string
 *
 * @param   event - Event type value
 *
 * @return  String representation of event type
 */
const char* Param_GetEventTypeName(uint8_t event)
{
    switch (event) {
        case EVENT_QC:  return "QC";   /* Æ·¹ÜÒº */
        case EVENT_AC:  return "AC";   /* ïˆÇ° */
        case EVENT_PC:  return "PC";   /* ïˆáá */
        default:       return "?";    /* Î´Öª */
    }
}

/*********************************************************************
 * @fn      Param_GetUnitName
 *
 * @brief   Get measurement unit name as a string
 *
 * @param   unit - Unit type value
 *
 * @return  String representation of unit
 */
const char* Param_GetUnitName(uint8_t unit)
{
    switch (unit) {
        case UNIT_MMOL_L:  return "mmol/L";
        case UNIT_MG_DL:   return "mg/dL";
        case UNIT_GM_DL:   return "gm/dL";
        default:          return "?";
    }
}

/*********************************************************************
 * @fn      ParamTable_Test
 *
 * @brief   Test function for parameter table operations
 *
 * @return  None
 */
void ParamTable_Test(void)
{
    printf("\r\n===== Parameter Table Test =====\r\n");
    
    /* Initialize parameter table */
    printf("1. Initializing parameter table...\r\n");
    if (Param_Init() == 0) {
        printf("   Parameter table initialized successfully.\r\n");
    } else {
        printf("   Parameter table initialization failed!\r\n");
        return;
    }
    
    /* Display current values */
    printf("2. Current parameter values:\r\n");
    printf("   - Model No: %d\r\n", Param_GetValue(PARAM_MODEL_NO));
    printf("   - Firmware: %d\r\n", Param_GetValue(PARAM_FW_NO));
    printf("   - Factory Mode: %s\r\n", 
           Param_GetValue(PARAM_FACTORY) == FACTORY_MODE ? "YES" : "NO");
    printf("   - Strip Type: %s\r\n", 
           Param_GetStripTypeName(Param_GetValue(PARAM_STRIP_TYPE)));
    printf("   - Measurement Unit: %s\r\n", 
           Param_GetUnitName(Param_GetValue(PARAM_MGDL)));
    printf("   - Event Type: %s\r\n", 
           Param_GetEventTypeName(Param_GetValue(PARAM_EVENT)));
    
    /* Test parameter modification */
    printf("3. Modifying a parameter...\r\n");
    uint8_t original_event = Param_GetValue(PARAM_EVENT);
    uint8_t new_event = (original_event + 1) % 3; /* Cycle through event types */
    printf("   - Changing EVENT from %s to %s\r\n", 
           Param_GetEventTypeName(original_event),
           Param_GetEventTypeName(new_event));
    
    Param_SetValue(PARAM_EVENT, new_event);
    printf("   - New EVENT value: %s\r\n", 
           Param_GetEventTypeName(Param_GetValue(PARAM_EVENT)));
    
    /* Test parameter saving */
    printf("4. Saving parameter table...\r\n");
    if (Param_Save() == 0) {
        printf("   Parameter table saved successfully.\r\n");
    } else {
        printf("   Parameter table save failed!\r\n");
    }
    
    /* Verify parameter table */
    printf("5. Verifying parameter table...\r\n");
    if (Param_Verify() == 0) {
        printf("   Parameter table verification passed.\r\n");
    } else {
        printf("   Parameter table verification failed!\r\n");
    }
    
    /* Test parameter reload */
    printf("6. Reloading parameter table from flash...\r\n");
    if (Param_Init() == 0) {
        printf("   Parameter table reloaded successfully.\r\n");
        printf("   - EVENT value after reload: %s\r\n", 
               Param_GetEventTypeName(Param_GetValue(PARAM_EVENT)));
    } else {
        printf("   Parameter table reload failed!\r\n");
    }
    
    /* Reset to original value */
    printf("7. Restoring original value...\r\n");
    Param_SetValue(PARAM_EVENT, original_event);
    Param_Save();
    printf("   - EVENT restored to: %s\r\n", 
           Param_GetEventTypeName(Param_GetValue(PARAM_EVENT)));
    
    printf("===== Parameter Table Test Completed =====\r\n\r\n");
} 
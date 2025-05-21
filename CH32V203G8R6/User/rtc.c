/********************************** (C) COPYRIGHT *******************************
 * File Name          : rtc.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/21
 * Description        : 多功能生化測試儀RTC功能
*********************************************************************************
* Copyright (c) 2025 HMD Biomedical.
*******************************************************************************/

#include "rtc.h"
#include "ch32v20x_rtc.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_pwr.h"
#include "debug.h"

/*********************************************************************
 * @fn      RTC_Init
 *
 * @brief   初始化RTC，使用內部震盪器並配置時間。
 *
 * @return  none
 */
void RTC_Init(void)
{
    /* 啟用PWR和BKP的時鐘 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    
    /* 允許訪問RTC和備份寄存器 */
    PWR_BackupAccessCmd(ENABLE);
    
    /* 重置備份域 */
    BKP_DeInit();
    
    /* 啟用LSI作為RTC時鐘源 */
    RCC_LSICmd(ENABLE);
    
    /* 等待LSI啟動 */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
    
    /* 選擇LSI作為RTC時鐘源 */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    
    /* 啟用RTC時鐘 */
    RCC_RTCCLKCmd(ENABLE);
    
    /* 等待RTC寄存器同步 */
    RTC_WaitForSynchro();
    
    /* 等待上一次操作完成 */
    RTC_WaitForLastTask();
    
    /* 設置RTC預分頻器，以產生1秒的時基 */
    /* LSI通常運行在40KHz左右，因此我們使用39999的分頻值 (40000-1) */
    RTC_SetPrescaler(39999);
    
    /* 等待設置完成 */
    RTC_WaitForLastTask();
    
    /* 從參數表獲取時間並設置RTC */
    RTC_SyncWithParam();
    
    printf("RTC Initialized\r\n");
}

/*********************************************************************
 * @fn      RTC_Config
 *
 * @brief   配置RTC，但不修改當前時間
 *
 * @return  none
 */
void RTC_Config(void)
{
    /* 檢查是否已經完成配置 */
    if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        RTC_Init();
        
        /* 設置備份寄存器來標記RTC已被配置 */
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    else
    {
        /* 檢查是否需要重置RTC */
        if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
            printf("Power On Reset Occurred...\r\n");
        }
        else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
            printf("External Reset Occurred...\r\n");
        }
        
        printf("No need to reconfigure RTC...\r\n");
        
        /* 允許訪問RTC和備份寄存器 */
        PWR_BackupAccessCmd(ENABLE);
        
        /* 等待RTC寄存器同步 */
        RTC_WaitForSynchro();
        
        /* 啟用秒中斷 */
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        
        /* 等待設置完成 */
        RTC_WaitForLastTask();
    }
    
    /* 從RTC更新參數表中的時間 */
    RTC_UpdateParam();
}

/*********************************************************************
 * @fn      RTC_SetTime
 *
 * @brief   設置RTC時間
 *
 * @param   year, month, date, hour, minute, second - 時間參數
 *
 * @return  none
 */
void RTC_SetTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    uint32_t counter_val = 0;
    uint16_t days_since_2000 = 0;
    uint8_t leap_years = 0;
    uint16_t days_in_year = 0;
    uint8_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t i;
    
    /* 計算自2000年以來的閏年數 */
    leap_years = (year / 4);
    if(year % 4 == 0 && month <= 2)
    {
        leap_years--;
    }
    
    /* 計算自2000年以來的天數 */
    days_since_2000 = year * 365 + leap_years;
    
    /* 如果當前年是閏年，且月份大於2月，則2月有29天 */
    if(year % 4 == 0)
    {
        days_in_month[2] = 29;
    }
    
    /* 計算當前年中已經過去的天數 */
    for(i = 1; i < month; i++)
    {
        days_in_year += days_in_month[i];
    }
    days_in_year += (date - 1);
    
    days_since_2000 += days_in_year;
    
    /* 計算總秒數 */
    counter_val = days_since_2000 * 24 * 3600 + hour * 3600 + minute * 60 + second;
    
    /* 設置RTC計數器值 */
    RTC_WaitForLastTask();
    RTC_SetCounter(counter_val);
    RTC_WaitForLastTask();
    
    printf("RTC Time Set: %02d/%02d/%02d %02d:%02d:%02d\r\n", 
           2000 + year, month, date, hour, minute, second);
}

/*********************************************************************
 * @fn      RTC_GetTime
 *
 * @brief   獲取RTC時間
 *
 * @param   year, month, date, hour, minute, second - 時間參數指針
 *
 * @return  none
 */
void RTC_GetTime(uint8_t* year, uint8_t* month, uint8_t* date, uint8_t* hour, uint8_t* minute, uint8_t* second)
{
    uint32_t time_counter = RTC_GetCounter();
    uint32_t days_since_2000, days_in_year, seconds_in_day;
    uint16_t temp_year;
    uint8_t leap_year;
    uint8_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    /* 計算自2000年以來的天數 */
    days_since_2000 = time_counter / (24 * 3600);
    seconds_in_day = time_counter % (24 * 3600);
    
    /* 計算小時、分鐘和秒 */
    *hour = seconds_in_day / 3600;
    *minute = (seconds_in_day % 3600) / 60;
    *second = (seconds_in_day % 3600) % 60;
    
    /* 計算年份 */
    temp_year = 0;
    days_in_year = days_since_2000;
    
    while(days_in_year >= (temp_year % 4 == 0 ? 366 : 365))
    {
        days_in_year -= (temp_year % 4 == 0 ? 366 : 365);
        temp_year++;
    }
    
    *year = temp_year;
    
    /* 判斷是否為閏年 */
    leap_year = (temp_year % 4 == 0) ? 1 : 0;
    if(leap_year)
    {
        days_in_month[2] = 29;
    }
    
    /* 計算月份和日期 */
    *month = 1;
    while(days_in_year >= days_in_month[*month])
    {
        days_in_year -= days_in_month[*month];
        (*month)++;
    }
    
    *date = days_in_year + 1;
}

/*********************************************************************
 * @fn      RTC_SyncWithParam
 *
 * @brief   用參數表中的時間同步RTC
 *
 * @return  none
 */
void RTC_SyncWithParam(void)
{
    uint8_t year, month, date, hour, minute, second;
    
    /* 從參數表中獲取時間 */
    if(PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second))
    {
        /* 設置RTC時間 */
        RTC_SetTime(year, month, date, hour, minute, second);
        printf("RTC Synced with Parameter Table\r\n");
    }
    else
    {
        printf("Failed to get time from Parameter Table\r\n");
    }
}

/*********************************************************************
 * @fn      RTC_UpdateParam
 *
 * @brief   用RTC當前時間更新參數表
 *
 * @return  none
 */
void RTC_UpdateParam(void)
{
    uint8_t year, month, date, hour, minute, second;
    
    /* 從RTC獲取當前時間 */
    RTC_GetTime(&year, &month, &date, &hour, &minute, &second);
    
    /* 更新參數表 */
    if(PARAM_SetDateTime(year, month, date, hour, minute, second))
    {
        printf("Parameter Table Time Updated: %02d/%02d/%02d %02d:%02d:%02d\r\n", 
               2000 + year, month, date, hour, minute, second);
    }
    else
    {
        printf("Failed to update time in Parameter Table\r\n");
    }
}

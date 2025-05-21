/********************************** (C) COPYRIGHT *******************************
 * File Name          : rtc.c
 * Author             : HMD Team
 * Version            : V1.0.0
 * Date               : 2025/05/21
 * Description        : �๦�������yԇ�xRTC����
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
 * @brief   ��ʼ��RTC��ʹ�ÃȲ���U���K���Õr�g��
 *
 * @return  none
 */
void RTC_Init(void)
{
    /* ����PWR��BKP�ĕr� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    
    /* ���S�L��RTC�͂�ݼĴ��� */
    PWR_BackupAccessCmd(ENABLE);
    
    /* ���Â���� */
    BKP_DeInit();
    
    /* ����LSI����RTC�r�Դ */
    RCC_LSICmd(ENABLE);
    
    /* �ȴ�LSI���� */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
    
    /* �x��LSI����RTC�r�Դ */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    
    /* ����RTC�r� */
    RCC_RTCCLKCmd(ENABLE);
    
    /* �ȴ�RTC�Ĵ���ͬ�� */
    RTC_WaitForSynchro();
    
    /* �ȴ���һ�β������ */
    RTC_WaitForLastTask();
    
    /* �O��RTC�A���l�����Ԯa��1��ĕr�� */
    /* LSIͨ���\����40KHz���ң�����҂�ʹ��39999�ķ��lֵ (40000-1) */
    RTC_SetPrescaler(39999);
    
    /* �ȴ��O����� */
    RTC_WaitForLastTask();
    
    /* �ą�����@ȡ�r�g�K�O��RTC */
    RTC_SyncWithParam();
    
    printf("RTC Initialized\r\n");
}

/*********************************************************************
 * @fn      RTC_Config
 *
 * @brief   ����RTC�������޸Į�ǰ�r�g
 *
 * @return  none
 */
void RTC_Config(void)
{
    /* �z���Ƿ��ѽ�������� */
    if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        RTC_Init();
        
        /* �O�Â�ݼĴ������ӛRTC�ѱ����� */
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    else
    {
        /* �z���Ƿ���Ҫ����RTC */
        if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
            printf("Power On Reset Occurred...\r\n");
        }
        else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
            printf("External Reset Occurred...\r\n");
        }
        
        printf("No need to reconfigure RTC...\r\n");
        
        /* ���S�L��RTC�͂�ݼĴ��� */
        PWR_BackupAccessCmd(ENABLE);
        
        /* �ȴ�RTC�Ĵ���ͬ�� */
        RTC_WaitForSynchro();
        
        /* �������Д� */
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        
        /* �ȴ��O����� */
        RTC_WaitForLastTask();
    }
    
    /* ��RTC���������еĕr�g */
    RTC_UpdateParam();
}

/*********************************************************************
 * @fn      RTC_SetTime
 *
 * @brief   �O��RTC�r�g
 *
 * @param   year, month, date, hour, minute, second - �r�g����
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
    
    /* Ӌ����2000���ԁ���c�ꔵ */
    leap_years = (year / 4);
    if(year % 4 == 0 && month <= 2)
    {
        leap_years--;
    }
    
    /* Ӌ����2000���ԁ���씵 */
    days_since_2000 = year * 365 + leap_years;
    
    /* �����ǰ�����c�꣬���·ݴ��2�£��t2����29�� */
    if(year % 4 == 0)
    {
        days_in_month[2] = 29;
    }
    
    /* Ӌ�㮔ǰ�����ѽ��^ȥ���씵 */
    for(i = 1; i < month; i++)
    {
        days_in_year += days_in_month[i];
    }
    days_in_year += (date - 1);
    
    days_since_2000 += days_in_year;
    
    /* Ӌ�㿂�딵 */
    counter_val = days_since_2000 * 24 * 3600 + hour * 3600 + minute * 60 + second;
    
    /* �O��RTCӋ����ֵ */
    RTC_WaitForLastTask();
    RTC_SetCounter(counter_val);
    RTC_WaitForLastTask();
    
    printf("RTC Time Set: %02d/%02d/%02d %02d:%02d:%02d\r\n", 
           2000 + year, month, date, hour, minute, second);
}

/*********************************************************************
 * @fn      RTC_GetTime
 *
 * @brief   �@ȡRTC�r�g
 *
 * @param   year, month, date, hour, minute, second - �r�g����ָ�
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
    
    /* Ӌ����2000���ԁ���씵 */
    days_since_2000 = time_counter / (24 * 3600);
    seconds_in_day = time_counter % (24 * 3600);
    
    /* Ӌ��С�r����犺��� */
    *hour = seconds_in_day / 3600;
    *minute = (seconds_in_day % 3600) / 60;
    *second = (seconds_in_day % 3600) % 60;
    
    /* Ӌ����� */
    temp_year = 0;
    days_in_year = days_since_2000;
    
    while(days_in_year >= (temp_year % 4 == 0 ? 366 : 365))
    {
        days_in_year -= (temp_year % 4 == 0 ? 366 : 365);
        temp_year++;
    }
    
    *year = temp_year;
    
    /* �Д��Ƿ���c�� */
    leap_year = (temp_year % 4 == 0) ? 1 : 0;
    if(leap_year)
    {
        days_in_month[2] = 29;
    }
    
    /* Ӌ���·ݺ����� */
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
 * @brief   �Å������еĕr�gͬ��RTC
 *
 * @return  none
 */
void RTC_SyncWithParam(void)
{
    uint8_t year, month, date, hour, minute, second;
    
    /* �ą������Ы@ȡ�r�g */
    if(PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second))
    {
        /* �O��RTC�r�g */
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
 * @brief   ��RTC��ǰ�r�g��������
 *
 * @return  none
 */
void RTC_UpdateParam(void)
{
    uint8_t year, month, date, hour, minute, second;
    
    /* ��RTC�@ȡ��ǰ�r�g */
    RTC_GetTime(&year, &month, &date, &hour, &minute, &second);
    
    /* �������� */
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

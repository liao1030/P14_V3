/********************************** (C) COPYRIGHT *******************************
 * File Name          : interrupt_handler.c
 * Author             : HMD System Engineer  
 * Version            : V1.0
 * Date               : 2025/09/25
 * Description        : CH582F GPIO中斷處理程序，用於試片插入喚醒功能
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "HAL.h"
#include "StripDetect.h"
#include "CH58x_common.h"

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void StripDetect_WakeupFromSleep(void);

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA中斷處理函數，處理PA15 (Strip_Detect_5) 的中斷
 *
 * @param   none
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    uint16_t flag = GPIOA_ReadITFlagPort();
    
    // 檢查是否是PA15 (Strip_Detect_5) 的中斷
    if(flag & GPIO_Pin_15)
    {
        // 清除中斷標誌
        GPIOA_ClearITFlagBit(GPIO_Pin_15);
        
        // 處理試片插入喚醒
        StripDetect_WakeupFromSleep();
        
        PRINT("GPIO PA15 interrupt - strip inserted, system wakeup\n");
    }
}

/*********************************************************************
 * @fn      GPIOB_IRQHandler
 *
 * @brief   GPIOB中斷處理函數，處理PB11 (Strip_Detect_3) 的中斷
 *
 * @param   none
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void)
{
    uint16_t flag = GPIOB_ReadITFlagPort();
    
    // 檢查是否是PB11 (Strip_Detect_3) 的中斷
    if(flag & GPIO_Pin_11)
    {
        // 清除中斷標誌
        GPIOB_ClearITFlagBit(GPIO_Pin_11);
        
        // 處理試片插入喚醒
        StripDetect_WakeupFromSleep();
        
        PRINT("GPIO PB11 interrupt - strip inserted, system wakeup\n");
    }
}
/********************************** (C) COPYRIGHT *******************************
 * File Name          : system_state.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/22
 * Description        : 系統狀態機相關功能
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#include "debug.h"
#include "system_state.h"

/* 定義全局變量 */
static SystemState_TypeDef currentSystemState = STATE_IDLE;

/*********************************************************************
 * @fn      System_SetState
 *
 * @brief   設置系統狀態
 *
 * @param   newState - 新狀態
 *
 * @return  none
 */
void System_SetState(SystemState_TypeDef newState)
{
    if (newState != currentSystemState) {
        printf("System State: %d -> %d\r\n", currentSystemState, newState);
        currentSystemState = newState;
    }
}

/*********************************************************************
 * @fn      System_GetState
 *
 * @brief   獲取當前系統狀態
 *
 * @return  當前系統狀態
 */
SystemState_TypeDef System_GetState(void)
{
    return currentSystemState;
}

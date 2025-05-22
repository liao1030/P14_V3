/********************************** (C) COPYRIGHT *******************************
 * File Name          : system_state.c
 * Author             : HMD System Engineer
 * Version            : V1.0
 * Date               : 2025/05/22
 * Description        : ϵ�y��B�C���P����
 ********************************************************************************
 * Copyright (c) 2025 HMD Technology Corp.
 *******************************************************************************/

#include "debug.h"
#include "system_state.h"

/* ���xȫ��׃�� */
static SystemState_TypeDef currentSystemState = STATE_IDLE;

/*********************************************************************
 * @fn      System_SetState
 *
 * @brief   �O��ϵ�y��B
 *
 * @param   newState - � �B
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
 * @brief   �@ȡ��ǰϵ�y��B
 *
 * @return  ��ǰϵ�y��B
 */
SystemState_TypeDef System_GetState(void)
{
    return currentSystemState;
}

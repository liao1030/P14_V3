/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 This example demonstrates receiving indeterminate length data via USART's IDLE interrupt and DMA.
 USART2_Tx(PA2), USART2_Rx(PA3).


*/

#include "debug.h"
#include "param_utils.h"
#include "param_table.h"
#include "param_code_table.h"
#include "string.h"
#include "param_table.h"
#include "uart_protocol.h"
#include "strip_detect.h"

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

// ring buffer size
#define RING_BUFFER_LEN     (1024u)

// The length of a single buffer used by DMA
#define RX_BUFFER_LEN       (128u)

#define USART_RX_CH         DMA1_Channel6



struct
{
    volatile uint8_t DMA_USE_BUFFER;
    uint8_t          Rx_Buffer[2][RX_BUFFER_LEN];

} USART_DMA_CTRL = {
    .DMA_USE_BUFFER = 0,
    .Rx_Buffer      = {0},
};

struct
{
    uint8_t           buffer[RING_BUFFER_LEN];
    volatile uint16_t RecvPos;  //
    volatile uint16_t SendPos;  //
    volatile uint16_t RemainCount;

} ring_buffer = {{0}, 0, 0, 0};

/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART2 peripheral.
 *
 * @return  none
 */
void USARTx_CFG(uint32_t baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure  = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef  NVIC_InitStructure  = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* USART2 TX-->A.2   RX-->A.3 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = baudrate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}


/*********************************************************************
 * @fn      DMA_INIT
 *
 * @brief   Configures the DMA for USART2.
 *
 * @return  none
 */
void DMA_INIT(void)
{
    DMA_InitTypeDef  DMA_InitStructure  = {0};
    NVIC_InitTypeDef NVIC_InitStructure  = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(USART_RX_CH);
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)USART_DMA_CTRL.Rx_Buffer[0];
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = RX_BUFFER_LEN;
    DMA_Init(USART_RX_CH, &DMA_InitStructure);

    DMA_ITConfig(USART_RX_CH, DMA_IT_TC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_Cmd(USART_RX_CH, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
}


/*********************************************************************
 * @fn      ring_buffer_push_huge
 *
 * @brief   Put a large amount of data into the ring buffer.
 *
 * @return  none
 */
void ring_buffer_push_huge(uint8_t *buffer, uint16_t len)
{
    const uint16_t bufferRemainCount = RING_BUFFER_LEN - ring_buffer.RemainCount;
    if (bufferRemainCount < len)
    {
        len = bufferRemainCount;
    }

    const uint16_t bufferSize = RING_BUFFER_LEN - ring_buffer.RecvPos;
    if (bufferSize >= len)
    {
        memcpy(&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, len);
        ring_buffer.RecvPos += len;
    }
    else
    {
        uint16_t otherSize = len - bufferSize;
        memcpy(&(ring_buffer.buffer[ring_buffer.RecvPos]), buffer, bufferSize);
        memcpy(ring_buffer.buffer, &(buffer[bufferSize]), otherSize);
        ring_buffer.RecvPos = otherSize;
    }
    ring_buffer.RemainCount += len;
}

/*********************************************************************
 * @fn      ring_buffer_pop
 *
 * @brief   Get a data from the ring buffer.
 *
 * @return  the Data
 */
uint8_t ring_buffer_pop()
{
    uint8_t data = ring_buffer.buffer[ring_buffer.SendPos];

    ring_buffer.SendPos++;
    if (ring_buffer.SendPos >= RING_BUFFER_LEN)
    {
        ring_buffer.SendPos = 0;
    }
    ring_buffer.RemainCount--;
    return data;
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Delay_Init();
    USART_Printf_Init(115200);
    USARTx_CFG(115200);
    DMA_INIT();
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("This is an example\r\n");
    
    /* 初始化參數表 */
    PARAM_Init();
    printf("Parameter Table Initialized\r\n");
    
    /* 初始化UART通訊協議 */
    UART_Protocol_Init();
    printf("UART Protocol Initialized\r\n");
    
    /* 初始化試片偵測相關設置 */
    STRIP_DETECT_Init();
    
    /* 顯示一些基本系統資訊 */
    printf("Model No: %d\r\n", PARAM_GetByte(PARAM_MODEL_NO));
    printf("FW Version: %d.%d\r\n", PARAM_GetByte(PARAM_FW_NO)/10, PARAM_GetByte(PARAM_FW_NO)%10);
    printf("Parameter Table Version: %d\r\n", PARAM_GetWord(PARAM_CODE_TABLE_V));
    printf("Test Count: %d\r\n", PARAM_GetWord(PARAM_NOT));
    
    /* 顯示當前設定的試片類型 */
    StripType_TypeDef stripType = (StripType_TypeDef)PARAM_GetByte(PARAM_STRIP_TYPE);
    printf("Strip Type: %s\r\n", StripType_GetName(stripType));
    
    /* 顯示當前設定的單位 */
    Unit_TypeDef unit = (Unit_TypeDef)PARAM_GetByte(PARAM_MGDL);
    printf("Unit: %s\r\n", Unit_GetSymbol(unit));

    while (1)
    {
        // 處理UART協議資料
        UART_Protocol_Process();
        
        // 處理試片偵測相關任務
        STRIP_DETECT_Process();
        
        // /* 測試環境可以保留以下程式碼，以便於偵錯 */
        // if (ring_buffer.RemainCount > 0)
        // {
        //     printf("UART Echo Test: %d bytes\r\n", ring_buffer.RemainCount);
            
        //     while (ring_buffer.RemainCount > 0)
        //     {
        //         uint8_t data = ring_buffer_pop();
        //         // 通過 USART2 將接收到的資料發送回去
        //         USART_SendData(USART2, data);
        //         // 等待發送完成
        //         while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        //     }
            
        //     printf("Echo Complete\r\n");
        // }
        
        Delay_Ms(10); // 減少延遲時間以增加處理速度
    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 global interrupt request.
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // IDLE
        uint16_t rxlen     = (RX_BUFFER_LEN - USART_RX_CH->CNTR);
        uint8_t  oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;

        USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;

        DMA_Cmd(USART_RX_CH, DISABLE);
        DMA_SetCurrDataCounter(USART_RX_CH, RX_BUFFER_LEN);
        // Switch buffer
        USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);
        DMA_Cmd(USART_RX_CH, ENABLE);

        USART_ReceiveData(USART2); // clear IDLE flag
        ring_buffer_push_huge(USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);
        
        // 如果接收到資料，呼叫UART協議處理函數
        uint8_t i;
        for(i = 0; i < rxlen; i++)
        {
            UART2_Receive_Byte_ISR(USART_DMA_CTRL.Rx_Buffer[oldbuffer][i]);
        }
    }
    
    // 檢查是否有接收到資料
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // 讀取接收到的位元組並處理
        uint8_t data = USART_ReceiveData(USART2);
        UART2_Receive_Byte_ISR(data);
    }
}

/*********************************************************************
 * @fn      DMA1_Channel6_IRQHandler
 *
 * @brief   This function handles DMA1 Channel 6 global interrupt request.
 *
 * @return  none
 */
void DMA1_Channel6_IRQHandler(void)
{
    uint16_t rxlen     = RX_BUFFER_LEN;
    uint8_t  oldbuffer = USART_DMA_CTRL.DMA_USE_BUFFER;
    // FULL

    USART_DMA_CTRL.DMA_USE_BUFFER = !oldbuffer;

    DMA_Cmd(USART_RX_CH, DISABLE);
    DMA_SetCurrDataCounter(USART_RX_CH, RX_BUFFER_LEN);
    // Switch buffer
    USART_RX_CH->MADDR = (uint32_t)(USART_DMA_CTRL.Rx_Buffer[USART_DMA_CTRL.DMA_USE_BUFFER]);
    DMA_Cmd(USART_RX_CH, ENABLE);

    ring_buffer_push_huge(USART_DMA_CTRL.Rx_Buffer[oldbuffer], rxlen);
    
    // 處理DMA接收到的資料
    uint8_t i;
    for(i = 0; i < rxlen; i++)
    {
        UART2_Receive_Byte_ISR(USART_DMA_CTRL.Rx_Buffer[oldbuffer][i]);
    }

    DMA_ClearITPendingBit(DMA1_IT_TC6);
}

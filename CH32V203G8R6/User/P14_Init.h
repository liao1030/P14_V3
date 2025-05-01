#ifndef __P14_INIT_H__
#define __P14_INIT_H__

#include "debug.h"
#include "ch32v20x.h"
#include "P14_Parameter_Table.h"

/* GPIO≥ı ºªØœ‡ÍP∫Øîµ */
void P14_CH32V203_GPIO_Init(void);
void P14_CH32V203_UART_Init(void);
void P14_CH32V203_ParameterTable_Init(void);
void P14_CH32V203_System_Init(void);

#endif /* __P14_INIT_H__ */ 
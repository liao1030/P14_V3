#ifndef __P14_INIT_H__
#define __P14_INIT_H__

#include "CH58x_common.h"

/* 防彈跳設定 */
#define STRIP_DEBOUNCE_TIME    20      // 防彈跳時間（毫秒）
#define STRIP_DETECT_INTERVAL  100     // 試片偵測間隔（毫秒）

/* 試片狀態定義 */
typedef enum {
    STRIP_STATE_NONE,      // 無試片
    STRIP_STATE_INSERTING, // 插入過程中（防彈跳）
    STRIP_STATE_INSERTED,  // 已插入
    STRIP_STATE_READY      // 已準備好測量
} StripState_TypeDef;

/* 試片類型定義 (與CH32V203通用) */
/* 注意：試片類型枚舉由P14_Parameter_Table.h提供，
 * 在CH32V203端已定義，這裡僅宣告而非定義，
 * 確保CH582F和CH32V203使用相同的枚舉值。
 */
typedef enum {
    STRIP_TYPE_UNKNOWN = 255, // 未知或錯誤
    STRIP_TYPE_GLV = 0,       // 血糖(GLV)
    STRIP_TYPE_U = 1,         // 尿酸
    STRIP_TYPE_C = 2,         // 總膽固醇
    STRIP_TYPE_TG = 3,        // 三酸甘油脂
    STRIP_TYPE_GAV = 4        // 血糖(GAV)
} StripType_TypeDef;

/* GPIO初始化相關函數 */
void P14_CH582F_GPIO_Init(void);
void P14_CH582F_UART_Init(void);
void P14_CH582F_System_Init(void);

/* 試片插入偵測相關函數 */
void P14_StripDetectInit(void);
uint8_t P14_CheckStripInsertion(void);
StripType_TypeDef P14_IdentifyStripType(void);
void P14_NotifyStripInserted(StripType_TypeDef type);
void P14_StripStateReset(void);

/* UART1接收處理相關函數 */
void P14_UART1_RxHandler(uint8_t rx_data);
void P14_UART1_CheckTimeout(void);

#endif /* __P14_INIT_H__ */ 
/********************************** (C) COPYRIGHT *******************************
 * File Name          : P14_BLE_Protocol.h
 * Author             : HMD
 * Version            : V1.0
 * Date               : 2024/08/10
 * Description        : P14項目CH582F藍牙通訊協議定義
 *********************************************************************************
 * Copyright (c) 2024 HMD
 *******************************************************************************/

#ifndef __P14_BLE_PROTOCOL_H__
#define __P14_BLE_PROTOCOL_H__

#include "CH58x_common.h"

/* CH582F-CH32V203通訊命令定義 */
#define CMD_START_MARKER        0xAA    // 命令開始標記
#define CMD_END_MARKER          0xBB    // 命令結束標記

/* 命令類型 */
#define CMD_MEASURE_T1_OUT      0x01    // 測量T1_OUT電壓
#define CMD_STRIP_INSERTED      0x02    // 試片插入通知
#define CMD_STRIP_TYPE_RESULT   0x03    // 試片類型判斷結果
#define CMD_START_MEASUREMENT   0x04    // 開始測量

/* T1電極測量結果定義 */
#define T1_OUT_NEAR_0V          0       // T1_OUT接近0V
#define T1_OUT_NEAR_2_5V        1       // T1_OUT接近2.5V

/* 協議處理函數 */
void P14_BLE_ProtocolInit(void);
void P14_BLE_UartCallback(uint8_t *data, uint16_t length);

#endif /* __P14_BLE_PROTOCOL_H__ */ 
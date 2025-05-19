/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_code_table.h
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : 多功能生化測試儀參數代碼表頭文件
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#ifndef __PARAM_CODE_TABLE_H
#define __PARAM_CODE_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* 系統基本參數 */
#define PARAM_LBT                   0    // 低電池閾值
#define PARAM_OBT                   1    // 電池耗盡閾值
#define PARAM_FACTORY               2    // 儀器操作模式
#define PARAM_MODEL_NO              3    // 產品型號
#define PARAM_FW_NO                 4    // 韌體版本號
#define PARAM_NOT                   5    // 測試次數 (2 bytes)
#define PARAM_CODE_TABLE_V          7    // 代碼表版本編號 (2 bytes)

/* 時間設定參數 */
#define PARAM_YEAR                  9    // 年份設定
#define PARAM_MONTH                10    // 月份設定
#define PARAM_DATE                 11    // 日期設定
#define PARAM_HOUR                 12    // 小時設定
#define PARAM_MINUTE               13    // 分鐘設定
#define PARAM_SECOND               14    // 秒數設定

/* 測試環境參數 */
#define PARAM_TLL                  15    // 操作溫度範圍下限
#define PARAM_TLH                  16    // 操作溫度範圍上限
#define PARAM_MGDL                 17    // 測量單位設定
#define PARAM_EVENT                18    // 預設事件類型
#define PARAM_STRIP_TYPE           19    // 測試項目

/* 硬體校準參數 */
#define PARAM_EV_T3_TRG            20    // EV_T3觸發電壓 (2 bytes)
#define PARAM_EV_WORKING           22    // 測量工作電極電壓
#define PARAM_EV_T3                23    // 血液檢測電極電壓
#define PARAM_CAL_TOL              24    // OPS/OPI校準容差
#define PARAM_OPS                  25    // OPA校準斜率 (4 bytes)
#define PARAM_OPI                  29    // OPA校準截距 (4 bytes)
#define PARAM_QCT                  33    // QCT校準測試低位元組
#define PARAM_TOFFSET              34    // 溫度校準偏移
#define PARAM_BOFFSET              35    // 電池校準偏移

/* 血糖(GLV)專用參數 */
/* 試片參數 */
#define PARAM_GLV_CSU_TOL          36    // 試片檢查容差
#define PARAM_GLV_NDL              37    // 新試片檢測水平 (2 bytes)
#define PARAM_GLV_UDL              39    // 已使用試片檢測水平 (2 bytes)
#define PARAM_GLV_BLOOD_IN         41    // 血液檢測水平 (2 bytes)
#define PARAM_GLV_EV_W_BACKGROUND  43    // W電極背景值 (2 bytes)
#define PARAM_GLV_EV_W_PWM_DUTY    45    // W電極PWM占空比 (2 bytes)
#define PARAM_GLV_COUNT_DOWN_TIME  47    // 倒數時間

/* 測量範圍參數 */
#define PARAM_GLV_L                48    // 血糖值下限
#define PARAM_GLV_H                49    // 血糖值上限

/* 測試時序參數 */
#define PARAM_GLV_TPL_1            50    // 時間脈衝低 (第一組) (2 bytes)
#define PARAM_GLV_TRD_1            52    // 原始數據時間 (第一組) (2 bytes)
#define PARAM_GLV_EV_WIDTH_1       54    // 燃燒時間 (第一組) (2 bytes)
#define PARAM_GLV_TPL_2            56    // 時間脈衝低 (第二組) (2 bytes)
#define PARAM_GLV_TRD_2            58    // 原始數據時間 (第二組) (2 bytes)
#define PARAM_GLV_EV_WIDTH_2       60    // 燃燒時間 (第二組) (2 bytes)

/* 計算式補償參數 */
#define PARAM_GLV_S2               62    // 試片補償參數
#define PARAM_GLV_I2               63    // 試片補償參數
#define PARAM_GLV_SQ               64    // 品管液QC補償參數Sq
#define PARAM_GLV_IQ               65    // 品管液QC補償參數Iq
#define PARAM_GLV_SR               66    // OPA硬體迴路補償(斜率) (4 bytes)
#define PARAM_GLV_IR               70    // OPA硬體迴路補償(截距) (4 bytes)
#define PARAM_GLV_GLU0             74    // 多段校正參數(量測分段點) (14 bytes)
#define PARAM_GLV_GOAL0            88    // 多段校正參數(目標值分段點) (14 bytes)
#define PARAM_GLV_S3               102   // 試片補償參數S3(1-30) (30 bytes)
#define PARAM_GLV_I3               132   // 試片補償參數I3(1-30) (30 bytes)

/* 溫度補償參數 */
#define PARAM_GLV_TF               162   // AC/PC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_GLV_CTF              169   // QC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_GLV_TO               176   // AC/PC溫度補償截距(10-40°C) (7 bytes)
#define PARAM_GLV_CTO              183   // QC溫度補償截距(10-40°C) (7 bytes)

/* 計算後優化補償參數 */
#define PARAM_GLV_CVQ              190   // CV Level of QC
#define PARAM_GLV_AQ               191   // A of QC Compensation
#define PARAM_GLV_BQ               192   // B of QC Compensation
#define PARAM_GLV_TCV              193   // Time of BG Optimize
#define PARAM_GLV_CVBG             194   // CV Level of BG
#define PARAM_GLV_ABG              195   // A of BG Compensation
#define PARAM_GLV_BBG              196   // B of BG Compensation
#define PARAM_GLV_AQ5              197   // A of QC Compensation Level 5
#define PARAM_GLV_BQ5              198   // B of QC Compensation Level 5

/* 尿酸(U)專用參數 */
/* 試片參數 */
#define PARAM_U_CSU_TOL            199   // 試片檢查容差
#define PARAM_U_NDL                200   // 新試片檢測水平 (2 bytes)
#define PARAM_U_UDL                202   // 已使用試片檢測水平 (2 bytes)
#define PARAM_U_BLOOD_IN           204   // 血液檢測水平 (2 bytes)
#define PARAM_U_EV_W_BACKGROUND    206   // W電極背景值 (2 bytes)
#define PARAM_U_EV_W_PWM_DUTY      208   // W電極PWM占空比 (2 bytes)
#define PARAM_U_COUNT_DOWN_TIME    210   // 倒數時間

/* 測量範圍參數 */
#define PARAM_U_L                  211   // 尿酸值下限
#define PARAM_U_H                  212   // 尿酸值上限

/* 測試時序參數 */
#define PARAM_U_TPL_1              213   // 時間脈衝低 (第一組) (2 bytes)
#define PARAM_U_TRD_1              215   // 原始數據時間 (第一組) (2 bytes)
#define PARAM_U_EV_WIDTH_1         217   // 燃燒時間 (第一組) (2 bytes)
#define PARAM_U_TPL_2              219   // 時間脈衝低 (第二組) (2 bytes)
#define PARAM_U_TRD_2              221   // 原始數據時間 (第二組) (2 bytes)
#define PARAM_U_EV_WIDTH_2         223   // 燃燒時間 (第二組) (2 bytes)

/* 計算式補償參數 */
#define PARAM_U_S2                 225   // 試片補償參數
#define PARAM_U_I2                 226   // 試片補償參數
#define PARAM_U_SQ                 227   // 品管液QC補償參數Sq
#define PARAM_U_IQ                 228   // 品管液QC補償參數Iq
#define PARAM_U_SR                 229   // OPA硬體迴路補償(斜率) (4 bytes)
#define PARAM_U_IR                 233   // OPA硬體迴路補償(截距) (4 bytes)
#define PARAM_U_GLU0               237   // 多段校正參數(量測分段點) (14 bytes)
#define PARAM_U_GOAL0              251   // 多段校正參數(目標值分段點) (14 bytes)
#define PARAM_U_S3                 265   // 試片補償參數S3(1-15) (15 bytes)
#define PARAM_U_I3                 280   // 試片補償參數I3(1-15) (15 bytes)

/* 溫度補償參數 */
#define PARAM_U_TF                 295   // AC/PC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_U_CTF                302   // QC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_U_TO                 309   // AC/PC溫度補償截距(10-40°C) (7 bytes)
#define PARAM_U_CTO                316   // QC溫度補償截距(10-40°C) (7 bytes)

/* 計算後優化補償參數 */
#define PARAM_U_CVQ                323   // CV Level of QC
#define PARAM_U_AQ                 324   // A of QC Compensation
#define PARAM_U_BQ                 325   // B of QC Compensation
#define PARAM_U_TCV                326   // Time of BG Optimize
#define PARAM_U_CVBG               327   // CV Level of BG
#define PARAM_U_ABG                328   // A of BG Compensation
#define PARAM_U_BBG                329   // B of BG Compensation
#define PARAM_U_AQ5                330   // A of QC Compensation Level 5
#define PARAM_U_BQ5                331   // B of QC Compensation Level 5

/* 總膽固醇(C)專用參數 */
/* 試片參數 */
#define PARAM_C_CSU_TOL            332   // 試片檢查容差
#define PARAM_C_NDL                333   // 新試片檢測水平 (2 bytes)
#define PARAM_C_UDL                335   // 已使用試片檢測水平 (2 bytes)
#define PARAM_C_BLOOD_IN           337   // 血液檢測水平 (2 bytes)
#define PARAM_C_EV_W_BACKGROUND    339   // W電極背景值 (2 bytes)
#define PARAM_C_EV_W_PWM_DUTY      341   // W電極PWM占空比 (2 bytes)
#define PARAM_C_COUNT_DOWN_TIME    343   // 倒數時間

/* 測量範圍參數 */
#define PARAM_C_L                  344   // 膽固醇值下限
#define PARAM_C_H                  345   // 膽固醇值上限

/* 測試時序參數 */
#define PARAM_C_TPL_1              346   // 時間脈衝低 (第一組) (2 bytes)
#define PARAM_C_TRD_1              348   // 原始數據時間 (第一組) (2 bytes)
#define PARAM_C_EV_WIDTH_1         350   // 燃燒時間 (第一組) (2 bytes)
#define PARAM_C_TPL_2              352   // 時間脈衝低 (第二組) (2 bytes)
#define PARAM_C_TRD_2              354   // 原始數據時間 (第二組) (2 bytes)
#define PARAM_C_EV_WIDTH_2         356   // 燃燒時間 (第二組) (2 bytes)

/* 計算式補償參數 */
#define PARAM_C_S2                 358   // 試片補償參數
#define PARAM_C_I2                 359   // 試片補償參數
#define PARAM_C_SQ                 360   // 品管液QC補償參數Sq
#define PARAM_C_IQ                 361   // 品管液QC補償參數Iq
#define PARAM_C_SR                 362   // OPA硬體迴路補償(斜率) (4 bytes)
#define PARAM_C_IR                 366   // OPA硬體迴路補償(截距) (4 bytes)
#define PARAM_C_GLU0               370   // 多段校正參數(量測分段點) (14 bytes)
#define PARAM_C_GOAL0              384   // 多段校正參數(目標值分段點) (14 bytes)
#define PARAM_C_S3                 398   // 試片補償參數S3(1-15) (15 bytes)
#define PARAM_C_I3                 413   // 試片補償參數I3(1-15) (15 bytes)

/* 溫度補償參數 */
#define PARAM_C_TF                 428   // AC/PC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_C_CTF                435   // QC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_C_TO                 442   // AC/PC溫度補償截距(10-40°C) (7 bytes)
#define PARAM_C_CTO                449   // QC溫度補償截距(10-40°C) (7 bytes)

/* 計算後優化補償參數 */
#define PARAM_C_CVQ                456   // CV Level of QC
#define PARAM_C_AQ                 457   // A of QC Compensation
#define PARAM_C_BQ                 458   // B of QC Compensation
#define PARAM_C_TCV                459   // Time of BG Optimize
#define PARAM_C_CVBG               460   // CV Level of BG
#define PARAM_C_ABG                461   // A of BG Compensation
#define PARAM_C_BBG                462   // B of BG Compensation
#define PARAM_C_AQ5                463   // A of QC Compensation Level 5
#define PARAM_C_BQ5                464   // B of QC Compensation Level 5

/* 三酸甘油脂(TG)專用參數 */
/* 試片參數 */
#define PARAM_TG_CSU_TOL           465   // 試片檢查容差
#define PARAM_TG_NDL               466   // 新試片檢測水平 (2 bytes)
#define PARAM_TG_UDL               468   // 已使用試片檢測水平 (2 bytes)
#define PARAM_TG_BLOOD_IN          470   // 血液檢測水平 (2 bytes)
#define PARAM_TG_EV_W_BACKGROUND   472   // W電極背景值 (2 bytes)
#define PARAM_TG_EV_W_PWM_DUTY     474   // W電極PWM占空比 (2 bytes)
#define PARAM_TG_COUNT_DOWN_TIME   476   // 倒數時間

/* 測量範圍參數 */
#define PARAM_TG_L                 477   // 三酸甘油脂值下限
#define PARAM_TG_H                 478   // 三酸甘油脂值上限

/* 測試時序參數 */
#define PARAM_TG_TPL_1             479   // 時間脈衝低 (第一組) (2 bytes)
#define PARAM_TG_TRD_1             481   // 原始數據時間 (第一組) (2 bytes)
#define PARAM_TG_EV_WIDTH_1        483   // 燃燒時間 (第一組) (2 bytes)
#define PARAM_TG_TPL_2             485   // 時間脈衝低 (第二組) (2 bytes)
#define PARAM_TG_TRD_2             487   // 原始數據時間 (第二組) (2 bytes)
#define PARAM_TG_EV_WIDTH_2        489   // 燃燒時間 (第二組) (2 bytes)

/* 計算式補償參數 */
#define PARAM_TG_S2                491   // 試片補償參數
#define PARAM_TG_I2                492   // 試片補償參數
#define PARAM_TG_SQ                493   // 品管液QC補償參數Sq
#define PARAM_TG_IQ                494   // 品管液QC補償參數Iq
#define PARAM_TG_SR                495   // OPA硬體迴路補償(斜率) (4 bytes)
#define PARAM_TG_IR                499   // OPA硬體迴路補償(截距) (4 bytes)
#define PARAM_TG_GLU0              503   // 多段校正參數(量測分段點) (14 bytes)
#define PARAM_TG_GOAL0             517   // 多段校正參數(目標值分段點) (14 bytes)
#define PARAM_TG_S3                531   // 試片補償參數S3(1-15) (15 bytes)
#define PARAM_TG_I3                546   // 試片補償參數I3(1-15) (15 bytes)

/* 溫度補償參數 */
#define PARAM_TG_TF                561   // AC/PC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_TG_CTF               568   // QC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_TG_TO                575   // AC/PC溫度補償截距(10-40°C) (7 bytes)
#define PARAM_TG_CTO               582   // QC溫度補償截距(10-40°C) (7 bytes)

/* 計算後優化補償參數 */
#define PARAM_TG_CVQ               589   // CV Level of QC
#define PARAM_TG_AQ                590   // A of QC Compensation
#define PARAM_TG_BQ                591   // B of QC Compensation
#define PARAM_TG_TCV               592   // Time of BG Optimize
#define PARAM_TG_CVBG              593   // CV Level of BG
#define PARAM_TG_ABG               594   // A of BG Compensation
#define PARAM_TG_BBG               595   // B of BG Compensation
#define PARAM_TG_AQ5               596   // A of QC Compensation Level 5
#define PARAM_TG_BQ5               597   // B of QC Compensation Level 5

/* 血糖(GAV)專用參數 */
/* 試片參數 */
#define PARAM_GAV_CSU_TOL          598   // 試片檢查容差
#define PARAM_GAV_NDL              599   // 新試片檢測水平 (2 bytes)
#define PARAM_GAV_UDL              601   // 已使用試片檢測水平 (2 bytes)
#define PARAM_GAV_BLOOD_IN         603   // 血液檢測水平 (2 bytes)
#define PARAM_GAV_EV_W_BACKGROUND  605   // W電極背景值 (2 bytes)
#define PARAM_GAV_EV_W_PWM_DUTY    607   // W電極PWM占空比 (2 bytes)
#define PARAM_GAV_COUNT_DOWN_TIME  609   // 倒數時間

/* 測量範圍參數 */
#define PARAM_GAV_L                610   // 血糖值下限
#define PARAM_GAV_H                611   // 血糖值上限
#define PARAM_GAV_T3_E37           612   // T3 ADV錯誤37閾值 (2 bytes)

/* 測試時序參數 */
#define PARAM_GAV_TPL_1            614   // 時間脈衝低 (第一組) (2 bytes)
#define PARAM_GAV_TRD_1            616   // 原始數據時間 (第一組) (2 bytes)
#define PARAM_GAV_EV_WIDTH_1       618   // 燃燒時間 (第一組) (2 bytes)
#define PARAM_GAV_TPL_2            620   // 時間脈衝低 (第二組) (2 bytes)
#define PARAM_GAV_TRD_2            622   // 原始數據時間 (第二組) (2 bytes)
#define PARAM_GAV_EV_WIDTH_2       624   // 燃燒時間 (第二組) (2 bytes)

/* 計算式補償參數 */
#define PARAM_GAV_S2               626   // 試片補償參數
#define PARAM_GAV_I2               627   // 試片補償參數
#define PARAM_GAV_SQ               628   // 品管液QC補償參數Sq
#define PARAM_GAV_IQ               629   // 品管液QC補償參數Iq
#define PARAM_GAV_SR               630   // OPA硬體迴路補償(斜率) (4 bytes)
#define PARAM_GAV_IR               634   // OPA硬體迴路補償(截距) (4 bytes)
#define PARAM_GAV_GLU0             638   // 多段校正參數(量測分段點) (14 bytes)
#define PARAM_GAV_GOAL0            652   // 多段校正參數(目標值分段點) (14 bytes)
#define PARAM_GAV_S3               666   // 試片補償參數S3(1-30) (30 bytes)
#define PARAM_GAV_I3               696   // 試片補償參數I3(1-30) (30 bytes)

/* AC補償參數 */
#define PARAM_GAV_AC_L_WAVE_L      726   // AC振幅判斷 (2 bytes)
#define PARAM_GAV_AC_H_WAVE_L      728   // AC振幅判斷 (2 bytes)
#define PARAM_GAV_S4_0             730   // AC補償 (HCT10)
#define PARAM_GAV_S4_1             731   // AC補償 (HCT20)
#define PARAM_GAV_S4_2             732   // AC補償 (HCT30)
#define PARAM_GAV_S4_3             733   // AC補償 (HCT40)
#define PARAM_GAV_S4_4             734   // AC補償 (HCT50)
#define PARAM_GAV_S4_5             735   // AC補償 (HCT60)
#define PARAM_GAV_S4_6             736   // AC補償 (HCT70)
#define PARAM_GAV_I4_0             737   // AC補償 (HCT10)
#define PARAM_GAV_I4_1             738   // AC補償 (HCT20)
#define PARAM_GAV_I4_2             739   // AC補償 (HCT30)
#define PARAM_GAV_I4_3             740   // AC補償 (HCT40)
#define PARAM_GAV_I4_4             741   // AC補償 (HCT50)
#define PARAM_GAV_I4_5             742   // AC補償 (HCT60)
#define PARAM_GAV_I4_6             743   // AC補償 (HCT70)
#define PARAM_GAV_AC_ADC_0         744   // AC補償判定 (HCT10) (2 bytes)
#define PARAM_GAV_AC_ADC_1         746   // AC補償判定 (HCT20) (2 bytes)
#define PARAM_GAV_AC_ADC_2         748   // AC補償判定 (HCT30) (2 bytes)
#define PARAM_GAV_AC_ADC_3         750   // AC補償判定 (HCT40) (2 bytes)
#define PARAM_GAV_AC_ADC_4         752   // AC補償判定 (HCT50) (2 bytes)
#define PARAM_GAV_AC_ADC_5         754   // AC補償判定 (HCT60) (2 bytes)
#define PARAM_GAV_AC_ADC_6         756   // AC補償判定 (HCT70) (2 bytes)

/* 溫度補償參數 */
#define PARAM_GAV_TF               758   // AC/PC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_GAV_CTF              765   // QC溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_GAV_TO               772   // AC/PC溫度補償截距(10-40°C) (7 bytes)
#define PARAM_GAV_CTO              779   // QC溫度補償截距(10-40°C) (7 bytes)
#define PARAM_GAV_AC_TF            786   // AC(交流)溫度補償斜率(10-40°C) (7 bytes)
#define PARAM_GAV_AC_TO            793   // AC(交流)溫度補償截距(10-40°C) (7 bytes)

/* 計算後優化補償參數 */
#define PARAM_GAV_CVQ              800   // CV Level of QC
#define PARAM_GAV_AQ               801   // A of QC Compensation
#define PARAM_GAV_BQ               802   // B of QC Compensation
#define PARAM_GAV_TCV              803   // Time of BG Optimize
#define PARAM_GAV_CVBG             804   // CV Level of BG
#define PARAM_GAV_ABG              805   // A of BG Compensation
#define PARAM_GAV_BBG              806   // B of BG Compensation
#define PARAM_GAV_AQ5              807   // A of QC Compensation Level 5
#define PARAM_GAV_BQ5              808   // B of QC Compensation Level 5

/* 保留與校驗區 */
#define PARAM_RESERVED             809   // 保留區域供未來擴展 (36 bytes)
#define PARAM_SUM_L                845   // 地址0~844校驗和(低)
#define PARAM_SUM_H                846   // 地址0~844校驗和(高)
#define PARAM_CRC16                847   // 完整參數表CRC校驗

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_CODE_TABLE_H */

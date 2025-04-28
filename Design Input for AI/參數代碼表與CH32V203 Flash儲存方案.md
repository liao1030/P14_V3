## 目錄
1. [多功能生化測試儀參數代碼表](#多功能生化測試儀參數代碼表)
   - [系統基本參數](#1-系統基本參數)
   - [時間設定參數](#2-時間設定參數)
   - [測試環境參數](#3-測試環境參數)
   - [硬體校準參數](#4-硬體校準參數)
   - [血糖(GLV/GAV)專用參數](#5-血糖glvgav專用參數)
   - [尿酸(U)專用參數](#6-尿酸u專用參數)
   - [總膽固醇(C)專用參數](#7-總膽固醇c專用參數)
   - [三酸甘油脂(TG)專用參數](#8-三酸甘油脂tg專用參數)
   - [保留與校驗區](#9-保留與校驗區)
2. [CH32V203 Flash 參數儲存方案](#ch32v203-flash-參數儲存方案)
   - [硬體特性](#硬體特性)
   - [記憶體分配](#記憶體分配)
   - [參數表區域細分](#參數表區域細分)
   - [參數表資料結構設計](#參數表資料結構設計)
   - [參數存取函數設計](#參數存取函數設計)
   - [參數使用範例](#參數使用範例)
   - [結構重組的優勢](#結構重組的優勢)

## 多功能生化測試儀參數代碼表

### 1. 系統基本參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 0 | LBT | 低電池閾值 | 1 | 58=2.58V (電池圖示閃爍) | Meter |
| 1 | OBT | 電池耗盡閾值 | 1 | 55=2.55V (電池圖示常亮) | APP |
| 2 | FACTORY | 儀器操作模式 | 1 | 0:使用者模式; 210:工廠模式 | Meter |
| 3 | MODEL_NO | 產品型號 | 1 | 由韌體修改 | Meter |
| 4 | FW_NO | 韌體版本號 | 1 | 由韌體修改 | Meter |
| 5-6 | NOT | 測試次數 | 2 | 已完成的測試次數 | Meter |
| 7-8 | Code_Table_V | 代碼表版本編號 | 2 | 1~65535 | Meter+APP |

### 2. 時間設定參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 9 | YEAR | 年份設定 | 1 | 0~99 (2000~2099年) | Meter |
| 10 | MONTH | 月份設定 | 1 | 1~12 | Meter |
| 11 | DATE | 日期設定 | 1 | 1~31 | Meter |
| 12 | HOUR | 小時設定 | 1 | 0~23 | Meter |
| 13 | MINUTE | 分鐘設定 | 1 | 0~59 | Meter |
| 14 | SECOND | 秒數設定 | 1 | 0~59 | Meter |

### 3. 測試環境參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 15 | TLL | 操作溫度範圍下限 | 1 | 1=1.0°C | Meter |
| 16 | TLH | 操作溫度範圍上限 | 1 | 40=40.0°C | Meter |
| 17 | MGDL | 測量單位設定 | 1 | 0:mmol/L; 1:mg/dL; 2:gm/dl | Meter+APP |
| 18 | EVENT | 預設事件類型 | 1 | 0:QC, 1:AC, 2:PC | APP+Meter |
| 19 | Strip_Type | 測試項目 | 1 | 0:GLV(血糖), 1:U(尿酸), 2:C(總膽固醇), 3:TG(三酸甘油脂), 4:GAV(血糖) | APP+Meter |

### 4. 硬體校準參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 20-21 | EV_T3_TRG | EV_T3觸發電壓 | 2 | 40~250 (400~2500mV) | Meter |
| 22 | EV_WORKING | 測量工作電極電壓 | 1 | 164 ((12bitADC)/4) | Meter |
| 23 | EV_T3 | 血液檢測電極電壓 | 1 | 164 ((12bitADC)/4) | Meter |
| 24 | DACO | DAC偏移補償 | 1 | -128~127 | Meter |
| 25 | DACDO | DAC校正管理參數 | 1 | 0:執行校正,其他:不動作 | Meter |
| 26 | CC211NoDone | CC211未完成旗標 | 1 | 0:正常,其他:顯示E12 | Meter |
| 27 | CAL_TOL | OPS/OPI校準容差 | 1 | 8 (0.8%) | Meter |
| 28-31 | OPS | OPA校準斜率 | 4 | 浮點數 | Meter |
| 32-35 | OPI | OPA校準截距 | 4 | 浮點數 | Meter |
| 36 | QCT | QCT校準測試低位元組 | 1 | - | Meter |
| 37 | TOFFSET | 溫度校準偏移 | 1 | -128~127 | Meter |
| 38 | BOFFSET | 電池校準偏移 | 1 | -128~127 | Meter |

### 5. 血糖(GLV/GAV)專用參數

#### 5.1 試片參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 39 | BG_CSU_TOL | 試片檢查容差 | 1 | 10 (3%容差) | Meter |
| 40-41 | BG_NDL | 新試片檢測水平 | 2 | - | Meter |
| 42-43 | BG_UDL | 已使用試片檢測水平 | 2 | - | Meter |
| 44-45 | BG_BLOOD_IN | 血液檢測水平 | 2 | - | Meter |
| 46-61 | BG_Strip_Lot | 試片批號(16字節) | 16 | ASCII碼 | Meter |

#### 5.2 測量範圍參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 62 | BG_L | 血糖值下限 | 1 | 顯示"LO"如果低於BG_L*10 | Meter |
| 63 | BG_H | 血糖值上限 | 1 | 顯示"HI"如果高於BG_H*10 | Meter |
| 64-65 | BG_T3_E37 | T3 ADV錯誤37閾值 | 2 | 如果T3 ADV>T3_E37，顯示E37 | Meter |

#### 5.3 測試時序參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 66-67 | BG_TPL_1 | 時間脈衝低 (第一組) | 2 | 單位:100ms | Meter |
| 68-69 | BG_TRD_1 | 原始數據時間 (第一組) | 2 | 單位:100ms | Meter |
| 70-71 | BG_EV_Width_1 | 燃燒時間 (第一組) | 2 | 單位:100ms | Meter |
| 72-73 | BG_TPL_2 | 時間脈衝低 (第二組) | 2 | 單位:100ms | Meter |
| 74-75 | BG_TRD_2 | 原始數據時間 (第二組) | 2 | 單位:100ms | Meter |
| 76-77 | BG_EV_Width_2 | 燃燒時間 (第二組) | 2 | 單位:100ms | Meter |

#### 5.4 品管液(QC)參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 78 | BG_Sq | QC補償參數Sq | 1 | 計算參數 | Meter |
| 79 | BG_Iq | QC補償參數Iq | 1 | 計算參數 | Meter |
| 80 | BG_CVq | QC水平的CV | 1 | 0~100% (100=關閉QC優化) | Meter |
| 81 | BG_Aq | QC補償參數A | 1 | 0~10 | Meter |
| 82 | BG_Bq | QC補償參數B | 1 | 0~10 | Meter |
| 83 | BG_Aq5 | QC補償參數A(等級5) | 1 | 0~10 | Meter |
| 84 | BG_Bq5 | QC補償參數B(等級5) | 1 | 0~10 | Meter |
| 85-114 | BG_QC1-QC5 | QC標準範圍參數 | 30 | QC5為最優先 | Meter |

#### 5.5 計算式補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 115 | BG_S2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 116 | BG_I2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 117-120 | BG_Sr | OPA硬體迴路補償(斜率) | 4 | 浮點數 | Meter |
| 121-124 | BG_Ir | OPA硬體迴路補償(截距) | 4 | 浮點數 | Meter |
| 125-154 | BG_S3 | 試片補償參數S3(1-30) | 30 | AC/PC計算參數 | Meter |
| 155-184 | BG_I3 | 試片補償參數I3(1-30) | 30 | AC/PC計算參數 | Meter |

#### 5.6 溫度補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 185-191 | BG_TF | AC/PC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 192-198 | BG_CTF | QC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 199-205 | BG_TO | AC/PC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |
| 206-212 | BG_CTO | QC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |

### 6. 尿酸(U)專用參數

#### 6.1 試片參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 213 | U_CSU_TOL | 試片檢查容差 | 1 | 10 (3%容差) | Meter |
| 214-215 | U_NDL | 新試片檢測水平 | 2 | - | Meter |
| 216-217 | U_UDL | 已使用試片檢測水平 | 2 | - | Meter |
| 218-219 | U_BLOOD_IN | 血液檢測水平 | 2 | - | Meter |
| 220-235 | U_Strip_Lot | 試片批號(16字節) | 16 | ASCII碼 | Meter |

#### 6.2 測量範圍參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 236 | U_L | 尿酸值下限 | 1 | 顯示"LO"如果低於設定值 | Meter |
| 237 | U_H | 尿酸值上限 | 1 | 顯示"HI"如果高於設定值 | Meter |
| 238-239 | U_T3_E37 | T3 ADV錯誤37閾值 | 2 | 如果T3 ADV>T3_E37，顯示E37 | Meter |

#### 6.3 測試時序參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 240-241 | U_TPL_1 | 時間脈衝低 (第一組) | 2 | 單位:100ms | Meter |
| 242-243 | U_TRD_1 | 原始數據時間 (第一組) | 2 | 單位:100ms | Meter |
| 244-245 | U_EV_Width_1 | 燃燒時間 (第一組) | 2 | 單位:100ms | Meter |
| 246-247 | U_TPL_2 | 時間脈衝低 (第二組) | 2 | 單位:100ms | Meter |
| 248-249 | U_TRD_2 | 原始數據時間 (第二組) | 2 | 單位:100ms | Meter |
| 250-251 | U_EV_Width_2 | 燃燒時間 (第二組) | 2 | 單位:100ms | Meter |

#### 6.4 品管液(QC)參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 252 | U_Sq | QC補償參數Sq | 1 | 計算參數 | Meter |
| 253 | U_Iq | QC補償參數Iq | 1 | 計算參數 | Meter |
| 254 | U_CVq | QC水平的CV | 1 | 0~100% (100=關閉QC優化) | Meter |
| 255 | U_Aq | QC補償參數A | 1 | 0~10 | Meter |
| 256 | U_Bq | QC補償參數B | 1 | 0~10 | Meter |
| 257-286 | U_QC1-QC5 | QC標準範圍參數 | 30 | QC5為最優先 | Meter |

#### 6.5 計算式補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 287 | U_S2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 288 | U_I2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 289-292 | U_Sr | OPA硬體迴路補償(斜率) | 4 | 浮點數 | Meter |
| 293-296 | U_Ir | OPA硬體迴路補償(截距) | 4 | 浮點數 | Meter |
| 297-311 | U_S3 | 試片補償參數S3(1-15) | 15 | 計算參數 | Meter |
| 312-326 | U_I3 | 試片補償參數I3(1-15) | 15 | 計算參數 | Meter |

#### 6.6 溫度補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 327-333 | U_TF | AC/PC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 334-340 | U_CTF | QC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 341-347 | U_TO | AC/PC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |
| 348-354 | U_CTO | QC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |

### 7. 總膽固醇(C)專用參數

#### 7.1 試片參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 355 | C_CSU_TOL | 試片檢查容差 | 1 | 10 (3%容差) | Meter |
| 356-357 | C_NDL | 新試片檢測水平 | 2 | - | Meter |
| 358-359 | C_UDL | 已使用試片檢測水平 | 2 | - | Meter |
| 360-361 | C_BLOOD_IN | 血液檢測水平 | 2 | - | Meter |
| 362-377 | C_Strip_Lot | 試片批號(16字節) | 16 | ASCII碼 | Meter |

#### 7.2 測量範圍參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 378 | C_L | 膽固醇值下限 | 1 | 顯示"LO"如果低於設定值 | Meter |
| 379 | C_H | 膽固醇值上限 | 1 | 顯示"HI"如果高於設定值 | Meter |
| 380-381 | C_T3_E37 | T3 ADV錯誤37閾值 | 2 | 如果T3 ADV>T3_E37，顯示E37 | Meter |

#### 7.3 測試時序參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 382-383 | C_TPL_1 | 時間脈衝低 (第一組) | 2 | 單位:100ms | Meter |
| 384-385 | C_TRD_1 | 原始數據時間 (第一組) | 2 | 單位:100ms | Meter |
| 386-387 | C_EV_Width_1 | 燃燒時間 (第一組) | 2 | 單位:100ms | Meter |
| 388-389 | C_TPL_2 | 時間脈衝低 (第二組) | 2 | 單位:100ms | Meter |
| 390-391 | C_TRD_2 | 原始數據時間 (第二組) | 2 | 單位:100ms | Meter |
| 392-393 | C_EV_Width_2 | 燃燒時間 (第二組) | 2 | 單位:100ms | Meter |

#### 7.4 品管液(QC)參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 394 | C_Sq | QC補償參數Sq | 1 | 計算參數 | Meter |
| 395 | C_Iq | QC補償參數Iq | 1 | 計算參數 | Meter |
| 396 | C_CVq | QC水平的CV | 1 | 0~100% (100=關閉QC優化) | Meter |
| 397 | C_Aq | QC補償參數A | 1 | 0~10 | Meter |
| 398 | C_Bq | QC補償參數B | 1 | 0~10 | Meter |
| 399-428 | C_QC1-QC5 | QC標準範圍參數 | 30 | QC5為最優先 | Meter |

#### 7.5 計算式補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 429 | C_S2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 430 | C_I2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 431-434 | C_Sr | OPA硬體迴路補償(斜率) | 4 | 浮點數 | Meter |
| 435-438 | C_Ir | OPA硬體迴路補償(截距) | 4 | 浮點數 | Meter |
| 439-453 | C_S3 | 試片補償參數S3(1-15) | 15 | 計算參數 | Meter |
| 454-468 | C_I3 | 試片補償參數I3(1-15) | 15 | 計算參數 | Meter |

#### 7.6 溫度補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 469-475 | C_TF | AC/PC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 476-482 | C_CTF | QC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 483-489 | C_TO | AC/PC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |
| 490-496 | C_CTO | QC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |

### 8. 三酸甘油脂(TG)專用參數

#### 8.1 試片參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 497 | TG_CSU_TOL | 試片檢查容差 | 1 | 10 (3%容差) | Meter |
| 498-499 | TG_NDL | 新試片檢測水平 | 2 | - | Meter |
| 500-501 | TG_UDL | 已使用試片檢測水平 | 2 | - | Meter |
| 502-503 | TG_BLOOD_IN | 血液檢測水平 | 2 | - | Meter |
| 504-519 | TG_Strip_Lot | 試片批號(16字節) | 16 | ASCII碼 | Meter |

#### 8.2 測量範圍參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 520 | TG_L | 三酸甘油脂值下限 | 1 | 顯示"LO"如果低於設定值 | Meter |
| 521 | TG_H | 三酸甘油脂值上限 | 1 | 顯示"HI"如果高於設定值 | Meter |
| 522-523 | TG_T3_E37 | T3 ADV錯誤37閾值 | 2 | 如果T3 ADV>T3_E37，顯示E37 | Meter |

#### 8.3 測試時序參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 524-525 | TG_TPL_1 | 時間脈衝低 (第一組) | 2 | 單位:100ms | Meter |
| 526-527 | TG_TRD_1 | 原始數據時間 (第一組) | 2 | 單位:100ms | Meter |
| 528-529 | TG_EV_Width_1 | 燃燒時間 (第一組) | 2 | 單位:100ms | Meter |
| 530-531 | TG_TPL_2 | 時間脈衝低 (第二組) | 2 | 單位:100ms | Meter |
| 532-533 | TG_TRD_2 | 原始數據時間 (第二組) | 2 | 單位:100ms | Meter |
| 534-535 | TG_EV_Width_2 | 燃燒時間 (第二組) | 2 | 單位:100ms | Meter |

#### 8.4 品管液(QC)參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 536 | TG_Sq | QC補償參數Sq | 1 | 計算參數 | Meter |
| 537 | TG_Iq | QC補償參數Iq | 1 | 計算參數 | Meter |
| 538 | TG_CVq | QC水平的CV | 1 | 0~100% (100=關閉QC優化) | Meter |
| 539 | TG_Aq | QC補償參數A | 1 | 0~10 | Meter |
| 540 | TG_Bq | QC補償參數B | 1 | 0~10 | Meter |
| 541-570 | TG_QC1-QC5 | QC標準範圍參數 | 30 | QC5為最優先 | Meter |

#### 8.5 計算式補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 571 | TG_S2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 572 | TG_I2 | 試片補償參數 | 1 | 計算參數 | Meter |
| 573-576 | TG_Sr | OPA硬體迴路補償(斜率) | 4 | 浮點數 | Meter |
| 577-580 | TG_Ir | OPA硬體迴路補償(截距) | 4 | 浮點數 | Meter |
| 581-595 | TG_S3 | 試片補償參數S3(1-15) | 15 | 計算參數 | Meter |
| 596-610 | TG_I3 | 試片補償參數I3(1-15) | 15 | 計算參數 | Meter |

#### 8.6 溫度補償參數
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 611-617 | TG_TF | AC/PC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 618-624 | TG_CTF | QC溫度補償斜率(10-40°C) | 7 | 計算參數 | Meter |
| 625-631 | TG_TO | AC/PC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |
| 632-638 | TG_CTO | QC溫度補償截距(10-40°C) | 7 | 計算參數 | Meter |

### 9. 保留與校驗區
| 地址 | 參數名稱 | 描述 | 大小(Bytes) | 範圍/單位 | 使用者 |
|------|----------|------|-------------|-----------|--------|
| 639-674 | Reserved | 保留區域供未來擴展 | 36 | - | - |
| 675 | SUM_L | 地址0~674校驗和(低) | 1 | - | Meter |
| 676 | SUM_H | 地址0~674校驗和(高) | 1 | - | Meter |
| 677 | CRC16 | 完整參數表CRC校驗 | 1 | - | Meter |

## CH32V203 Flash 參數儲存方案

### 硬體特性
CH32V203G8R6 微控制器具有以下 Flash 記憶體特性：
- 總容量：64KB (0x10000 位元組)
- 頁大小：1KB (0x400 位元組)
- 寫入單位：半字 (2 位元組)
- 最小擦除單位：1 頁 (1KB)
- 擦寫壽命：約 10,000 次

### 記憶體分配

| 區域名稱 | 地址範圍 | 大小 | 用途 |
|---------|---------|------|------|
| 主程式區 | 0x08000000 - 0x0800D7FF | 54KB | 系統韌體和主程式 |
| 參數區 A | 0x0800D800 - 0x0800DFFF | 2KB | 參數表主要儲存區域 |
| 參數區 B | 0x0800E000 - 0x0800E7FF | 2KB | 參數表備份區域 |
| 測試記錄區 | 0x0800E800 - 0x0800FFFF | 6KB | 最近測試記錄儲存 |

### 參數表區域細分

根據重組後的參數代碼表，參數表區域細分為以下功能區塊：

| 功能區塊 | 參數範圍 | 大小 | 更新頻率 | 描述 |
|---------|---------|------|---------|------|
| 基本系統區 | 0-19 | 20 bytes | 高 | 系統基本參數、時間、環境等 |
| 硬體校準區 | 20-38 | 19 bytes | 低 | 電極電壓、硬體校準、溫度/電池偏移等 |
| 血糖(GLV/GAV)參數區 | 39-212 | 174 bytes | 低 | 血糖相關所有測量參數 |
| 尿酸(U)參數區 | 213-354 | 142 bytes | 低 | 尿酸相關所有測量參數 |
| 總膽固醇(C)參數區 | 355-496 | 142 bytes | 低 | 膽固醇相關所有測量參數 |
| 三酸甘油脂(TG)參數區 | 497-638 | 142 bytes | 低 | 三酸甘油脂相關所有測量參數 |
| 保留與校驗區 | 639-677 | 39 bytes | 高 | 保留區域和校驗參數 |

### 結構重組的優勢

此次參數結構的重組帶來以下優勢：

1. **測試項目隔離**：
   - 每種測試項目(血糖、尿酸、總膽固醇、三酸甘油脂)都有專屬的完整參數區塊
   - 各測試項目的參數完全獨立，防止交叉干擾
   - 便於針對不同測試類型進行參數優化

2. **參數統一性提高**：
   - 所有測試項目採用統一的參數命名規則
   - 每種測試項目都有標準化的參數結構
   - 參數定義更加清晰明確

3. **維護與擴展性提高**：
   - 新增測試項目時，可以按照已有模式添加新的參數區塊
   - 各測試項目可以獨立演進，不影響其他測試項目
   - 代碼組織更加清晰，減少維護難度

4. **測試時序控制更加靈活**：
   - 每個測試項目都擁有兩組時序參數設定
   - 根據不同測試階段可選擇適合的時序參數
   - 支援更複雜的測試流程設計和優化

5. **參數存取更加直觀**：
   - 根據測試類型直接訪問對應參數區塊
   - 減少條件判斷和複雜的參數索引計算
   - 降低錯誤使用參數的風險

6. **Flash使用效率提高**：
   - 參數按功能分組，支持區塊級更新
   - 減少高頻更新參數對其他參數的影響
   - 降低Flash擦寫次數，延長Flash壽命

7. **適應不同試片特性**：
   - 每種測試類型可以有不同的試片檢測參數
   - 不同試片的補償方式可以獨立設計
   - 測量範圍和品管液參數可以根據不同生化指標特性設定

8. **與現有系統兼容**：
   - 總參數表大小控制在2KB以內，仍可使用原有的雙參數區備份機制
   - 主要參數ID保持不變，減少遷移工作量
   - 保留足夠的擴展空間，確保未來可擴展性

9. **參數類型更加合理**：
   - 將原本分散的1Byte參數合併為2Bytes，提高精度
   - 統一參數類型，使參數定義更加清晰
   - 參數分組更加合理，相關參數集中存儲

10. **試片專用化設計**：
    - 各種測試項目擁有各自的試片特性參數
    - 試片批號與測試類型直接關聯
    - 簡化試片類型切換處理邏輯### 參數使用範例

#### 1. 根據試片類型設定測試參數

```c
// 根據試片類型設定測試參數
bool SetupTestByStripType(void) {
    // 從系統參數區讀取當前測試項目類型
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    uint8_t stripType = basicParams.stripType;
    
    // 讀取對應的試片參數
    uint16_t ndl, udl, bloodIn;
    if (!PARAM_GetStripParametersByStripType(stripType, &ndl, &udl, &bloodIn)) {
        return false;
    }
    
    // 設定檢測閾值
    g_testParams.newDetect = ndl;
    g_testParams.usedDetect = udl;
    g_testParams.bloodIn = bloodIn;
    
    // 讀取測量範圍
    uint8_t lowLimit, highLimit;
    if (!PARAM_GetMeasurementRangeByStripType(stripType, &lowLimit, &highLimit)) {
        return false;
    }
    
    // 設定測量範圍參數
    g_testParams.lowLimit = lowLimit;
    g_testParams.highLimit = highLimit;
    
    // 讀取測試時序 (第一組)
    uint16_t tpl1, trd1, evWidth1;
    if (!PARAM_GetTimingParametersByStripType(stripType, &tpl1, &trd1, &evWidth1, 1)) {
        return false;
    }
    
    // 讀取測試時序 (第二組)
    uint16_t tpl2, trd2, evWidth2;
    if (!PARAM_GetTimingParametersByStripType(stripType, &tpl2, &trd2, &evWidth2, 2)) {
        return false;
    }
    
    // 設定第一組時序參數
    g_testParams.tpl1 = tpl1;
    g_testParams.trd1 = trd1;
    g_testParams.evWidth1 = evWidth1;
    
    // 設定第二組時序參數
    g_testParams.tpl2 = tpl2;
    g_testParams.trd2 = trd2;
    g_testParams.evWidth2 = evWidth2;
    
    // 根據測試類型設定補償參數
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
            return SetupGlucoseCompensation();
        case 1: // U(尿酸)
            return SetupUricAcidCompensation();
        case 2: // C(總膽固醇)
            return SetupCholesterolCompensation();
        case 3: // TG(三酸甘油脂)
            return SetupTriglycerideCompensation();
        default:
            return false;
    }
}
```

#### 2. 時間同步處理範例

```c
// APP向裝置同步時間的處理函數
bool HandleTimeSyncCommand(const uint8_t *data, uint8_t length) {
    // 檢查資料長度
    if (length != 7) {
        return false;
    }
    
    // 解析時間資料
    uint8_t year = data[0];
    uint8_t month = data[1];
    uint8_t date = data[2];
    uint8_t hour = data[3];
    uint8_t minute = data[4];
    uint8_t second = data[5];
    
    // 檢查時間參數有效性
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return false;
    }
    
    // 更新系統參數表中的時間
    bool result = PARAM_UpdateDateTime(year, month, date, hour, minute, second);
    
    // 同時設置系統RTC時間
    if (result) {
        RTC_TimeTypeDef rtcTime;
        RTC_DateTypeDef rtcDate;
        
        rtcTime.Hours = hour;
        rtcTime.Minutes = minute;
        rtcTime.Seconds = second;
        
        rtcDate.Year = year;
        rtcDate.Month = month;
        rtcDate.Date = date;
        rtcDate.WeekDay = 1; // 暫時不計算星期
        
        HAL_RTC_SetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN);
    }
    
    return result;
}
```

#### 3. 裝置狀態讀取範例

```c
// 讀取裝置狀態的函數
bool GetDeviceStatus(uint16_t *testType, uint16_t *stripStatus, uint16_t *batteryVoltage, uint16_t *temperature) {
    // 讀取試片類型
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 讀取硬體校準參數
    HardwareCalibBlock calibParams;
    if (!PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        return false;
    }
    
    // 檢查試片狀態
    uint16_t status = 0;
    
    // 根據不同測試類型讀取對應試片參數
    uint16_t ndl, udl, bloodIn;
    if (!PARAM_GetStripParametersByStripType(basicParams.stripType, &ndl, &udl, &bloodIn)) {
        return false;
    }
    
    // 檢查試片狀態
    if (!CheckStripStatus(basicParams.stripType, ndl, udl, &status)) {
        return false;
    }
    
    // 讀取電池電壓
    uint16_t adcValue = 0;
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        adcValue = HAL_ADC_GetValue(&hadc1);
    }
    
    // 將ADC值轉換為電壓值 (單位: mV)
    uint16_t voltage = (uint16_t)((adcValue * 3300UL / 4095UL) * 2); // 假設有2倍分壓
    
    // 應用電池校準偏移
    voltage += calibParams.batteryOffset * 10; // 假設偏移單位為10mV
    
    // 讀取溫度
    float tempValue = ReadTemperatureSensor();
    
    // 應用溫度校準偏移
    tempValue += calibParams.tempOffset * 0.1f; // 假設偏移單位為0.1°C
    
    // 將溫度轉換為整數 (單位: 0.1°C)
    uint16_t temp = (uint16_t)(tempValue * 10);
    
    // 返回結果
    if (testType) {
        *testType = basicParams.stripType;
    }
    
    if (stripStatus) {
        *stripStatus = status;
    }
    
    if (batteryVoltage) {
        *batteryVoltage = voltage;
    }
    
    if (temperature) {
        *temperature = temp;
    }
    
    return true;
}
```

#### 4. 測試結果儲存範例

```c
// 儲存測試結果的函數
bool SaveTestResult(uint16_t resultValue, uint16_t resultStatus, uint16_t event, uint8_t code,
                   uint16_t batteryVoltage, uint16_t temperature) {
    // 測試記錄結構
    typedef struct {
        uint16_t resultStatus;    // 測試結果狀態 (0=成功，其他=錯誤碼)
        uint16_t resultValue;     // 測試結果值
        uint16_t testType;        // 測試項目類型
        uint16_t event;           // 事件類型
        uint8_t code;             // 試片校正碼
        uint16_t year;            // 年
        uint16_t month;           // 月
        uint16_t date;            // 日
        uint16_t hour;            // 時
        uint16_t minute;          // 分
        uint16_t second;          // 秒
        uint16_t batteryVoltage;  // 電池電壓
        uint16_t temperature;     // 環境溫度
    } TestRecord;
    
    // 讀取當前時間
    uint8_t year, month, date, hour, minute, second;
    if (!PARAM_GetDateTime(&year, &month, &date, &hour, &minute, &second)) {
        return false;
    }
    
    // 讀取當前測試類型
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 填充測試記錄
    TestRecord record;
    record.resultStatus = resultStatus;
    record.resultValue = resultValue;
    record.testType = basicParams.stripType;
    record.event = event;
    record.code = code;
    record.year = 2000 + year;
    record.month = month;
    record.date = date;
    record.hour = hour;
    record.minute = minute;
    record.second = second;
    record.batteryVoltage = batteryVoltage;
    record.temperature = temperature;
    
    // 查找測試記錄儲存位置
    uint32_t recordBase = 0x0800E800; // 測試記錄區起始地址
    uint32_t recordSize = sizeof(TestRecord);
    uint32_t maxRecords = 6 * 1024 / recordSize; // 6KB 記錄區可儲存的記錄數
    
    // 查找下一個可用的記錄位置
    uint32_t addr = recordBase;
    uint32_t endAddr = recordBase + maxRecords * recordSize;
    uint16_t checkFlag;
    
    while (addr < endAddr) {
        memcpy(&checkFlag, (void *)addr, sizeof(uint16_t));
        if (checkFlag == 0xFFFF) { // 假設空記錄的標誌為0xFFFF
            break;
        }
        addr += recordSize;
    }
    
    // 如果記錄區已滿，則擦除整個記錄區並從頭開始
    if (addr >= endAddr) {
        HAL_FLASH_Unlock();
        
        for (uint32_t pageAddr = recordBase; pageAddr < endAddr; pageAddr += 1024) {
            FLASH_ErasePage(pageAddr);
        }
        
        addr = recordBase; // 回到記錄區起始地址
        
        HAL_FLASH_Lock();
    }
    
    // 寫入新記錄
    HAL_FLASH_Unlock();
    FLASH_WriteBuffer(addr, (uint8_t *)&record, recordSize);
    HAL_FLASH_Lock();
    
    // 增加測試次數計數
    PARAM_IncreaseTestCount();
    
    return true;
}
```

#### 5. 硬體校準處理範例

```c
// 處理溫度校準的函數
bool HandleTemperatureCalibration(float referenceTemp) {
    // 讀取當前溫度
    float currentTemp = ReadTemperatureSensor();
    
    // 計算偏移值
    float diff = referenceTemp - currentTemp;
    
    // 檢查偏移是否在合理範圍內 (-12.8°C ~ 12.7°C)
    if (diff < -12.8f || diff > 12.7f) {
        return false;
    }
    
    // 將偏移值轉換為整數 (單位: 0.1°C)
    int8_t tempOffset = (int8_t)(diff * 10);
    
    // 讀取當前硬體校準參數
    HardwareCalibBlock calibParams;
    if (!PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        return false;
    }
    
    // 更新溫度偏移參數
    calibParams.tempOffset = tempOffset;
    
    // 寫回參數表
    return PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock));
}

// 處理電池電壓校準的函數
bool HandleBatteryCalibration(uint16_t referenceVoltage) {
    // 讀取ADC值
    uint16_t adcValue = 0;
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        adcValue = HAL_ADC_GetValue(&hadc1);
    }
    
    // 將ADC值轉換為電壓值 (單位: mV)
    uint16_t currentVoltage = (uint16_t)((adcValue * 3300UL / 4095UL) * 2); // 假設有2倍分壓
    
    // 計算偏移值
    int16_t diff = referenceVoltage - currentVoltage;
    
    // 檢查偏移是否在合理範圍內 (-1.28V ~ 1.27V)
    if (diff < -1280 || diff > 1270) {
        return false;
    }
    
    // 將偏移值轉換為整數 (單位: 10mV)
    int8_t batteryOffset = (int8_t)(diff / 10);
    
    // 讀取當前硬體校準參數
    HardwareCalibBlock calibParams;
    if (!PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        return false;
    }
    
    // 更新電池偏移參數
    calibParams.batteryOffset = batteryOffset;
    
    // 寫回參數表
    return PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock));
}
```

#### 6. 參數表重設功能範例

```c
// 重設測試參數表為出廠設定
bool ResetParametersToDefault(void) {
    // 初始化默認參數區塊
    ParameterBlock defaultBlock;
    
    // 初始化頭部
    defaultBlock.header.magic = 0xA55A;
    defaultBlock.header.version = 1;
    defaultBlock.header.writeCounter = 1;
    defaultBlock.header.timestamp = 0;
    
    // 基本系統參數
    defaultBlock.params.basicSystem.lbt = 58;  // 2.58V
    defaultBlock.params.basicSystem.obt = 55;  // 2.55V
    defaultBlock.params.basicSystem.factory = 0; // 使用者模式
    defaultBlock.params.basicSystem.modelNo = 1;
    defaultBlock.params.basicSystem.fwNo = 10; // 版本1.0
    defaultBlock.params.basicSystem.testCount = 0;
    defaultBlock.params.basicSystem.codeTableVer = 1;
    
    // 設定默認時間 (2025-01-01 00:00:00)
    defaultBlock.params.basicSystem.year = 25;
    defaultBlock.params.basicSystem.month = 1;
    defaultBlock.params.basicSystem.date = 1;
    defaultBlock.params.basicSystem.hour = 0;
    defaultBlock.params.basicSystem.minute = 0;
    defaultBlock.params.basicSystem.second = 0;
    
    // 設定環境參數
    defaultBlock.params.basicSystem.tempLowLimit = 10;  // 10°C
    defaultBlock.params.basicSystem.tempHighLimit = 40; // 40°C
    defaultBlock.params.basicSystem.measureUnit = 1;    // mg/dL
    defaultBlock.params.basicSystem.defaultEvent = 1;   // AC
    defaultBlock.params.basicSystem.stripType = 0;      // GLV(血糖)
    
    // 硬體校準參數
    defaultBlock.params.hardwareCalib.evT3Trigger = 800;  // 800mV
    defaultBlock.params.hardwareCalib.evWorking = 164;
    defaultBlock.params.hardwareCalib.evT3 = 164;
    defaultBlock.params.hardwareCalib.dacoOffset = 0;
    defaultBlock.params.hardwareCalib.dacdo = 0;
    defaultBlock.params.hardwareCalib.cc211Status = 0;
    defaultBlock.params.hardwareCalib.calTolerance = 8;  // 0.8%
    defaultBlock.params.hardwareCalib.ops = 1.0f;       // 預設校準斜率為1.0
    defaultBlock.params.hardwareCalib.opi = 0.0f;       // 預設校準截距為0.0
    defaultBlock.params.hardwareCalib.qct = 0;
    defaultBlock.params.hardwareCalib.tempOffset = 0;   // 溫度偏移補償
    defaultBlock.params.hardwareCalib.batteryOffset = 0; // 電池偏移補償
    
    // 初始化各測試類型參數 (這裡只展示血糖部分，其他類似)
    // 血糖參數
    defaultBlock.params.bgParams.bgCsuTolerance = 10;  // 3%容差
    defaultBlock.params.bgParams.bgNdl = 100;  // 新試片檢測水平
    defaultBlock.params.bgParams.bgUdl = 50;   // 已使用試片檢測水平
    defaultBlock.params.bgParams.bgBloodIn = 150; // 血液檢測水平
    memset(defaultBlock.params.bgParams.bgStripLot, 0, 16); // 清空試片批號
    
    defaultBlock.params.bgParams.bgL = 2;    // 低於20 mg/dL顯示"LO"
    defaultBlock.params.bgParams.bgH = 60;   // 高於600 mg/dL顯示"HI"
    defaultBlock.params.bgParams.bgT3E37 = 2000;  // T3 ADV錯誤閾值
    
    // 血糖測試時序參數
    defaultBlock.params.bgParams.bgTPL1 = 10;        // 1秒
    defaultBlock.params.bgParams.bgTRD1 = 10;        // 1秒
    defaultBlock.params.bgParams.bgEVWidth1 = 5;     // 0.5秒
    defaultBlock.params.bgParams.bgTPL2 = 20;        // 2秒
    defaultBlock.params.bgParams.bgTRD2 = 20;        // 2秒
    defaultBlock.params.bgParams.bgEVWidth2 = 10;    // 1秒
    
    // ... 其他參數初始化 ...
    
    // 計算校驗和
    UpdateChecksum(&defaultBlock);
    
    // 寫入兩個參數區
    HAL_FLASH_Unlock();
    
    // 擦除參數區A
    FLASH_ErasePage(0x0800D800);
    bool resultA = FLASH_WriteBuffer(0x0800D800, (uint8_t *)&defaultBlock, sizeof(ParameterBlock));
    
    // 擦除參數區B
    FLASH_ErasePage(0x0800E000);
    bool resultB = FLASH_WriteBuffer(0x0800E000, (uint8_t *)&defaultBlock, sizeof(ParameterBlock));
    
    HAL_FLASH_Lock();
    
    // 更新活動參數區地址
    if (resultA) {
        g_activeBlockAddr = 0x0800D800;
    } else if (resultB) {
        g_activeBlockAddr = 0x0800E000;
    } else {
        return false;
    }
    
    return (resultA || resultB);
}
```#### 試片參數專用函數

```c
// 根據當前測試類型獲取試片參數
bool PARAM_GetStripParametersByStripType(uint8_t stripType, uint16_t *ndl, uint16_t *udl, uint16_t *bloodIn) {
    if (!ndl || !udl || !bloodIn) {
        return false;
    }
    
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            BGParameterBlock bgParams;
            if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock))) {
                *ndl = bgParams.bgNdl;
                *udl = bgParams.bgUdl;
                *bloodIn = bgParams.bgBloodIn;
                return true;
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UParameterBlock uParams;
            if (PARAM_ReadBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock))) {
                *ndl = uParams.uNdl;
                *udl = uParams.uUdl;
                *bloodIn = uParams.uBloodIn;
                return true;
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CParameterBlock cParams;
            if (PARAM_ReadBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock))) {
                *ndl = cParams.cNdl;
                *udl = cParams.cUdl;
                *bloodIn = cParams.cBloodIn;
                return true;
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TGParameterBlock tgParams;
            if (PARAM_ReadBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock))) {
                *ndl = tgParams.tgNdl;
                *udl = tgParams.tgUdl;
                *bloodIn = tgParams.tgBloodIn;
                return true;
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}

// 獲取試片批號
bool PARAM_GetStripLotByStripType(uint8_t stripType, char *lotBuffer, size_t bufferSize) {
    if (!lotBuffer || bufferSize < 16) {
        return false;
    }
    
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            BGParameterBlock bgParams;
            if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock))) {
                memcpy(lotBuffer, bgParams.bgStripLot, 16);
                return true;
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UParameterBlock uParams;
            if (PARAM_ReadBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock))) {
                memcpy(lotBuffer, uParams.uStripLot, 16);
                return true;
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CParameterBlock cParams;
            if (PARAM_ReadBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock))) {
                memcpy(lotBuffer, cParams.cStripLot, 16);
                return true;
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TGParameterBlock tgParams;
            if (PARAM_ReadBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock))) {
                memcpy(lotBuffer, tgParams.tgStripLot, 16);
                return true;
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}

// 更新試片參數
bool PARAM_UpdateStripParameters(uint8_t stripType, uint16_t ndl, uint16_t udl, uint16_t bloodIn) {
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            BGParameterBlock bgParams;
            if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock))) {
                bgParams.bgNdl = ndl;
                bgParams.bgUdl = udl;
                bgParams.bgBloodIn = bloodIn;
                return PARAM_UpdateBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock));
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UParameterBlock uParams;
            if (PARAM_ReadBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock))) {
                uParams.uNdl = ndl;
                uParams.uUdl = udl;
                uParams.uBloodIn = bloodIn;
                return PARAM_UpdateBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock));
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CParameterBlock cParams;
            if (PARAM_ReadBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock))) {
                cParams.cNdl = ndl;
                cParams.cUdl = udl;
                cParams.cBloodIn = bloodIn;
                return PARAM_UpdateBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock));
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TGParameterBlock tgParams;
            if (PARAM_ReadBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock))) {
                tgParams.tgNdl = ndl;
                tgParams.tgUdl = udl;
                tgParams.tgBloodIn = bloodIn;
                return PARAM_UpdateBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock));
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}

// 更新試片批號
bool PARAM_UpdateStripLot(uint8_t stripType, const char *lotNumber, size_t length) {
    if (!lotNumber || length > 16) {
        return false;
    }
    
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            BGParameterBlock bgParams;
            if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock))) {
                // 清空舊批號
                memset(bgParams.bgStripLot, 0, 16);
                // 複製新批號
                memcpy(bgParams.bgStripLot, lotNumber, length);
                return PARAM_UpdateBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock));
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UParameterBlock uParams;
            if (PARAM_ReadBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock))) {
                // 清空舊批號
                memset(uParams.uStripLot, 0, 16);
                // 複製新批號
                memcpy(uParams.uStripLot, lotNumber, length);
                return PARAM_UpdateBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock));
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CParameterBlock cParams;
            if (PARAM_ReadBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock))) {
                // 清空舊批號
                memset(cParams.cStripLot, 0, 16);
                // 複製新批號
                memcpy(cParams.cStripLot, lotNumber, length);
                return PARAM_UpdateBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock));
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TGParameterBlock tgParams;
            if (PARAM_ReadBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock))) {
                // 清空舊批號
                memset(tgParams.tgStripLot, 0, 16);
                // 複製新批號
                memcpy(tgParams.tgStripLot, lotNumber, length);
                return PARAM_UpdateBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock));
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}
```

#### 測量範圍參數函數

```c
// 根據當前測試類型獲取相應的測量範圍參數
bool PARAM_GetMeasurementRangeByStripType(uint8_t stripType, uint8_t *lowLimit, uint8_t *highLimit) {
    if (!lowLimit || !highLimit) {
        return false;
    }
    
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            BGParameterBlock bgParams;
            if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock))) {
                *lowLimit = bgParams.bgL;
                *highLimit = bgParams.bgH;
                return true;
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UParameterBlock uParams;
            if (PARAM_ReadBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock))) {
                *lowLimit = uParams.uL;
                *highLimit = uParams.uH;
                return true;
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CParameterBlock cParams;
            if (PARAM_ReadBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock))) {
                *lowLimit = cParams.cL;
                *highLimit = cParams.cH;
                return true;
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TGParameterBlock tgParams;
            if (PARAM_ReadBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock))) {
                *lowLimit = tgParams.tgL;
                *highLimit = tgParams.tgH;
                return true;
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}
```

#### 測試時序參數函數

```c
// 根據當前測試類型獲取相應的測試時序參數
bool PARAM_GetTimingParametersByStripType(uint8_t stripType, uint16_t *tpl, uint16_t *trd, uint16_t *evWidth, uint8_t group) {
    if (!tpl || !trd || !evWidth) {
        return false;
    }
    
    switch (stripType) {
        case 0: // GLV(血糖)
        case 4: // GAV(血糖)
        {
            BGParameterBlock bgParams;
            if (PARAM_ReadBlock(BLOCK_BG_PARAMS, &bgParams, sizeof(BGParameterBlock))) {
                if (group == 1) {  // 第一組時序
                    *tpl = bgParams.bgTPL1;
                    *trd = bgParams.bgTRD1;
                    *evWidth = bgParams.bgEVWidth1;
                } else {  // 第二組時序
                    *tpl = bgParams.bgTPL2;
                    *trd = bgParams.bgTRD2;
                    *evWidth = bgParams.bgEVWidth2;
                }
                return true;
            }
            break;
        }
        case 1: // U(尿酸)
        {
            UParameterBlock uParams;
            if (PARAM_ReadBlock(BLOCK_U_PARAMS, &uParams, sizeof(UParameterBlock))) {
                if (group == 1) {  // 第一組時序
                    *tpl = uParams.uTPL1;
                    *trd = uParams.uTRD1;
                    *evWidth = uParams.uEVWidth1;
                } else {  // 第二組時序
                    *tpl = uParams.uTPL2;
                    *trd = uParams.uTRD2;
                    *evWidth = uParams.uEVWidth2;
                }
                return true;
            }
            break;
        }
        case 2: // C(總膽固醇)
        {
            CParameterBlock cParams;
            if (PARAM_ReadBlock(BLOCK_C_PARAMS, &cParams, sizeof(CParameterBlock))) {
                if (group == 1) {  // 第一組時序
                    *tpl = cParams.cTPL1;
                    *trd = cParams.cTRD1;
                    *evWidth = cParams.cEVWidth1;
                } else {  // 第二組時序
                    *tpl = cParams.cTPL2;
                    *trd = cParams.cTRD2;
                    *evWidth = cParams.cEVWidth2;
                }
                return true;
            }
            break;
        }
        case 3: // TG(三酸甘油脂)
        {
            TGParameterBlock tgParams;
            if (PARAM_ReadBlock(BLOCK_TG_PARAMS, &tgParams, sizeof(TGParameterBlock))) {
                if (group == 1) {  // 第一組時序
                    *tpl = tgParams.tgTPL1;
                    *trd = tgParams.tgTRD1;
                    *evWidth = tgParams.tgEVWidth1;
                } else {  // 第二組時序
                    *tpl = tgParams.tgTPL2;
                    *trd = tgParams.tgTRD2;
                    *evWidth = tgParams.tgEVWidth2;
                }
                return true;
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}
```#### Flash 讀寫底層函數

```c
// Flash 擦除頁函數
static HAL_StatusTypeDef FLASH_ErasePage(uint32_t PageAddress) {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = PageAddress;
    EraseInitStruct.NbPages = 1;
    
    return HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
}

// Flash 寫入半字函數
static HAL_StatusTypeDef FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data) {
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, Data);
}

// Flash 寫入緩衝區函數 (將數據緩衝區寫入Flash指定地址)
static HAL_StatusTypeDef FLASH_WriteBuffer(uint32_t Address, const uint8_t *Buffer, uint32_t Length) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t i;
    uint16_t halfword;
    
    // 長度必須為偶數
    if (Length % 2 != 0) {
        return HAL_ERROR;
    }
    
    // 地址必須為半字對齊
    if (Address % 2 != 0) {
        return HAL_ERROR;
    }
    
    // 逐半字寫入
    for (i = 0; i < Length; i += 2) {
        halfword = (((uint16_t)Buffer[i+1]) << 8) | Buffer[i];
        status = FLASH_ProgramHalfWord(Address + i, halfword);
        
        if (status != HAL_OK) {
            return status;
        }
    }
    
    return HAL_OK;
}
```

#### 參數區塊管理函數

```c
// 全局變數
static uint32_t g_activeBlockAddr = 0;  // 當前活動參數區地址

// 初始化參數管理模塊
bool PARAM_Initialize(void) {
    // 參數區A和B的起始地址
    const uint32_t paramAreaA = 0x0800D800;
    const uint32_t paramAreaB = 0x0800E000;
    
    ParameterHeader headerA, headerB;
    
    // 讀取兩個參數區的頭部信息
    memcpy(&headerA, (void *)paramAreaA, sizeof(ParameterHeader));
    memcpy(&headerB, (void *)paramAreaB, sizeof(ParameterHeader));
    
    // 檢查參數區的有效性並選擇活動區域
    bool isAreaAValid = (headerA.magic == 0xA55A);
    bool isAreaBValid = (headerB.magic == 0xA55A);
    
    if (isAreaAValid && isAreaBValid) {
        // 兩個區域都有效，選擇版本號更高的或者寫入次數更多的
        if (headerA.version > headerB.version ||
           (headerA.version == headerB.version && headerA.writeCounter > headerB.writeCounter)) {
            g_activeBlockAddr = paramAreaA;
        } else {
            g_activeBlockAddr = paramAreaB;
        }
    } else if (isAreaAValid) {
        g_activeBlockAddr = paramAreaA;
    } else if (isAreaBValid) {
        g_activeBlockAddr = paramAreaB;
    } else {
        // 兩個區域都無效，初始化參數區A
        ParameterBlock defaultBlock;
        
        // 初始化頭部
        defaultBlock.header.magic = 0xA55A;
        defaultBlock.header.version = 1;
        defaultBlock.header.writeCounter = 1;
        defaultBlock.header.timestamp = 0;
        
        // 初始化參數為默認值
        InitializeDefaultParameters(&defaultBlock.params);
        
        // 計算校驗和
        UpdateChecksum(&defaultBlock);
        
        // 寫入Flash
        HAL_FLASH_Unlock();
        FLASH_ErasePage(paramAreaA);
        FLASH_WriteBuffer(paramAreaA, (uint8_t *)&defaultBlock, sizeof(ParameterBlock));
        HAL_FLASH_Lock();
        
        g_activeBlockAddr = paramAreaA;
    }
    
    return (g_activeBlockAddr != 0);
}

// 計算並更新校驗和
static void UpdateChecksum(ParameterBlock *block) {
    uint16_t sum = 0;
    uint8_t *ptr = (uint8_t *)&block->params;
    
    // 計算參數區域校驗和 (不包括校驗和字段本身)
    for (uint32_t i = 0; i < sizeof(ParameterTable) - 3; i++) {
        sum += ptr[i];
    }
    
    // 更新校驗和字段
    block->params.reserveAndCsum.checksum = sum;
    
    // 更新CRC16 (此處簡化，實際應使用標準CRC16算法)
    block->params.reserveAndCsum.crc16 = (uint8_t)(sum & 0xFF);
}

// 寫入參數區塊
static bool WriteParameterBlock(const ParameterBlock *block) {
    HAL_StatusTypeDef status;
    
    // 確定目標地址 (交替使用兩個參數區)
    uint32_t targetAddr;
    if (g_activeBlockAddr == 0x0800D800) {
        targetAddr = 0x0800E000;  // 如果當前是區域A，則寫入區域B
    } else {
        targetAddr = 0x0800D800;  // 否則寫入區域A
    }
    
    // 寫入新參數區塊
    HAL_FLASH_Unlock();
    status = FLASH_ErasePage(targetAddr);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    status = FLASH_WriteBuffer(targetAddr, (uint8_t *)block, sizeof(ParameterBlock));
    HAL_FLASH_Lock();
    
    if (status != HAL_OK) {
        return false;
    }
    
    // 更新活動區域地址
    g_activeBlockAddr = targetAddr;
    
    return true;
}
```

#### 通用參數讀取與寫入函數

```c
// 讀取參數區塊
bool PARAM_ReadBlock(BlockType blockType, void *buffer, uint16_t size) {
    if (blockType >= BLOCK_MAX || !buffer || size != BlockInfo[blockType].size) {
        return false;
    }
    
    if (g_activeBlockAddr == 0) {
        // 未初始化參數區，嘗試加載
        if (!PARAM_Initialize()) {
            return false;
        }
    }
    
    uint32_t sourceAddr = g_activeBlockAddr + sizeof(ParameterHeader) + BlockInfo[blockType].offset;
    memcpy(buffer, (void *)sourceAddr, size);
    return true;
}

// 更新參數區塊
bool PARAM_UpdateBlock(BlockType blockType, const void *buffer, uint16_t size) {
    if (blockType >= BLOCK_MAX || !buffer || size != BlockInfo[blockType].size) {
        return false;
    }
    
    if (g_activeBlockAddr == 0) {
        // 未初始化參數區，嘗試加載
        if (!PARAM_Initialize()) {
            return false;
        }
    }
    
    // 讀取當前參數區塊
    ParameterBlock block;
    memcpy(&block, (void *)g_activeBlockAddr, sizeof(ParameterBlock));
    
    // 更新指定區塊
    uint8_t *targetAddr = (uint8_t *)&block.params + BlockInfo[blockType].offset;
    memcpy(targetAddr, buffer, size);
    
    // 更新校驗和
    UpdateChecksum(&block);
    
    // 更新版本號和寫入計數器
    block.header.version++;
    block.header.writeCounter++;
    block.header.timestamp = GetSystemTimestamp();
    
    // 寫入新參數區塊
    return WriteParameterBlock(&block);
}
```

#### 特定參數讀取函數

```c
// 讀取系統版本資訊
bool PARAM_GetVersionInfo(uint8_t *modelNo, uint8_t *fwVersion, uint16_t *tableVersion) {
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    if (modelNo) {
        *modelNo = basicParams.modelNo;
    }
    
    if (fwVersion) {
        *fwVersion = basicParams.fwNo;
    }
    
    if (tableVersion) {
        *tableVersion = basicParams.codeTableVer;
    }
    
    return true;
}

// 讀取當前時間
bool PARAM_GetDateTime(uint8_t *year, uint8_t *month, uint8_t *date, 
                      uint8_t *hour, uint8_t *minute, uint8_t *second) {
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    if (year) {
        *year = basicParams.year;
    }
    
    if (month) {
        *month = basicParams.month;
    }
    
    if (date) {
        *date = basicParams.date;
    }
    
    if (hour) {
        *hour = basicParams.hour;
    }
    
    if (minute) {
        *minute = basicParams.minute;
    }
    
    if (second) {
        *second = basicParams.second;
    }
    
    return true;
}

// 讀取測試項目類型
bool PARAM_GetStripType(uint8_t *stripType) {
    if (!stripType) {
        return false;
    }
    
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    *stripType = basicParams.stripType;
    return true;
}

// 讀取電池閾值
bool PARAM_GetBatteryThresholds(uint8_t *lowBattery, uint8_t *outOfBattery) {
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    if (lowBattery) {
        *lowBattery = basicParams.lbt;
    }
    
    if (outOfBattery) {
        *outOfBattery = basicParams.obt;
    }
    
    return true;
}

// 檢查是否為工廠模式
bool PARAM_IsFactoryMode(void) {
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    return (basicParams.factory == 210);
}
```

#### 參數更新函數

```c
// 更新時間設定
bool PARAM_UpdateDateTime(uint8_t year, uint8_t month, uint8_t date, 
                         uint8_t hour, uint8_t minute, uint8_t second) {
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    // 檢查參數有效性
    if (year > 99 || month < 1 || month > 12 || date < 1 || date > 31 ||
        hour > 23 || minute > 59 || second > 59) {
        return false;
    }
    
    basicParams.year = year;
    basicParams.month = month;
    basicParams.date = date;
    basicParams.hour = hour;
    basicParams.minute = minute;
    basicParams.second = second;
    
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

// 更新測試次數
bool PARAM_IncreaseTestCount(void) {
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    basicParams.testCount++;
    
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

// 更新測試項目類型
bool PARAM_UpdateStripType(uint8_t stripType) {
    // 檢查參數有效性
    if (stripType > 4) {
        return false;
    }
    
    BasicSystemBlock basicParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock))) {
        return false;
    }
    
    basicParams.stripType = stripType;
    
    return PARAM_UpdateBlock(BLOCK_BASIC_SYSTEM, &basicParams, sizeof(BasicSystemBlock));
}

// 更新溫度與電池偏移參數
bool PARAM_UpdateCalibrationOffsets(int8_t tempOffset, int8_t batteryOffset) {
    HardwareCalibBlock calibParams;
    if (!PARAM_ReadBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock))) {
        return false;
    }
    
    calibParams.tempOffset = tempOffset;
    calibParams.batteryOffset = batteryOffset;
    
    return PARAM_UpdateBlock(BLOCK_HARDWARE_CALIB, &calibParams, sizeof(HardwareCalibBlock));
}
```// 總膽固醇(C)專用參數區塊
typedef struct {
    // 試片參數 (23個參數)
    uint8_t cCsuTolerance;   // 試片檢查容差
    uint16_t cNdl;           // 新試片檢測水平
    uint16_t cUdl;           // 已使用試片檢測水平
    uint16_t cBloodIn;       // 血液檢測水平
    char cStripLot[16];      // 試片批號
    
    // 測量範圍參數 (4個參數)
    uint8_t cL;              // 膽固醇值下限
    uint8_t cH;              // 膽固醇值上限
    uint16_t cT3E37;         // T3 ADV錯誤37閾值
    
    // 測試時序參數 (12個參數)
    uint16_t cTPL1;          // 時間脈衝低 (第一組)
    uint16_t cTRD1;          // 原始數據時間 (第一組)
    uint16_t cEVWidth1;      // 燃燒時間 (第一組)
    uint16_t cTPL2;          // 時間脈衝低 (第二組)
    uint16_t cTRD2;          // 原始數據時間 (第二組)
    uint16_t cEVWidth2;      // 燃燒時間 (第二組)
    
    // 品管液參數 (35個參數)
    uint8_t cSq;             // QC補償參數Sq
    uint8_t cIq;             // QC補償參數Iq
    uint8_t cCVq;            // QC水平的CV
    uint8_t cAq;             // QC補償參數A
    uint8_t cBq;             // QC補償參數B
    uint8_t cQC[30];         // QC標準範圍參數
    
    // 計算式補償參數 (36個參數)
    uint8_t cS2;             // 試片補償參數
    uint8_t cI2;             // 試片補償參數
    float cSr;               // OPA硬體迴路補償(斜率)
    float cIr;               // OPA硬體迴路補償(截距)
    uint8_t cS3[15];         // 試片補償參數S3(1-15)
    uint8_t cI3[15];         // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t cTF[7];          // AC/PC溫度補償斜率(10-40°C)
    uint8_t cCTF[7];         // QC溫度補償斜率(10-40°C)
    uint8_t cTO[7];          // AC/PC溫度補償截距(10-40°C)
    uint8_t cCTO[7];         // QC溫度補償截距(10-40°C)
} CParameterBlock;

// 三酸甘油脂(TG)專用參數區塊
typedef struct {
    // 試片參數 (23個參數)
    uint8_t tgCsuTolerance;  // 試片檢查容差
    uint16_t tgNdl;          // 新試片檢測水平
    uint16_t tgUdl;          // 已使用試片檢測水平
    uint16_t tgBloodIn;      // 血液檢測水平
    char tgStripLot[16];     // 試片批號
    
    // 測量範圍參數 (4個參數)
    uint8_t tgL;             // 三酸甘油脂值下限
    uint8_t tgH;             // 三酸甘油脂值上限
    uint16_t tgT3E37;        // T3 ADV錯誤37閾值
    
    // 測試時序參數 (12個參數)
    uint16_t tgTPL1;         // 時間脈衝低 (第一組)
    uint16_t tgTRD1;         // 原始數據時間 (第一組)
    uint16_t tgEVWidth1;     // 燃燒時間 (第一組)
    uint16_t tgTPL2;         // 時間脈衝低 (第二組)
    uint16_t tgTRD2;         // 原始數據時間 (第二組)
    uint16_t tgEVWidth2;     // 燃燒時間 (第二組)
    
    // 品管液參數 (35個參數)
    uint8_t tgSq;            // QC補償參數Sq
    uint8_t tgIq;            // QC補償參數Iq
    uint8_t tgCVq;           // QC水平的CV
    uint8_t tgAq;            // QC補償參數A
    uint8_t tgBq;            // QC補償參數B
    uint8_t tgQC[30];        // QC標準範圍參數
    
    // 計算式補償參數 (36個參數)
    uint8_t tgS2;            // 試片補償參數
    uint8_t tgI2;            // 試片補償參數
    float tgSr;              // OPA硬體迴路補償(斜率)
    float tgIr;              // OPA硬體迴路補償(截距)
    uint8_t tgS3[15];        // 試片補償參數S3(1-15)
    uint8_t tgI3[15];        // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t tgTF[7];         // AC/PC溫度補償斜率(10-40°C)
    uint8_t tgCTF[7];        // QC溫度補償斜率(10-40°C)
    uint8_t tgTO[7];         // AC/PC溫度補償截距(10-40°C)
    uint8_t tgCTO[7];        // QC溫度補償截距(10-40°C)
} TGParameterBlock;

// 保留與校驗區塊
typedef struct {
    uint8_t reserved[36];    // 保留區域供未來擴展
    uint16_t checksum;       // 參數校驗和
    uint8_t crc16;           // CRC16校驗
} ReservedAndChecksumBlock;

// 完整參數表結構
typedef struct {
    BasicSystemBlock        basicSystem;    // 基本系統參數區塊
    HardwareCalibBlock      hardwareCalib;  // 硬體校準參數區塊
    BGParameterBlock        bgParams;       // 血糖專用參數區塊
    UParameterBlock         uParams;        // 尿酸專用參數區塊
    CParameterBlock         cParams;        // 總膽固醇專用參數區塊
    TGParameterBlock        tgParams;       // 三酸甘油脂專用參數區塊
    ReservedAndChecksumBlock reserveAndCsum;// 保留與校驗區塊
} ParameterTable;

// 完整參數區塊
typedef struct {
    ParameterHeader header;  // 參數表頭部
    ParameterTable params;   // 參數表內容
} ParameterBlock;

#pragma pack()  // 恢復默認對齊方式
```

### 參數存取函數設計

#### 參數區塊類型枚舉

```c
// 參數區塊類型
typedef enum {
    BLOCK_BASIC_SYSTEM = 0,  // 基本系統參數區塊
    BLOCK_HARDWARE_CALIB,    // 硬體校準參數區塊
    BLOCK_BG_PARAMS,         // 血糖專用參數區塊
    BLOCK_U_PARAMS,          // 尿酸專用參數區塊
    BLOCK_C_PARAMS,          // 總膽固醇專用參數區塊
    BLOCK_TG_PARAMS,         // 三酸甘油脂專用參數區塊
    BLOCK_RESERVED_CHECKSUM, // 保留與校驗區塊
    BLOCK_MAX
} BlockType;

// 參數區塊偏移量和大小表
const struct {
    uint16_t offset;
    uint16_t size;
} BlockInfo[BLOCK_MAX] = {
    {offsetof(ParameterTable, basicSystem), sizeof(BasicSystemBlock)},
    {offsetof(ParameterTable, hardwareCalib), sizeof(HardwareCalibBlock)},
    {offsetof(ParameterTable, bgParams), sizeof(BGParameterBlock)},
    {offsetof(ParameterTable, uParams), sizeof(UParameterBlock)},
    {offsetof(ParameterTable, cParams), sizeof(CParameterBlock)},
    {offsetof(ParameterTable, tgParams), sizeof(TGParameterBlock)},
    {offsetof(ParameterTable, reserveAndCsum), sizeof(ReservedAndChecksumBlock)},
};### 參數表資料結構設計

根據重組後的參數表，更新後的資料結構如下：

```c
#pragma pack(1)  // 確保結構體緊密封裝

// 參數表頭部結構
typedef struct {
    uint16_t magic;          // 魔術數字 (0xA55A)，表示有效的參數區
    uint16_t version;        // 參數表版本號，對應 Code_Table_V
    uint32_t writeCounter;   // 寫入次數計數器
    uint32_t timestamp;      // 最後更新時間戳記
} ParameterHeader;

// 基本系統參數區塊
typedef struct {
    // 系統基本參數 (9個參數)
    uint8_t lbt;             // 低電池閾值
    uint8_t obt;             // 電池耗盡閾值
    uint8_t factory;         // 儀器操作模式
    uint8_t modelNo;         // 產品型號
    uint8_t fwNo;            // 韌體版本號
    uint16_t testCount;      // 測試次數
    uint16_t codeTableVer;   // 代碼表版本
    
    // 時間設定參數 (6個參數)
    uint8_t year;            // 年份設定 (0-99 for 2000-2099)
    uint8_t month;           // 月份設定 (1-12)
    uint8_t date;            // 日期設定 (1-31)
    uint8_t hour;            // 小時設定 (0-23)
    uint8_t minute;          // 分鐘設定 (0-59)
    uint8_t second;          // 秒數設定 (0-59)
    
    // 測試環境參數 (5個參數)
    uint8_t tempLowLimit;    // 操作溫度下限
    uint8_t tempHighLimit;   // 操作溫度上限
    uint8_t measureUnit;     // 測量單位設定
    uint8_t defaultEvent;    // 預設事件
    uint8_t stripType;       // 測試項目類型
} BasicSystemBlock;

// 硬體校準參數區塊
typedef struct {
    uint16_t evT3Trigger;    // EV_T3觸發電壓
    uint8_t evWorking;       // 測量工作電極電壓
    uint8_t evT3;            // 血液檢測電極電壓
    uint8_t dacoOffset;      // DAC偏移補償
    uint8_t dacdo;           // DAC校正管理參數
    uint8_t cc211Status;     // CC211未完成旗標
    uint8_t calTolerance;    // OPS/OPI校準容差
    float ops;               // OPA校準斜率
    float opi;               // OPA校準截距
    uint8_t qct;             // QCT校準測試低位元組
    int8_t tempOffset;       // 溫度校準偏移
    int8_t batteryOffset;    // 電池校準偏移
} HardwareCalibBlock;

// 血糖(GLV/GAV)專用參數區塊
typedef struct {
    // 試片參數 (23個參數)
    uint8_t bgCsuTolerance;  // 試片檢查容差
    uint16_t bgNdl;          // 新試片檢測水平
    uint16_t bgUdl;          // 已使用試片檢測水平
    uint16_t bgBloodIn;      // 血液檢測水平
    char bgStripLot[16];     // 試片批號
    
    // 測量範圍參數 (4個參數)
    uint8_t bgL;             // 血糖值下限
    uint8_t bgH;             // 血糖值上限
    uint16_t bgT3E37;        // T3 ADV錯誤37閾值
    
    // 測試時序參數 (12個參數)
    uint16_t bgTPL1;         // 時間脈衝低 (第一組)
    uint16_t bgTRD1;         // 原始數據時間 (第一組)
    uint16_t bgEVWidth1;     // 燃燒時間 (第一組)
    uint16_t bgTPL2;         // 時間脈衝低 (第二組)
    uint16_t bgTRD2;         // 原始數據時間 (第二組)
    uint16_t bgEVWidth2;     // 燃燒時間 (第二組)
    
    // 品管液參數 (37個參數)
    uint8_t bgSq;            // QC補償參數Sq
    uint8_t bgIq;            // QC補償參數Iq
    uint8_t bgCVq;           // QC水平的CV
    uint8_t bgAq;            // QC補償參數A
    uint8_t bgBq;            // QC補償參數B
    uint8_t bgAq5;           // QC補償參數A(等級5)
    uint8_t bgBq5;           // QC補償參數B(等級5)
    uint8_t bgQC[30];        // QC標準範圍參數
    
    // 計算式補償參數 (66個參數)
    uint8_t bgS2;            // 試片補償參數
    uint8_t bgI2;            // 試片補償參數
    float bgSr;              // OPA硬體迴路補償(斜率)
    float bgIr;              // OPA硬體迴路補償(截距)
    uint8_t bgS3[30];        // 試片補償參數S3(1-30)
    uint8_t bgI3[30];        // 試片補償參數I3(1-30)
    
    // 溫度補償參數 (28個參數)
    uint8_t bgTF[7];         // AC/PC溫度補償斜率(10-40°C)
    uint8_t bgCTF[7];        // QC溫度補償斜率(10-40°C)
    uint8_t bgTO[7];         // AC/PC溫度補償截距(10-40°C)
    uint8_t bgCTO[7];        // QC溫度補償截距(10-40°C)
} BGParameterBlock;

// 尿酸(U)專用參數區塊
typedef struct {
    // 試片參數 (23個參數)
    uint8_t uCsuTolerance;   // 試片檢查容差
    uint16_t uNdl;           // 新試片檢測水平
    uint16_t uUdl;           // 已使用試片檢測水平
    uint16_t uBloodIn;       // 血液檢測水平
    char uStripLot[16];      // 試片批號
    
    // 測量範圍參數 (4個參數)
    uint8_t uL;              // 尿酸值下限
    uint8_t uH;              // 尿酸值上限
    uint16_t uT3E37;         // T3 ADV錯誤37閾值
    
    // 測試時序參數 (12個參數)
    uint16_t uTPL1;          // 時間脈衝低 (第一組)
    uint16_t uTRD1;          // 原始數據時間 (第一組)
    uint16_t uEVWidth1;      // 燃燒時間 (第一組)
    uint16_t uTPL2;          // 時間脈衝低 (第二組)
    uint16_t uTRD2;          // 原始數據時間 (第二組)
    uint16_t uEVWidth2;      // 燃燒時間 (第二組)
    
    // 品管液參數 (35個參數)
    uint8_t uSq;             // QC補償參數Sq
    uint8_t uIq;             // QC補償參數Iq
    uint8_t uCVq;            // QC水平的CV
    uint8_t uAq;             // QC補償參數A
    uint8_t uBq;             // QC補償參數B
    uint8_t uQC[30];         // QC標準範圍參數
    
    // 計算式補償參數 (36個參數)
    uint8_t uS2;             // 試片補償參數
    uint8_t uI2;             // 試片補償參數
    float uSr;               // OPA硬體迴路補償(斜率)
    float uIr;               // OPA硬體迴路補償(截距)
    uint8_t uS3[15];         // 試片補償參數S3(1-15)
    uint8_t uI3[15];         // 試片補償參數I3(1-15)
    
    // 溫度補償參數 (28個參數)
    uint8_t uTF[7];          // AC/PC溫度補償斜率(10-40°C)
    uint8_t uCTF[7];         // QC溫度補償斜率(10-40°C)
    uint8_t uTO[7];          // AC/PC溫度補償截距(10-40°C)
    uint8_t uCTO[7];         // QC溫度補償截距(10-40°C)
} UParameterBlock;# 多功能生化測試儀參數代碼表與CH32V203 Flash儲存方案


# 多功能生化測試儀參數代碼表程式

## 概述
此程式實現了一個完整的多功能生化測試儀參數代碼表系統，可用於管理、存儲和讀取生化測試儀的各項參數。程式基於CH32V203G8R6微控制器開發，支持多種生化測試項目，包括血糖、尿酸、總膽固醇和三酸甘油脂測試。

## 主要功能
1. **參數表結構設計**：設計完整的參數表結構，包含系統基本參數、時間設定、測試環境參數、硬體校準參數和各測試項目專用參數。
2. **參數初始化**：提供默認參數值的初始化功能。
3. **參數存儲與讀取**：實現將參數表保存到Flash和從Flash讀取的功能。
4. **校驗機制**：通過校驗和和CRC16校驗確保參數的正確性和完整性。
5. **參數信息顯示**：通過UART輸出參數信息，方便調試和查看。

## 檔案結構
- `param_table.h`: 參數表結構定義和函數聲明
- `param_table.c`: 參數表操作函數實現
- `main.c`: 主程式，演示參數表的使用

## 參數表結構
參數表分為以下幾個主要部分：
1. **系統基本參數**：包括低電池閾值、電池耗盡閾值、儀器操作模式、產品型號、韌體版本號、測試次數和代碼表版本編號。
2. **時間設定參數**：年、月、日、時、分、秒。
3. **測試環境參數**：操作溫度範圍、測量單位設定、預設事件類型和測試項目。
4. **硬體校準參數**：包括電極電壓、DAC設定、校準參數等。
5. **測試項目專用參數**：每種測試項目（血糖、尿酸、總膽固醇、三酸甘油脂）都有自己的專用參數，包括試片參數、測量範圍參數、測試時序參數、品管液參數、計算式補償參數和溫度補償參數。
6. **保留與校驗區**：保留區域供未來擴展，以及校驗和與CRC16校驗值。

## 使用方法
1. **初始化參數表**
```c
ParamTable_TypeDef param_table;
ParamTable_Init(&param_table);
```

2. **保存參數表到Flash**
```c
ParamTable_SaveToFlash(&param_table);
```

3. **從Flash加載參數表**
```c
uint8_t result = ParamTable_LoadFromFlash(&param_table);
if(result != 0) {
    // 加載失敗，參數表已初始化為默認值
}
```

4. **修改參數**
```c
param_table.MGDL = UNIT_MG_DL; // 設置測量單位為mg/dL
param_table.Strip_Type = STRIP_TYPE_GLV; // 設置測試項目為血糖
param_table.BG_H = 600; // 設置血糖上限為600 mg/dL
```

5. **查看參數信息**
```c
printParamTableInfo(&param_table); // 打印參數表信息
```

## 硬體連接
- USART1_Tx(PA9)：用於輸出調試信息

## 注意事項
1. 參數表保存在Flash的0x08010000地址處，請確保該區域未被其他程式使用。
2. 參數表大小為678字節，根據實際需求可以調整參數表的結構和大小。
3. 校驗機制採用簡化的校驗和和CRC16校驗，實際應用中可能需要更強的校驗算法。
4. 修改參數後需要調用`ParamTable_SaveToFlash`函數將參數表保存到Flash中，否則修改將不會被保留。
5. 系統重啟後，需要調用`ParamTable_LoadFromFlash`函數從Flash中加載參數表。

## 版本信息
- 版本：V1.0.0
- 日期：2024/09/18
- 作者：BRUCE 
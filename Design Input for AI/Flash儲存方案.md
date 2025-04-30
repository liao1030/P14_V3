## 目錄
1. [CH32V203 Flash 參數儲存方案](#ch32v203-flash-參數儲存方案)
   - [硬體特性](#硬體特性)
   - [記憶體分配](#記憶體分配)
   - [參數表區域細分](#參數表區域細分)
   - [參數表資料結構設計](#參數表資料結構設計)
   - [參數存取函數設計](#參數存取函數設計)
   - [參數使用範例](#參數使用範例)
   - [結構重組的優勢](#結構重組的優勢)

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
```


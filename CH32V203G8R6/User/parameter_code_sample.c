// 引入參數管理模塊
#include "parameter/flash_parameter.h"

// 參數初始化函數範例 - 添加Sample後綴避免命名衝突
void Parameter_Init_Sample(void)
{
    // 初始化參數管理模塊
    PARAM_Init();
    
    // 檢查是否需要重置參數（例如按住特定按鍵）
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_RESET)  // 假設按鍵接到 PB12
    {
        // 重置參數為出廠預設值
        PARAM_ResetToDefault();
    }
}

// 參數讀取範例
void Parameter_ReadSample(void)
{
    // 讀取基本系統參數
    BasicSystemBlock sysParams;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysParams, sizeof(BasicSystemBlock)))
    {
        printf("系統版本: %d.%d\r\n", sysParams.fwNo / 10, sysParams.fwNo % 10);
        printf("測試次數: %lu\r\n", sysParams.testCount);
        printf("當前時間: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
               sysParams.year, sysParams.month, sysParams.date,
               sysParams.hour, sysParams.minute, sysParams.second);
    }
    
    // 讀取當前試片類型的參數
    uint16_t ndl, udl, bloodIn;
    if (PARAM_GetStripParametersByStripType(sysParams.stripType, &ndl, &udl, &bloodIn))
    {
        printf("當前試片類型: %d\r\n", sysParams.stripType);
        printf("新試片檢測水平: %d\r\n", ndl);
        printf("已使用試片檢測水平: %d\r\n", udl);
        printf("血液檢測水平: %d\r\n", bloodIn);
    }
}

// 參數更新範例
void Parameter_UpdateSample(void)
{
    // 更新系統時間
    PARAM_UpdateDateTime(25, 5, 2, 15, 30, 0); // 2025-05-02 15:30:00
    
    // 更新血糖參數
    GlucoseParamBlock bgParams;
    if (PARAM_ReadBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock)))
    {
        // 修改參數
        bgParams.bgNdl = 120;
        bgParams.bgUdl = 60;
        
        // 寫回參數
        PARAM_UpdateBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock));
    }
}

// 測試記錄儲存範例
void SaveTestResultSample(uint16_t resultValue)
{
    // 讀取系統參數
    BasicSystemBlock sysParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysParams, sizeof(BasicSystemBlock)))
    {
        return;
    }
    
    // 準備測試記錄
    TestRecord record;
    
    // 填充測試記錄
    record.resultStatus = 0;  // 成功
    record.resultValue = resultValue;
    record.testType = sysParams.stripType;
    record.event = sysParams.defaultEvent;
    record.code = 1;  // 假設批次碼
    
    // 設置日期時間
    record.year = 2000 + sysParams.year;
    record.month = sysParams.month;
    record.date = sysParams.date;
    record.hour = sysParams.hour;
    record.minute = sysParams.minute;
    record.second = sysParams.second;
    
    // 設置其他資訊
    record.batteryVoltage = 320; // 3.2V，假設值
    record.temperature = 250;    // 25.0°C，假設值
    
    // 儲存測試記錄
    PARAM_SaveTestRecord(&record);
}

// 測試記錄讀取範例
void ReadTestRecordSample(void)
{
    // 獲取測試記錄總數
    uint32_t recordCount = PARAM_GetTestRecordCount();
    printf("測試記錄總數: %lu\r\n", recordCount);
    
    // 讀取最新的測試記錄
    TestRecord record;
    if (PARAM_GetTestRecord(0, &record))
    {
        printf("最新測試結果: %d\r\n", record.resultValue);
        printf("測試類型: %d\r\n", record.testType);
        printf("測試時間: %d-%02d-%02d %02d:%02d:%02d\r\n",
               record.year, record.month, record.date,
               record.hour, record.minute, record.second);
    }
}

// 主程序中的調用範例 - 改名為sample_main避免與main函數衝突
void sample_main(void)
{
    // 系統初始化
    SystemInit();
    
    // 參數初始化
    Parameter_Init_Sample();
    
    // 讀取參數範例
    Parameter_ReadSample();
    
    // 更新參數範例
    Parameter_UpdateSample();
    
    // 測試記錄範例
    SaveTestResultSample(123);
    ReadTestRecordSample();
    
    // 主循環
    while(1)
    {
        // 主程序代碼
    }
}
// ���녢������ģ�K
#include "parameter/flash_parameter.h"

// ������ʼ���������� - ���Sample��Y���������nͻ
void Parameter_Init_Sample(void)
{
    // ��ʼ����������ģ�K
    PARAM_Init();
    
    // �z���Ƿ���Ҫ���Å��������簴ס�ض����I��
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_RESET)  // ���O���I�ӵ� PB12
    {
        // ���Å�������S�A�Oֵ
        PARAM_ResetToDefault();
    }
}

// �����xȡ����
void Parameter_ReadSample(void)
{
    // �xȡ����ϵ�y����
    BasicSystemBlock sysParams;
    if (PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysParams, sizeof(BasicSystemBlock)))
    {
        printf("ϵ�y�汾: %d.%d\r\n", sysParams.fwNo / 10, sysParams.fwNo % 10);
        printf("�yԇ�Δ�: %lu\r\n", sysParams.testCount);
        printf("��ǰ�r�g: 20%02d-%02d-%02d %02d:%02d:%02d\r\n",
               sysParams.year, sysParams.month, sysParams.date,
               sysParams.hour, sysParams.minute, sysParams.second);
    }
    
    // �xȡ��ǰԇƬ��͵ą���
    uint16_t ndl, udl, bloodIn;
    if (PARAM_GetStripParametersByStripType(sysParams.stripType, &ndl, &udl, &bloodIn))
    {
        printf("��ǰԇƬ���: %d\r\n", sysParams.stripType);
        printf("��ԇƬ�z�yˮƽ: %d\r\n", ndl);
        printf("��ʹ��ԇƬ�z�yˮƽ: %d\r\n", udl);
        printf("ѪҺ�z�yˮƽ: %d\r\n", bloodIn);
    }
}

// �������¹���
void Parameter_UpdateSample(void)
{
    // ����ϵ�y�r�g
    PARAM_UpdateDateTime(25, 5, 2, 15, 30, 0); // 2025-05-02 15:30:00
    
    // ����Ѫ�ǅ���
    GlucoseParamBlock bgParams;
    if (PARAM_ReadBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock)))
    {
        // �޸ą���
        bgParams.bgNdl = 120;
        bgParams.bgUdl = 60;
        
        // ���؅���
        PARAM_UpdateBlock(BLOCK_GLUCOSE, &bgParams, sizeof(GlucoseParamBlock));
    }
}

// �yԇӛ䛃��湠��
void SaveTestResultSample(uint16_t resultValue)
{
    // �xȡϵ�y����
    BasicSystemBlock sysParams;
    if (!PARAM_ReadBlock(BLOCK_BASIC_SYSTEM, &sysParams, sizeof(BasicSystemBlock)))
    {
        return;
    }
    
    // �ʂ�yԇӛ�
    TestRecord record;
    
    // ���yԇӛ�
    record.resultStatus = 0;  // �ɹ�
    record.resultValue = resultValue;
    record.testType = sysParams.stripType;
    record.event = sysParams.defaultEvent;
    record.code = 1;  // ���O���δa
    
    // �O�����ڕr�g
    record.year = 2000 + sysParams.year;
    record.month = sysParams.month;
    record.date = sysParams.date;
    record.hour = sysParams.hour;
    record.minute = sysParams.minute;
    record.second = sysParams.second;
    
    // �O�������YӍ
    record.batteryVoltage = 320; // 3.2V�����Oֵ
    record.temperature = 250;    // 25.0��C�����Oֵ
    
    // ����yԇӛ�
    PARAM_SaveTestRecord(&record);
}

// �yԇӛ��xȡ����
void ReadTestRecordSample(void)
{
    // �@ȡ�yԇӛ䛿���
    uint32_t recordCount = PARAM_GetTestRecordCount();
    printf("�yԇӛ䛿���: %lu\r\n", recordCount);
    
    // �xȡ���µĜyԇӛ�
    TestRecord record;
    if (PARAM_GetTestRecord(0, &record))
    {
        printf("���yԇ�Y��: %d\r\n", record.resultValue);
        printf("�yԇ���: %d\r\n", record.testType);
        printf("�yԇ�r�g: %d-%02d-%02d %02d:%02d:%02d\r\n",
               record.year, record.month, record.date,
               record.hour, record.minute, record.second);
    }
}

// �������е��{�ù��� - ������sample_main�����cmain�����nͻ
void sample_main(void)
{
    // ϵ�y��ʼ��
    SystemInit();
    
    // ������ʼ��
    Parameter_Init_Sample();
    
    // �xȡ��������
    Parameter_ReadSample();
    
    // ����������
    Parameter_UpdateSample();
    
    // �yԇӛ䛹���
    SaveTestResultSample(123);
    ReadTestRecordSample();
    
    // ��ѭ�h
    while(1)
    {
        // ��������a
    }
}
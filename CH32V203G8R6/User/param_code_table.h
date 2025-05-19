/********************************** (C) COPYRIGHT  *******************************
 * File Name          : param_code_table.h
 * Author             : HMD Technical Team
 * Version            : V1.0.0
 * Date               : 2025/05/19
 * Description        : �๦�������yԇ�x�������a���^�ļ�
 * Copyright (c) 2025 Healthynamics Biotech Co., Ltd.
*******************************************************************************/

#ifndef __PARAM_CODE_TABLE_H
#define __PARAM_CODE_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ϵ�y�������� */
#define PARAM_LBT                   0    // ��늳��ֵ
#define PARAM_OBT                   1    // 늳غıM�ֵ
#define PARAM_FACTORY               2    // �x������ģʽ
#define PARAM_MODEL_NO              3    // �aƷ��̖
#define PARAM_FW_NO                 4    // �g�w�汾̖
#define PARAM_NOT                   5    // �yԇ�Δ� (2 bytes)
#define PARAM_CODE_TABLE_V          7    // ���a��汾��̖ (2 bytes)

/* �r�g�O������ */
#define PARAM_YEAR                  9    // ����O��
#define PARAM_MONTH                10    // �·��O��
#define PARAM_DATE                 11    // �����O��
#define PARAM_HOUR                 12    // С�r�O��
#define PARAM_MINUTE               13    // ����O��
#define PARAM_SECOND               14    // �딵�O��

/* �yԇ�h������ */
#define PARAM_TLL                  15    // �����ضȹ�������
#define PARAM_TLH                  16    // �����ضȹ�������
#define PARAM_MGDL                 17    // �y����λ�O��
#define PARAM_EVENT                18    // �A�O�¼����
#define PARAM_STRIP_TYPE           19    // �yԇ�Ŀ

/* Ӳ�wУ�ʅ��� */
#define PARAM_EV_T3_TRG            20    // EV_T3�|�l늉� (2 bytes)
#define PARAM_EV_WORKING           22    // �y������늘O늉�
#define PARAM_EV_T3                23    // ѪҺ�z�y늘O늉�
#define PARAM_CAL_TOL              24    // OPS/OPIУ���ݲ�
#define PARAM_OPS                  25    // OPAУ��б�� (4 bytes)
#define PARAM_OPI                  29    // OPAУ�ʽؾ� (4 bytes)
#define PARAM_QCT                  33    // QCTУ�ʜyԇ��λԪ�M
#define PARAM_TOFFSET              34    // �ض�У��ƫ��
#define PARAM_BOFFSET              35    // 늳�У��ƫ��

/* Ѫ��(GLV)���Å��� */
/* ԇƬ���� */
#define PARAM_GLV_CSU_TOL          36    // ԇƬ�z���ݲ�
#define PARAM_GLV_NDL              37    // ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_GLV_UDL              39    // ��ʹ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_GLV_BLOOD_IN         41    // ѪҺ�z�yˮƽ (2 bytes)
#define PARAM_GLV_EV_W_BACKGROUND  43    // W늘O����ֵ (2 bytes)
#define PARAM_GLV_EV_W_PWM_DUTY    45    // W늘OPWMռ�ձ� (2 bytes)
#define PARAM_GLV_COUNT_DOWN_TIME  47    // �����r�g

/* �y���������� */
#define PARAM_GLV_L                48    // Ѫ��ֵ����
#define PARAM_GLV_H                49    // Ѫ��ֵ����

/* �yԇ�r�򅢔� */
#define PARAM_GLV_TPL_1            50    // �r�g�}�n�� (��һ�M) (2 bytes)
#define PARAM_GLV_TRD_1            52    // ԭʼ�����r�g (��һ�M) (2 bytes)
#define PARAM_GLV_EV_WIDTH_1       54    // ȼ���r�g (��һ�M) (2 bytes)
#define PARAM_GLV_TPL_2            56    // �r�g�}�n�� (�ڶ��M) (2 bytes)
#define PARAM_GLV_TRD_2            58    // ԭʼ�����r�g (�ڶ��M) (2 bytes)
#define PARAM_GLV_EV_WIDTH_2       60    // ȼ���r�g (�ڶ��M) (2 bytes)

/* Ӌ��ʽ�a������ */
#define PARAM_GLV_S2               62    // ԇƬ�a������
#define PARAM_GLV_I2               63    // ԇƬ�a������
#define PARAM_GLV_SQ               64    // Ʒ��ҺQC�a������Sq
#define PARAM_GLV_IQ               65    // Ʒ��ҺQC�a������Iq
#define PARAM_GLV_SR               66    // OPAӲ�wޒ·�a��(б��) (4 bytes)
#define PARAM_GLV_IR               70    // OPAӲ�wޒ·�a��(�ؾ�) (4 bytes)
#define PARAM_GLV_GLU0             74    // ���У������(���y�ֶ��c) (14 bytes)
#define PARAM_GLV_GOAL0            88    // ���У������(Ŀ��ֵ�ֶ��c) (14 bytes)
#define PARAM_GLV_S3               102   // ԇƬ�a������S3(1-30) (30 bytes)
#define PARAM_GLV_I3               132   // ԇƬ�a������I3(1-30) (30 bytes)

/* �ض��a������ */
#define PARAM_GLV_TF               162   // AC/PC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_GLV_CTF              169   // QC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_GLV_TO               176   // AC/PC�ض��a���ؾ�(10-40��C) (7 bytes)
#define PARAM_GLV_CTO              183   // QC�ض��a���ؾ�(10-40��C) (7 bytes)

/* Ӌ���პ���a������ */
#define PARAM_GLV_CVQ              190   // CV Level of QC
#define PARAM_GLV_AQ               191   // A of QC Compensation
#define PARAM_GLV_BQ               192   // B of QC Compensation
#define PARAM_GLV_TCV              193   // Time of BG Optimize
#define PARAM_GLV_CVBG             194   // CV Level of BG
#define PARAM_GLV_ABG              195   // A of BG Compensation
#define PARAM_GLV_BBG              196   // B of BG Compensation
#define PARAM_GLV_AQ5              197   // A of QC Compensation Level 5
#define PARAM_GLV_BQ5              198   // B of QC Compensation Level 5

/* ����(U)���Å��� */
/* ԇƬ���� */
#define PARAM_U_CSU_TOL            199   // ԇƬ�z���ݲ�
#define PARAM_U_NDL                200   // ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_U_UDL                202   // ��ʹ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_U_BLOOD_IN           204   // ѪҺ�z�yˮƽ (2 bytes)
#define PARAM_U_EV_W_BACKGROUND    206   // W늘O����ֵ (2 bytes)
#define PARAM_U_EV_W_PWM_DUTY      208   // W늘OPWMռ�ձ� (2 bytes)
#define PARAM_U_COUNT_DOWN_TIME    210   // �����r�g

/* �y���������� */
#define PARAM_U_L                  211   // ����ֵ����
#define PARAM_U_H                  212   // ����ֵ����

/* �yԇ�r�򅢔� */
#define PARAM_U_TPL_1              213   // �r�g�}�n�� (��һ�M) (2 bytes)
#define PARAM_U_TRD_1              215   // ԭʼ�����r�g (��һ�M) (2 bytes)
#define PARAM_U_EV_WIDTH_1         217   // ȼ���r�g (��һ�M) (2 bytes)
#define PARAM_U_TPL_2              219   // �r�g�}�n�� (�ڶ��M) (2 bytes)
#define PARAM_U_TRD_2              221   // ԭʼ�����r�g (�ڶ��M) (2 bytes)
#define PARAM_U_EV_WIDTH_2         223   // ȼ���r�g (�ڶ��M) (2 bytes)

/* Ӌ��ʽ�a������ */
#define PARAM_U_S2                 225   // ԇƬ�a������
#define PARAM_U_I2                 226   // ԇƬ�a������
#define PARAM_U_SQ                 227   // Ʒ��ҺQC�a������Sq
#define PARAM_U_IQ                 228   // Ʒ��ҺQC�a������Iq
#define PARAM_U_SR                 229   // OPAӲ�wޒ·�a��(б��) (4 bytes)
#define PARAM_U_IR                 233   // OPAӲ�wޒ·�a��(�ؾ�) (4 bytes)
#define PARAM_U_GLU0               237   // ���У������(���y�ֶ��c) (14 bytes)
#define PARAM_U_GOAL0              251   // ���У������(Ŀ��ֵ�ֶ��c) (14 bytes)
#define PARAM_U_S3                 265   // ԇƬ�a������S3(1-15) (15 bytes)
#define PARAM_U_I3                 280   // ԇƬ�a������I3(1-15) (15 bytes)

/* �ض��a������ */
#define PARAM_U_TF                 295   // AC/PC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_U_CTF                302   // QC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_U_TO                 309   // AC/PC�ض��a���ؾ�(10-40��C) (7 bytes)
#define PARAM_U_CTO                316   // QC�ض��a���ؾ�(10-40��C) (7 bytes)

/* Ӌ���პ���a������ */
#define PARAM_U_CVQ                323   // CV Level of QC
#define PARAM_U_AQ                 324   // A of QC Compensation
#define PARAM_U_BQ                 325   // B of QC Compensation
#define PARAM_U_TCV                326   // Time of BG Optimize
#define PARAM_U_CVBG               327   // CV Level of BG
#define PARAM_U_ABG                328   // A of BG Compensation
#define PARAM_U_BBG                329   // B of BG Compensation
#define PARAM_U_AQ5                330   // A of QC Compensation Level 5
#define PARAM_U_BQ5                331   // B of QC Compensation Level 5

/* ��đ�̴�(C)���Å��� */
/* ԇƬ���� */
#define PARAM_C_CSU_TOL            332   // ԇƬ�z���ݲ�
#define PARAM_C_NDL                333   // ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_C_UDL                335   // ��ʹ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_C_BLOOD_IN           337   // ѪҺ�z�yˮƽ (2 bytes)
#define PARAM_C_EV_W_BACKGROUND    339   // W늘O����ֵ (2 bytes)
#define PARAM_C_EV_W_PWM_DUTY      341   // W늘OPWMռ�ձ� (2 bytes)
#define PARAM_C_COUNT_DOWN_TIME    343   // �����r�g

/* �y���������� */
#define PARAM_C_L                  344   // đ�̴�ֵ����
#define PARAM_C_H                  345   // đ�̴�ֵ����

/* �yԇ�r�򅢔� */
#define PARAM_C_TPL_1              346   // �r�g�}�n�� (��һ�M) (2 bytes)
#define PARAM_C_TRD_1              348   // ԭʼ�����r�g (��һ�M) (2 bytes)
#define PARAM_C_EV_WIDTH_1         350   // ȼ���r�g (��һ�M) (2 bytes)
#define PARAM_C_TPL_2              352   // �r�g�}�n�� (�ڶ��M) (2 bytes)
#define PARAM_C_TRD_2              354   // ԭʼ�����r�g (�ڶ��M) (2 bytes)
#define PARAM_C_EV_WIDTH_2         356   // ȼ���r�g (�ڶ��M) (2 bytes)

/* Ӌ��ʽ�a������ */
#define PARAM_C_S2                 358   // ԇƬ�a������
#define PARAM_C_I2                 359   // ԇƬ�a������
#define PARAM_C_SQ                 360   // Ʒ��ҺQC�a������Sq
#define PARAM_C_IQ                 361   // Ʒ��ҺQC�a������Iq
#define PARAM_C_SR                 362   // OPAӲ�wޒ·�a��(б��) (4 bytes)
#define PARAM_C_IR                 366   // OPAӲ�wޒ·�a��(�ؾ�) (4 bytes)
#define PARAM_C_GLU0               370   // ���У������(���y�ֶ��c) (14 bytes)
#define PARAM_C_GOAL0              384   // ���У������(Ŀ��ֵ�ֶ��c) (14 bytes)
#define PARAM_C_S3                 398   // ԇƬ�a������S3(1-15) (15 bytes)
#define PARAM_C_I3                 413   // ԇƬ�a������I3(1-15) (15 bytes)

/* �ض��a������ */
#define PARAM_C_TF                 428   // AC/PC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_C_CTF                435   // QC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_C_TO                 442   // AC/PC�ض��a���ؾ�(10-40��C) (7 bytes)
#define PARAM_C_CTO                449   // QC�ض��a���ؾ�(10-40��C) (7 bytes)

/* Ӌ���პ���a������ */
#define PARAM_C_CVQ                456   // CV Level of QC
#define PARAM_C_AQ                 457   // A of QC Compensation
#define PARAM_C_BQ                 458   // B of QC Compensation
#define PARAM_C_TCV                459   // Time of BG Optimize
#define PARAM_C_CVBG               460   // CV Level of BG
#define PARAM_C_ABG                461   // A of BG Compensation
#define PARAM_C_BBG                462   // B of BG Compensation
#define PARAM_C_AQ5                463   // A of QC Compensation Level 5
#define PARAM_C_BQ5                464   // B of QC Compensation Level 5

/* �������֬(TG)���Å��� */
/* ԇƬ���� */
#define PARAM_TG_CSU_TOL           465   // ԇƬ�z���ݲ�
#define PARAM_TG_NDL               466   // ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_TG_UDL               468   // ��ʹ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_TG_BLOOD_IN          470   // ѪҺ�z�yˮƽ (2 bytes)
#define PARAM_TG_EV_W_BACKGROUND   472   // W늘O����ֵ (2 bytes)
#define PARAM_TG_EV_W_PWM_DUTY     474   // W늘OPWMռ�ձ� (2 bytes)
#define PARAM_TG_COUNT_DOWN_TIME   476   // �����r�g

/* �y���������� */
#define PARAM_TG_L                 477   // �������ֵ֬����
#define PARAM_TG_H                 478   // �������ֵ֬����

/* �yԇ�r�򅢔� */
#define PARAM_TG_TPL_1             479   // �r�g�}�n�� (��һ�M) (2 bytes)
#define PARAM_TG_TRD_1             481   // ԭʼ�����r�g (��һ�M) (2 bytes)
#define PARAM_TG_EV_WIDTH_1        483   // ȼ���r�g (��һ�M) (2 bytes)
#define PARAM_TG_TPL_2             485   // �r�g�}�n�� (�ڶ��M) (2 bytes)
#define PARAM_TG_TRD_2             487   // ԭʼ�����r�g (�ڶ��M) (2 bytes)
#define PARAM_TG_EV_WIDTH_2        489   // ȼ���r�g (�ڶ��M) (2 bytes)

/* Ӌ��ʽ�a������ */
#define PARAM_TG_S2                491   // ԇƬ�a������
#define PARAM_TG_I2                492   // ԇƬ�a������
#define PARAM_TG_SQ                493   // Ʒ��ҺQC�a������Sq
#define PARAM_TG_IQ                494   // Ʒ��ҺQC�a������Iq
#define PARAM_TG_SR                495   // OPAӲ�wޒ·�a��(б��) (4 bytes)
#define PARAM_TG_IR                499   // OPAӲ�wޒ·�a��(�ؾ�) (4 bytes)
#define PARAM_TG_GLU0              503   // ���У������(���y�ֶ��c) (14 bytes)
#define PARAM_TG_GOAL0             517   // ���У������(Ŀ��ֵ�ֶ��c) (14 bytes)
#define PARAM_TG_S3                531   // ԇƬ�a������S3(1-15) (15 bytes)
#define PARAM_TG_I3                546   // ԇƬ�a������I3(1-15) (15 bytes)

/* �ض��a������ */
#define PARAM_TG_TF                561   // AC/PC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_TG_CTF               568   // QC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_TG_TO                575   // AC/PC�ض��a���ؾ�(10-40��C) (7 bytes)
#define PARAM_TG_CTO               582   // QC�ض��a���ؾ�(10-40��C) (7 bytes)

/* Ӌ���პ���a������ */
#define PARAM_TG_CVQ               589   // CV Level of QC
#define PARAM_TG_AQ                590   // A of QC Compensation
#define PARAM_TG_BQ                591   // B of QC Compensation
#define PARAM_TG_TCV               592   // Time of BG Optimize
#define PARAM_TG_CVBG              593   // CV Level of BG
#define PARAM_TG_ABG               594   // A of BG Compensation
#define PARAM_TG_BBG               595   // B of BG Compensation
#define PARAM_TG_AQ5               596   // A of QC Compensation Level 5
#define PARAM_TG_BQ5               597   // B of QC Compensation Level 5

/* Ѫ��(GAV)���Å��� */
/* ԇƬ���� */
#define PARAM_GAV_CSU_TOL          598   // ԇƬ�z���ݲ�
#define PARAM_GAV_NDL              599   // ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_GAV_UDL              601   // ��ʹ��ԇƬ�z�yˮƽ (2 bytes)
#define PARAM_GAV_BLOOD_IN         603   // ѪҺ�z�yˮƽ (2 bytes)
#define PARAM_GAV_EV_W_BACKGROUND  605   // W늘O����ֵ (2 bytes)
#define PARAM_GAV_EV_W_PWM_DUTY    607   // W늘OPWMռ�ձ� (2 bytes)
#define PARAM_GAV_COUNT_DOWN_TIME  609   // �����r�g

/* �y���������� */
#define PARAM_GAV_L                610   // Ѫ��ֵ����
#define PARAM_GAV_H                611   // Ѫ��ֵ����
#define PARAM_GAV_T3_E37           612   // T3 ADV�e�`37�ֵ (2 bytes)

/* �yԇ�r�򅢔� */
#define PARAM_GAV_TPL_1            614   // �r�g�}�n�� (��һ�M) (2 bytes)
#define PARAM_GAV_TRD_1            616   // ԭʼ�����r�g (��һ�M) (2 bytes)
#define PARAM_GAV_EV_WIDTH_1       618   // ȼ���r�g (��һ�M) (2 bytes)
#define PARAM_GAV_TPL_2            620   // �r�g�}�n�� (�ڶ��M) (2 bytes)
#define PARAM_GAV_TRD_2            622   // ԭʼ�����r�g (�ڶ��M) (2 bytes)
#define PARAM_GAV_EV_WIDTH_2       624   // ȼ���r�g (�ڶ��M) (2 bytes)

/* Ӌ��ʽ�a������ */
#define PARAM_GAV_S2               626   // ԇƬ�a������
#define PARAM_GAV_I2               627   // ԇƬ�a������
#define PARAM_GAV_SQ               628   // Ʒ��ҺQC�a������Sq
#define PARAM_GAV_IQ               629   // Ʒ��ҺQC�a������Iq
#define PARAM_GAV_SR               630   // OPAӲ�wޒ·�a��(б��) (4 bytes)
#define PARAM_GAV_IR               634   // OPAӲ�wޒ·�a��(�ؾ�) (4 bytes)
#define PARAM_GAV_GLU0             638   // ���У������(���y�ֶ��c) (14 bytes)
#define PARAM_GAV_GOAL0            652   // ���У������(Ŀ��ֵ�ֶ��c) (14 bytes)
#define PARAM_GAV_S3               666   // ԇƬ�a������S3(1-30) (30 bytes)
#define PARAM_GAV_I3               696   // ԇƬ�a������I3(1-30) (30 bytes)

/* AC�a������ */
#define PARAM_GAV_AC_L_WAVE_L      726   // AC����Д� (2 bytes)
#define PARAM_GAV_AC_H_WAVE_L      728   // AC����Д� (2 bytes)
#define PARAM_GAV_S4_0             730   // AC�a�� (HCT10)
#define PARAM_GAV_S4_1             731   // AC�a�� (HCT20)
#define PARAM_GAV_S4_2             732   // AC�a�� (HCT30)
#define PARAM_GAV_S4_3             733   // AC�a�� (HCT40)
#define PARAM_GAV_S4_4             734   // AC�a�� (HCT50)
#define PARAM_GAV_S4_5             735   // AC�a�� (HCT60)
#define PARAM_GAV_S4_6             736   // AC�a�� (HCT70)
#define PARAM_GAV_I4_0             737   // AC�a�� (HCT10)
#define PARAM_GAV_I4_1             738   // AC�a�� (HCT20)
#define PARAM_GAV_I4_2             739   // AC�a�� (HCT30)
#define PARAM_GAV_I4_3             740   // AC�a�� (HCT40)
#define PARAM_GAV_I4_4             741   // AC�a�� (HCT50)
#define PARAM_GAV_I4_5             742   // AC�a�� (HCT60)
#define PARAM_GAV_I4_6             743   // AC�a�� (HCT70)
#define PARAM_GAV_AC_ADC_0         744   // AC�a���ж� (HCT10) (2 bytes)
#define PARAM_GAV_AC_ADC_1         746   // AC�a���ж� (HCT20) (2 bytes)
#define PARAM_GAV_AC_ADC_2         748   // AC�a���ж� (HCT30) (2 bytes)
#define PARAM_GAV_AC_ADC_3         750   // AC�a���ж� (HCT40) (2 bytes)
#define PARAM_GAV_AC_ADC_4         752   // AC�a���ж� (HCT50) (2 bytes)
#define PARAM_GAV_AC_ADC_5         754   // AC�a���ж� (HCT60) (2 bytes)
#define PARAM_GAV_AC_ADC_6         756   // AC�a���ж� (HCT70) (2 bytes)

/* �ض��a������ */
#define PARAM_GAV_TF               758   // AC/PC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_GAV_CTF              765   // QC�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_GAV_TO               772   // AC/PC�ض��a���ؾ�(10-40��C) (7 bytes)
#define PARAM_GAV_CTO              779   // QC�ض��a���ؾ�(10-40��C) (7 bytes)
#define PARAM_GAV_AC_TF            786   // AC(����)�ض��a��б��(10-40��C) (7 bytes)
#define PARAM_GAV_AC_TO            793   // AC(����)�ض��a���ؾ�(10-40��C) (7 bytes)

/* Ӌ���პ���a������ */
#define PARAM_GAV_CVQ              800   // CV Level of QC
#define PARAM_GAV_AQ               801   // A of QC Compensation
#define PARAM_GAV_BQ               802   // B of QC Compensation
#define PARAM_GAV_TCV              803   // Time of BG Optimize
#define PARAM_GAV_CVBG             804   // CV Level of BG
#define PARAM_GAV_ABG              805   // A of BG Compensation
#define PARAM_GAV_BBG              806   // B of BG Compensation
#define PARAM_GAV_AQ5              807   // A of QC Compensation Level 5
#define PARAM_GAV_BQ5              808   // B of QC Compensation Level 5

/* �����cУ�^ */
#define PARAM_RESERVED             809   // ����^��δ��Uչ (36 bytes)
#define PARAM_SUM_L                845   // ��ַ0~844У��(��)
#define PARAM_SUM_H                846   // ��ַ0~844У��(��)
#define PARAM_CRC16                847   // ����������CRCУ�

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_CODE_TABLE_H */

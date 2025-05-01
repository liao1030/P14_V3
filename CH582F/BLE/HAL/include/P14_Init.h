#ifndef __P14_INIT_H__
#define __P14_INIT_H__

#include "CH58x_common.h"

/* �������O�� */
#define STRIP_DEBOUNCE_TIME    20      // �������r�g�����룩
#define STRIP_DETECT_INTERVAL  100     // ԇƬ�ɜy�g�������룩

/* ԇƬ��B���x */
typedef enum {
    STRIP_STATE_NONE,      // �oԇƬ
    STRIP_STATE_INSERTING, // �����^���У���������
    STRIP_STATE_INSERTED,  // �Ѳ���
    STRIP_STATE_READY      // �ќʂ�Üy��
} StripState_TypeDef;

/* ԇƬ��Ͷ��x (�cCH32V203ͨ��) */
/* ע�⣺ԇƬ���ö�e��P14_Parameter_Table.h�ṩ��
 * ��CH32V203���Ѷ��x���@�e�H������Ƕ��x��
 * �_��CH582F��CH32V203ʹ����ͬ��ö�eֵ��
 */
typedef enum {
    STRIP_TYPE_UNKNOWN = 255, // δ֪���e�`
    STRIP_TYPE_GLV = 0,       // Ѫ��(GLV)
    STRIP_TYPE_U = 1,         // ����
    STRIP_TYPE_C = 2,         // ��đ�̴�
    STRIP_TYPE_TG = 3,        // �������֬
    STRIP_TYPE_GAV = 4        // Ѫ��(GAV)
} StripType_TypeDef;

/* GPIO��ʼ�����P���� */
void P14_CH582F_GPIO_Init(void);
void P14_CH582F_UART_Init(void);
void P14_CH582F_System_Init(void);

/* ԇƬ����ɜy���P���� */
void P14_StripDetectInit(void);
uint8_t P14_CheckStripInsertion(void);
StripType_TypeDef P14_IdentifyStripType(void);
void P14_NotifyStripInserted(StripType_TypeDef type);
void P14_StripStateReset(void);

/* UART1����̎�����P���� */
void P14_UART1_RxHandler(uint8_t rx_data);
void P14_UART1_CheckTimeout(void);

#endif /* __P14_INIT_H__ */ 
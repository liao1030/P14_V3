/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_uart_service.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/19
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "CONFIG.h"
#include "gattprofile.h"
#include "stdint.h"
#include "ble_uart_service.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED    7

#define RAWPASS_TX_VALUE_HANDLE       4
#define RAWPASS_RX_VALUE_HANDLE       2
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// ʹ��16λUUID����128λUUID
static CONST uint16 ble_uart_ServiceUUID = BLE_UART_SERVICE_UUID;
static CONST uint16 ble_uart_TxCharUUID  = BLE_UART_TX_CHAR_UUID;
static CONST uint16 ble_uart_RxCharUUID  = BLE_UART_RX_CHAR_UUID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static ble_uart_ProfileChangeCB_t ble_uart_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Profile Service attribute
static CONST gattAttrType_t ble_uart_Service = {ATT_BT_UUID_SIZE, (uint8 *)&ble_uart_ServiceUUID};

// Profile Characteristic 1 Properties - RX����
static uint8 ble_uart_RxCharProps = GATT_PROP_WRITE_NO_RSP;

// ���վ��n�^���m��f�hҪ��
static uint8 ble_uart_RxCharValue[BLE_UART_RX_BUFF_SIZE];

// Profile Characteristic 2 Properties - TX����
static uint8 ble_uart_TxCharProps = GATT_PROP_NOTIFY;

// �l�͔���ֵ
static uint8 ble_uart_TxCharValue = 0;

// TX���Կ͑�������������
static gattCharCfg_t ble_uart_TxCCCD[4];

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t ble_uart_ProfileAttrTbl[] = {
    // Simple Profile Service
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8 *)&ble_uart_Service              /* pValue */
    },

    // Characteristic 1 Declaration - RX������
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ble_uart_RxCharProps
    },

    // Characteristic Value 1 - RX����ֵ
    {
        {ATT_BT_UUID_SIZE, (uint8 *)&ble_uart_RxCharUUID},
        GATT_PERMIT_WRITE,
        0,
        &ble_uart_RxCharValue[0]
    },

    // Characteristic 2 Declaration - TX������
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ble_uart_TxCharProps
    },

    // Characteristic Value 2 - TX����ֵ
    {
        {ATT_BT_UUID_SIZE, (uint8 *)&ble_uart_TxCharUUID},
        0,
        0,
        (uint8 *)&ble_uart_TxCharValue
    },

    // Characteristic 2 User Description - TX���Կ͑�������������
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)ble_uart_TxCCCD
    },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t ble_uart_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset, uint16 maxLen, uint8 method);
static bStatus_t ble_uart_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                      uint8 *pValue, uint16 len, uint16 offset, uint8 method);

static void ble_uart_HandleConnStatusCB(uint16 connHandle, uint8 changeType);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
gattServiceCBs_t ble_uart_ProfileCBs = {
    ble_uart_ReadAttrCB,  // Read callback function pointer
    ble_uart_WriteAttrCB, // Write callback function pointer
    NULL                  // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      ble_uart_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t ble_uart_add_service(ble_uart_ProfileChangeCB_t cb)
{
    uint8 status = SUCCESS;

    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, ble_uart_TxCCCD);
    // Register with Link DB to receive link status change callback
    linkDB_Register(ble_uart_HandleConnStatusCB);

    //    ble_uart_TxCCCD.connHandle = INVALID_CONNHANDLE;
    //    ble_uart_TxCCCD.value = 0;
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(ble_uart_ProfileAttrTbl,
                                         GATT_NUM_ATTRS(ble_uart_ProfileAttrTbl),
                                         GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &ble_uart_ProfileCBs);
    if(status != SUCCESS)
        PRINT("Add ble uart service failed!\n");
    ble_uart_AppCBs = cb;

    return (status);
}

/*********************************************************************
 * @fn          ble_uart_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static bStatus_t ble_uart_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset, uint16 maxLen, uint8 method)
{
    bStatus_t status = SUCCESS;
    PRINT("ReadAttrCB\n");

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            *pLen = 2;
            tmos_memcpy(pValue, pAttr->pValue, 2);
        }
    }
    else
    {
        if(tmos_memcmp(pAttr->type.uuid, ble_uart_TxCharUUID, 16))
        {
            *pLen = 1;
            pValue[0] = '1';
        }
        else if(tmos_memcmp(pAttr->type.uuid, ble_uart_RxCharUUID, 16))
        {
            PRINT("read tx char\n");
        }
    }

    return (status);
}

/*********************************************************************
 * @fn      ble_uart_WriteAttrCB
 *
 * @brief   ������Ի��{
 *
 * @param   connHandle - �B�Ӿ��
 * @param   pAttr - ����
 * @param   pValue - ����
 * @param   len - �L��
 * @param   offset - ƫ��
 * @param   method - ����
 *
 * @return  ��B
 */
static bStatus_t ble_uart_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                      uint8 *pValue, uint16 len, uint16 offset, uint8 method)
{
    bStatus_t status = SUCCESS;
    uint16 uuid;

    // BLE UART RX����
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // �@ȡUUID
        uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if(uuid == ble_uart_RxCharUUID)
        {
            if(ble_uart_AppCBs != NULL)
            {
                ble_uart_evt_t evt;
                evt.type = BLE_UART_EVT_BLE_DATA_RECIEVED;
                evt.data.p_data = pValue;
                evt.data.length = len;
                
                // �|�l���{
                ble_uart_AppCBs(connHandle, &evt);
            }
        }
        else if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                     offset, GATT_CLIENT_CFG_NOTIFY);
            if(status == SUCCESS)
            {
                // �@ȡTX���ԵĿ͑�������������ֵ
                uint16 charCfg = BUILD_UINT16(pValue[0], pValue[1]);
                
                // ֪ͨʹ�ܠ�B׃��
                ble_uart_evt_t evt;
                evt.type = (charCfg & GATT_CLIENT_CFG_NOTIFY) ? 
                            BLE_UART_EVT_TX_NOTI_ENABLED : 
                            BLE_UART_EVT_TX_NOTI_DISABLED;
                    
                // �|�l���{
                if(ble_uart_AppCBs != NULL)
                {
                    ble_uart_AppCBs(connHandle, &evt);
                }
            }
        }
        else
        {
            // ��֧�ֵ�Ո��
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    else
    {
        // ��֧�ֵ�Ո��
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    return status;
}

/*********************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void ble_uart_HandleConnStatusCB(uint16 connHandle, uint8 changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            //ble_uart_TxCCCD[0].value = 0;
            GATTServApp_InitCharCfg(connHandle, ble_uart_TxCCCD);
            //PRINT("clear client configuration\n");
        }
    }
}

uint8 ble_uart_notify_is_ready(uint16 connHandle)
{
    return (GATT_CLIENT_CFG_NOTIFY == GATTServApp_ReadCharCfg(connHandle, ble_uart_TxCCCD));
}
/*********************************************************************
 * @fn          BloodPressure_IMeasNotify
 *
 * @brief       Send a notification containing a bloodPressure
 *              measurement.
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */
bStatus_t ble_uart_notify(uint16 connHandle, attHandleValueNoti_t *pNoti, uint8 taskId)
{
    uint16 value = GATTServApp_ReadCharCfg(connHandle, ble_uart_TxCCCD);

    // �z��֪ͨ�Ƿ��ц���
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // �O��֪ͨ���Ծ��
        pNoti->handle = ble_uart_ProfileAttrTbl[RAWPASS_TX_VALUE_HANDLE].handle;
        
        // �l��֪ͨ
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

/*********************************************************************
*********************************************************************/

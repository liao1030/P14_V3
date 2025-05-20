/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "HAL.h"
#include "batt_measure.h"

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      App_TestBatteryVoltage
 *
 * @brief   œyÔ‡ëŠ³ØëŠ‰ºœyÁ¿¹¦ÄÜ
 *
 * @param   none
 *
 * @return  none
 */
void App_TestBatteryVoltage(void)
{
    uint16_t voltage = Batt_GetVoltage();
    uint8_t status = Batt_GetStatus();
    
    PRINT("Battery Voltage Test\n");
    PRINT("--------------------\n");
    PRINT("Measured Voltage: %d mV\n", voltage);
    PRINT("Battery Status: ");
    
    if(status == BATT_STATUS_LOW)
    {
        PRINT("LOW (Below 2500mV)\n");
    }
    else if(status == BATT_STATUS_NORMAL)
    {
        PRINT("NORMAL\n");
    }
    else if(status == BATT_STATUS_CHARGING)
    {
        PRINT("CHARGING\n");
    }
    
    PRINT("--------------------\n\n");
}

/********************************** (C) COPYRIGHT *******************************
 * File Name          : param_test.c
 * Author             : HMD
 * Version            : V1.0.0
 * Date               : 2024/06/12
 * Description        : Test program for parameter code table.
 *********************************************************************************
 * Copyright (c) 2024 HMD Biomedical Inc.
 *******************************************************************************/

#include "param_table.h"
#include "debug.h"

/*********************************************************************
 * @fn      Param_TestMenu
 *
 * @brief   Display a menu for testing parameter code table functions
 *
 * @return  none
 */
void Param_TestMenu(void)
{
    uint8_t choice = 0;
    int32_t value = 0;
    uint8_t strip_type = 0;
    uint8_t exit_flag = 0;
    uint8_t param_id = 0;
    uint8_t status = 0;
    
    while (!exit_flag) {
        printf("\r\n=============================\r\n");
        printf("Parameter Table Test Menu\r\n");
        printf("=============================\r\n");
        printf("1. Display Current Parameters\r\n");
        printf("2. Change Strip Type\r\n");
        printf("3. Change Measurement Unit\r\n");
        printf("4. Change Event Type\r\n");
        printf("5. Increment Test Count\r\n");
        printf("6. Load Default Parameters\r\n");
        printf("7. Save Parameters to Flash\r\n");
        printf("8. Verify Parameters\r\n");
        printf("9. Set Parameter by ID\r\n");
        printf("0. Exit\r\n");
        printf("Enter choice (0-9): ");
        
        choice = USART_ReceiveData(USART1) - '0';
        if (choice > 9) continue;
        
        printf("%d\r\n", choice);
        Delay_Ms(100);
        
        switch (choice) {
            case 0:
                exit_flag = 1;
                printf("Exiting parameter test menu...\r\n");
                break;
                
            case 1:
                printf("\r\nCurrent Parameters:\r\n");
                printf("- Model No.: %d\r\n", gParamTable.MODEL_NO);
                printf("- Firmware No.: %d\r\n", gParamTable.FW_NO);
                printf("- Code Table Version: %d\r\n", gParamTable.Code_Table_V);
                printf("- Test Count: %d\r\n", gParamTable.NOT);
                printf("- Date/Time: 20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
                       gParamTable.YEAR, gParamTable.MONTH, gParamTable.DATE,
                       gParamTable.HOUR, gParamTable.MINUTE, gParamTable.SECOND);
                printf("- Strip Type: %s\r\n", Param_GetStripTypeName(gParamTable.Strip_Type));
                printf("- Measurement Unit: %s\r\n", Param_GetUnitName(gParamTable.MGDL));
                printf("- Event Type: %s\r\n", Param_GetEventTypeName(gParamTable.EVENT));
                printf("- Temperature Range: %d¡ãC ~ %d¡ãC\r\n", gParamTable.TLL, gParamTable.TLH);
                
                /* Display limits based on current strip type */
                switch (gParamTable.Strip_Type) {
                    case STRIP_TYPE_GLV:
                    case STRIP_TYPE_GAV:
                        printf("- Blood Glucose Limits: %d ~ %d\r\n", gParamTable.BG_L, gParamTable.BG_H);
                        break;
                    case STRIP_TYPE_U:
                        printf("- Uric Acid Limits: %d ~ %d\r\n", gParamTable.U_L, gParamTable.U_H);
                        break;
                    case STRIP_TYPE_C:
                        printf("- Cholesterol Limits: %d ~ %d\r\n", gParamTable.C_L, gParamTable.C_H);
                        break;
                    case STRIP_TYPE_TG:
                        printf("- Triglycerides Limits: %d ~ %d\r\n", gParamTable.TG_L, gParamTable.TG_H);
                        break;
                }
                break;
                
            case 2:
                printf("\r\nSelect Strip Type:\r\n");
                printf("0. Blood Glucose (GLV)\r\n");
                printf("1. Uric Acid (U)\r\n");
                printf("2. Cholesterol (C)\r\n");
                printf("3. Triglycerides (TG)\r\n");
                printf("4. Blood Glucose (GAV)\r\n");
                printf("Enter strip type (0-4): ");
                
                strip_type = USART_ReceiveData(USART1) - '0';
                if (strip_type > 4) {
                    printf("\r\nInvalid strip type!\r\n");
                    break;
                }
                
                printf("%d\r\n", strip_type);
                gParamTable.Strip_Type = strip_type;
                printf("Strip type changed to: %s\r\n", Param_GetStripTypeName(strip_type));
                break;
                
            case 3:
                printf("\r\nSelect Measurement Unit:\r\n");
                printf("0. mmol/L\r\n");
                printf("1. mg/dL\r\n");
                printf("2. gm/dL\r\n");
                printf("Enter unit (0-2): ");
                
                value = USART_ReceiveData(USART1) - '0';
                if (value > 2) {
                    printf("\r\nInvalid unit!\r\n");
                    break;
                }
                
                printf("%d\r\n", (int)value);
                gParamTable.MGDL = value;
                printf("Measurement unit changed to: %s\r\n", Param_GetUnitName(value));
                break;
                
            case 4:
                printf("\r\nSelect Event Type:\r\n");
                printf("0. QC (Quality Control)\r\n");
                printf("1. AC (Before meal)\r\n");
                printf("2. PC (After meal)\r\n");
                printf("Enter event type (0-2): ");
                
                value = USART_ReceiveData(USART1) - '0';
                if (value > 2) {
                    printf("\r\nInvalid event type!\r\n");
                    break;
                }
                
                printf("%d\r\n", (int)value);
                gParamTable.EVENT = value;
                printf("Event type changed to: %s\r\n", Param_GetEventTypeName(value));
                break;
                
            case 5:
                gParamTable.NOT++;
                printf("\r\nTest count incremented to: %d\r\n", gParamTable.NOT);
                break;
                
            case 6:
                printf("\r\nLoading default parameters...\r\n");
                if (Param_LoadDefault() == 0) {
                    printf("Default parameters loaded successfully.\r\n");
                } else {
                    printf("Failed to load default parameters.\r\n");
                }
                break;
                
            case 7:
                printf("\r\nSaving parameters to flash...\r\n");
                if (Param_Save() == 0) {
                    printf("Parameters saved successfully.\r\n");
                } else {
                    printf("Failed to save parameters.\r\n");
                }
                break;
                
            case 8:
                printf("\r\nVerifying parameters...\r\n");
                if (Param_Verify() == 0) {
                    printf("Parameters are valid.\r\n");
                } else {
                    printf("Parameters are invalid!\r\n");
                }
                break;
                
            case 9:
                printf("\r\nEnter parameter ID (0-%d): ", PARAM_COUNT-1);
                
                param_id = (USART_ReceiveData(USART1) - '0');
                if (param_id >= 10) {
                    printf("\r\nValue too large, please enter a single digit.\r\n");
                    break;
                }
                
                printf("%d\r\n", param_id);
                printf("Enter new value: ");
                
                value = (USART_ReceiveData(USART1) - '0');
                printf("%d\r\n", (int)value);
                
                status = Param_SetValue((Param_ID_TypeDef)param_id, value);
                if (status == 0) {
                    printf("Parameter ID %d set to %d successfully.\r\n", param_id, (int)value);
                } else {
                    printf("Failed to set parameter ID %d.\r\n", param_id);
                }
                break;
                
            default:
                printf("\r\nInvalid choice. Please try again.\r\n");
                break;
        }
    }
} 
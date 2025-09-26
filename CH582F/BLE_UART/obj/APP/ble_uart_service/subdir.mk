################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/ble_uart_service/ble_uart_service_16bit.c 

C_DEPS += \
./APP/ble_uart_service/ble_uart_service_16bit.d 

OBJS += \
./APP/ble_uart_service/ble_uart_service_16bit.o 


EXPANDS += \
./APP/ble_uart_service/ble_uart_service_16bit.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
APP/ble_uart_service/%.o: ../APP/ble_uart_service/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=2 -DCH582F -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Startup" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Profile/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/StdPeriphDriver/inc" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/HAL/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Ld" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/LIB" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/RVMSIS" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/app_drv_fifo" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/ble_uart_service" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"


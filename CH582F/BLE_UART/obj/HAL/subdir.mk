################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HAL/MCU.c \
../HAL/RTC.c \
../HAL/SLEEP.c 

C_DEPS += \
./HAL/MCU.d \
./HAL/RTC.d \
./HAL/SLEEP.d 

OBJS += \
./HAL/MCU.o \
./HAL/RTC.o \
./HAL/SLEEP.o 



# Each subdirectory must supply rules for building sources it contributes
HAL/%.o: ../HAL/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DCH583 -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Startup" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Profile/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/StdPeriphDriver/inc" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/HAL/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Ld" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/LIB" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/RVMSIS" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/app_drv_fifo" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/ble_uart_service" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

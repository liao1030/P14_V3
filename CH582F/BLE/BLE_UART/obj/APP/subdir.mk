################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/P14_BLE_Protocol.c \
../APP/app_uart.c \
../APP/peripheral.c \
../APP/peripheral_main.c 

C_DEPS += \
./APP/P14_BLE_Protocol.d \
./APP/app_uart.d \
./APP/peripheral.d \
./APP/peripheral_main.d 

OBJS += \
./APP/P14_BLE_Protocol.o \
./APP/app_uart.o \
./APP/peripheral.o \
./APP/peripheral_main.o 



# Each subdirectory must supply rules for building sources it contributes
APP/%.o: ../APP/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DCH583 -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/SRC/Startup" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE/BLE_UART/APP/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE/BLE_UART/Profile/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/SRC/StdPeriphDriver/inc" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE/HAL/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/SRC/Ld" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE/LIB" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/SRC/RVMSIS" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE/BLE_UART/APP/app_drv_fifo" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE/BLE_UART/APP/ble_uart_service" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

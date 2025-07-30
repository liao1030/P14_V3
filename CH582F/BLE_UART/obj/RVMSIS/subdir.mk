################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RVMSIS/core_riscv.c 

C_DEPS += \
./RVMSIS/core_riscv.d 

OBJS += \
./RVMSIS/core_riscv.o 


EXPANDS += \
./RVMSIS/core_riscv.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
RVMSIS/%.o: ../RVMSIS/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=2 -DCH582F -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Startup" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Profile/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/StdPeriphDriver/inc" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/HAL/include" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/Ld" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/LIB" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/RVMSIS" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/app_drv_fifo" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/BLE_UART/APP/ble_uart_service" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"


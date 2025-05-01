################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/P14_BLE_Protocol.c \
../User/P14_Flash_Storage.c \
../User/P14_Init.c \
../User/P14_Parameter_Table.c \
../User/P14_UART.c \
../User/ch32v20x_it.c \
../User/main.c \
../User/system_ch32v20x.c 

C_DEPS += \
./User/P14_BLE_Protocol.d \
./User/P14_Flash_Storage.d \
./User/P14_Init.d \
./User/P14_Parameter_Table.d \
./User/P14_UART.d \
./User/ch32v20x_it.d \
./User/main.d \
./User/system_ch32v20x.d 

OBJS += \
./User/P14_BLE_Protocol.o \
./User/P14_Flash_Storage.o \
./User/P14_Init.o \
./User/P14_Parameter_Table.o \
./User/P14_UART.o \
./User/ch32v20x_it.o \
./User/main.o \
./User/system_ch32v20x.o 



# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Debug" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Core" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/User" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

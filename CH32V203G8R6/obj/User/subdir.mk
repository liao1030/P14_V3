################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Calculation.c \
../User/ch32v20x_it.c \
../User/main.c \
../User/param_table.c \
../User/rtc.c \
../User/strip_detect.c \
../User/system_ch32v20x.c \
../User/system_state.c \
../User/uart_protocol.c \
../User/uart_protocol_stubs.c 

C_DEPS += \
./User/Calculation.d \
./User/ch32v20x_it.d \
./User/main.d \
./User/param_table.d \
./User/rtc.d \
./User/strip_detect.d \
./User/system_ch32v20x.d \
./User/system_state.d \
./User/uart_protocol.d \
./User/uart_protocol_stubs.d 

OBJS += \
./User/Calculation.o \
./User/ch32v20x_it.o \
./User/main.o \
./User/param_table.o \
./User/rtc.o \
./User/strip_detect.o \
./User/system_ch32v20x.o \
./User/system_state.o \
./User/uart_protocol.o \
./User/uart_protocol_stubs.o 



# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Debug" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Core" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/User" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

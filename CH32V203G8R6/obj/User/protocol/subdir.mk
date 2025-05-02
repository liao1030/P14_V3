################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/protocol/protocol.c \
../User/protocol/uart_comm.c 

C_DEPS += \
./User/protocol/protocol.d \
./User/protocol/uart_comm.d 

OBJS += \
./User/protocol/protocol.o \
./User/protocol/uart_comm.o 



# Each subdirectory must supply rules for building sources it contributes
User/protocol/%.o: ../User/protocol/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Debug" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Core" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/User" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/Peripheral/inc" -I"/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH32V203G8R6/User/parameter" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

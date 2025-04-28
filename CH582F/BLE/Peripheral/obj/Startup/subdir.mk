################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
/Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/SRC/Startup/startup_CH583.S 

S_UPPER_DEPS += \
./Startup/startup_CH583.d 

OBJS += \
./Startup/startup_CH583.o 



# Each subdirectory must supply rules for building sources it contributes
Startup/startup_CH583.o: /Users/liao1030/HMD/_Projects/_P14_V2_1/Firmware/P14_V3/CH582F/SRC/Startup/startup_CH583.S
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"


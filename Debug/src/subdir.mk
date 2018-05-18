################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cr_startup_lpc43xx.c \
../src/ssp.c \
../src/sysinit.c 

OBJS += \
./src/cr_startup_lpc43xx.o \
./src/ssp.o \
./src/sysinit.o 

C_DEPS += \
./src/cr_startup_lpc43xx.d \
./src/ssp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__MULTICORE_NONE -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -D__REDLIB__ -DCORE_M4 -I"/Users/jesse/Documents/MCUXpressoIDE_10.1.1/workspace/lpc_chip_43xx/inc" -I"/Users/jesse/Documents/MCUXpressoIDE_10.1.1/workspace/lpc_chip_43xx/inc/config_43xx" -I"/Users/jesse/Documents/MCUXpressoIDE_10.1.1/workspace/lpc_board_nxp_lpcxpresso_4337/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



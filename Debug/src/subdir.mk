################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cdc_desc.c \
../src/cdc_vcom.c \
../src/cr_startup_lpc43xx.c \
../src/ssp.c \
../src/sysinit.c 

OBJS += \
./src/cdc_desc.o \
./src/cdc_vcom.o \
./src/cr_startup_lpc43xx.o \
./src/ssp.o \
./src/sysinit.o 

C_DEPS += \
./src/cdc_desc.d \
./src/cdc_vcom.d \
./src/cr_startup_lpc43xx.d \
./src/ssp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__MULTICORE_NONE -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -D__REDLIB__ -DCORE_M4 -I"/home/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/lpc_chip_43xx/inc" -I"/home/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/lpc_chip_43xx/inc/usbd_rom" -I"/home/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/LPC_SPI/inc" -I"/home/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/lpc_chip_43xx/inc/config_43xx" -I"/home/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/lpc_board_nxp_lpcxpresso_4337/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



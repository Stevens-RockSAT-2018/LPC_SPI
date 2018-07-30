################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: LPC_SPI.axf

# Tool invocations
LPC_SPI.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"/Users/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/lpc_chip_43xx/Debug" -L"/Users/jesse/Sync/RockSAT Vibe Iso/rocksat_workspace/lpc_board_nxp_lpcxpresso_4337/Debug" -Xlinker -Map="LPC_SPI.map" -Xlinker --gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -T "periph_ssp_Debug.ld" -o "LPC_SPI.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) LPC_SPI.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "LPC_SPI.axf"; # arm-none-eabi-objcopy -v -O binary "LPC_SPI.axf" "LPC_SPI.bin" ; # checksum -p LPC4337 -d "LPC_SPI.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets

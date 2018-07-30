#define PIN_CS1 5,6 // P2_6
#define PIN_CS2 1,12 // P2_12
#define PIN_CS3 1,10 // P2_9
#define PIN_CS4 1,0 // P1_7
#define PIN_CS5 5,2 // P2_2
#define PIN_CS6 1,1 // P1_8

#define MODE_CS1 SCU_MODE_FUNC4
#define MODE_CS2 SCU_MODE_FUNC0
#define MODE_CS3 SCU_MODE_FUNC0
#define MODE_CS4 SCU_MODE_FUNC0
#define MODE_CS5 SCU_MODE_FUNC4
#define MODE_CS6 SCU_MODE_FUNC0

#define PIN_EOC1 5,8 // P3_1
#define MODE_EOC1 3, 1, SCU_MODE_INACT | SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN
#define PIN_CNVST 3,4 // P6_5
#define MODE_CNVST SCU_MODE_FUNC0
#define NUM_CS 5


void set_chip_select(int accel, bool state) {
	switch (accel) {
	case 1:
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, PIN_CS1, state);
		break;
	case 2:
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, PIN_CS2, state);
		break;
	case 3:
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, PIN_CS3, state);
		break;
	case 4:
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, PIN_CS4, state);
		break;
	case 5:
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, PIN_CS5, state);
		break;
	case 6:
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, PIN_CS6, state);
		break;
	}
}


void SIT_Pin_Setup() {
	Chip_SCU_PinMuxSet(2,6, MODE_CS1);
	Chip_SCU_PinMuxSet(2,12, MODE_CS2);
	Chip_SCU_PinMuxSet(2,9, MODE_CS3);
	Chip_SCU_PinMuxSet(1,7, MODE_CS4);
	Chip_SCU_PinMuxSet(2,2, MODE_CS5);
	Chip_SCU_PinMuxSet(1,8, MODE_CS6);

	Chip_SCU_PinMuxSet(MODE_EOC1);
//	Chip_SCU_PinMuxSet(PIN_CNVST, MODE_CNVST);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CS1);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CS2);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CS3);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CS4);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CS5);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CS6);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, PIN_EOC1);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PIN_CNVST);
	for (int i = 1; i <= NUM_CS; i++) {
		set_chip_select(i, true);
		ADC_setup(i);
	}
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, PIN_CNVST);
}

void wait_for_conversion(int accel) {
	while (Chip_GPIO_GetPinState(LPC_GPIO_PORT, PIN_EOC1) == false) {
		__WFI();
	}
}

void start_conversion() {
	Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, PIN_CNVST);
}

void finish_conversion() {
	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, PIN_CNVST);
}

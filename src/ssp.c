/*
 * @brief SSP example
 * This example show how to use the SSP in 3 modes : Polling, Interrupt and DMA
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "stdio.h"
#include <string.h>
#include "app_usbd_cfg.h"
#include "cdc_vcom.h"
#include "pin_cfg.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#if (defined(BOARD_KEIL_MCB_1857) || defined(BOARD_KEIL_MCB_4357) || \
	defined(BOARD_NGX_XPLORER_1830) || defined(BOARD_NGX_XPLORER_4330) || \
	defined(BOARD_NXP_LPCLINK2_4370) || defined(BOARD_NXP_LPCXPRESSO_4337) || \
	defined(BOARD_NXP_LPCXPRESSO_1837))
#define LPC_SSP           LPC_SSP1
#define SSP_IRQ           SSP1_IRQn
#define LPC_GPDMA_SSP_TX  GPDMA_CONN_SSP1_Tx
#define LPC_GPDMA_SSP_RX  GPDMA_CONN_SSP1_Rx
#define SSPIRQHANDLER SSP1_IRQHandler
#elif (defined(BOARD_HITEX_EVA_1850) || defined(BOARD_HITEX_EVA_4350))
#define LPC_SSP           LPC_SSP0
#define SSP_IRQ           SSP0_IRQn
#define LPC_GPDMA_SSP_TX  GPDMA_CONN_SSP0_Tx
#define LPC_GPDMA_SSP_RX  GPDMA_CONN_SSP0_Rx
#define SSPIRQHANDLER SSP0_IRQHandler
#else
#warning Unsupported Board
#endif
#define BUFFER_SIZE                         (0x100)
#define SSP_DATA_BITS                       (SSP_BITS_8)
#define SSP_DATA_BIT_NUM(databits)          (databits+1)
#define SSP_DATA_BYTES(databits)            (((databits) > SSP_BITS_8) ? 2:1)
#define SSP_LO_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? 0xFF:(0xFF>>(8-SSP_DATA_BIT_NUM(databits))))
#define SSP_HI_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? (0xFF>>(16-SSP_DATA_BIT_NUM(databits))):0)

#define SSP_MODE_SEL                        (0x31)
#define SSP_TRANSFER_MODE_SEL               (0x32)
#define SSP_MASTER_MODE_SEL                 (0x31)
#define SSP_SLAVE_MODE_SEL                  (0x32)
#define SSP_POLLING_SEL                     (0x31)
#define SSP_INTERRUPT_SEL                   (0x32)
#define SSP_DMA_SEL                         (0x33)

#define SETUP_FLAG     	(0x80)
#define SCAN_MODE_NONE 	(0x06)
#define SCAN_MODE_0_N  	(0x00)
#define SCAN_MODE_N_4 	(0x04)

#define CSPIN_ADC1		5
#define CSPIN_ADC2		6
#define CSPIN_ADC3		7
#define CSPIN_ADC4		8
#define CSPIN_ADC5		9
#define CSPIN_ADC6		10
#define debugging_pin 5,2


#define TICKRATE_HZ (10000) /* 1000 ticks per second */

#define NUM_ACCEL 6
static uint32_t accel_readings[NUM_ACCEL+1]; // zeroth element is timestamp


/** USB THINGS **/
static USBD_HANDLE_T g_hUsb;
static uint8_t g_rxBuff[256];

/* Endpoint 0 patch that prevents nested NAK event processing */
static uint32_t g_ep0RxBusy = 0;/* flag indicating whether EP0 OUT/RX buffer is busy. */
static USB_EP_HANDLER_T g_Ep0BaseHdlr;	/* variable to store the pointer to base EP0 handler */

const USBD_API_T *g_pUsbApi;

ErrorCode_t EP0_patch(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	switch (event) {
	case USB_EVT_OUT_NAK:
		if (g_ep0RxBusy) {
			/* we already queued the buffer so ignore this NAK event. */
			return LPC_OK;
		}
		else {
			/* Mark EP0_RX buffer as busy and allow base handler to queue the buffer. */
			g_ep0RxBusy = 1;
		}
		break;

	case USB_EVT_SETUP:	/* reset the flag when new setup sequence starts */
	case USB_EVT_OUT:
		/* we received the packet so clear the flag. */
		g_ep0RxBusy = 0;
		break;
	}
	return g_Ep0BaseHdlr(hUsb, data, event);
}

void USB_IRQHandler(void)
{
	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}


/** End of USB things **/



/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

/* CS_PIN array, for selecting cs pins */
static const uint8_t cs_pin[6] = {
		CSPIN_ADC1, CSPIN_ADC2, CSPIN_ADC3,
		CSPIN_ADC4, CSPIN_ADC5, CSPIN_ADC6
};

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;
static volatile uint8_t  isXferCompleted = 0;
static uint8_t dmaChSSPTx, dmaChSSPRx;
static volatile uint8_t isDmaTxfCompleted = 0;
static volatile uint8_t isDmaRxfCompleted = 0;

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
static uint32_t tick_ct = 0;
void SysTick_Handler(void)
{
	tick_ct += 1;
}


/* Initialize buffer */
static void Buffer_Init(void)
{
	uint16_t i;
	uint8_t ch = 0;

	for (i = 0; i < BUFFER_SIZE; i++) {
		Tx_Buf[i] = ch++;
		Rx_Buf[i] = 0xAA;
	}
}

/* Verify buffer after transfer */
static uint8_t Buffer_Verify(void)
{
	uint16_t i;
	uint8_t *src_addr = (uint8_t *) &Tx_Buf[0];
	uint8_t *dest_addr = (uint8_t *) &Rx_Buf[0];

	for ( i = 0; i < BUFFER_SIZE; i++ ) {

		if (((*src_addr) & SSP_LO_BYTE_MSK(ssp_format.bits)) !=
				((*dest_addr) & SSP_LO_BYTE_MSK(ssp_format.bits))) {
				return 1;
		}
		src_addr++;
		dest_addr++;

		if (SSP_DATA_BYTES(ssp_format.bits) == 2) {
			if (((*src_addr) & SSP_HI_BYTE_MSK(ssp_format.bits)) !=
				  ((*dest_addr) & SSP_HI_BYTE_MSK(ssp_format.bits))) {
					return 1;
			}
			src_addr++;
			dest_addr++;
			i++;
		}
	}
	return 0;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	SSP interrupt handler sub-routine
 * @return	Nothing
 */
void SSPIRQHANDLER(void)
{
	Chip_SSP_Int_Disable(LPC_SSP);	/* Disable all interrupt */
	if (SSP_DATA_BYTES(ssp_format.bits) == 1) {
		Chip_SSP_Int_RWFrames8Bits(LPC_SSP, &xf_setup);
	}
	else {
		Chip_SSP_Int_RWFrames16Bits(LPC_SSP, &xf_setup);
	}

	if ((xf_setup.rx_cnt != xf_setup.length) || (xf_setup.tx_cnt != xf_setup.length)) {
		Chip_SSP_Int_Enable(LPC_SSP);	/* enable all interrupts */
	}
	else {
		isXferCompleted = 1;
	}
}

/**
 * @brief	DMA interrupt handler sub-routine. Set the waiting flag when transfer is successful
 * @return	Nothing
 */
void DMA_IRQHandler(void)
{
	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChSSPTx) == SUCCESS) {
		isDmaTxfCompleted = 1;
	}

	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChSSPRx) == SUCCESS) {
		isDmaRxfCompleted = 1;
	}
}

/**
 * @brief	Main routine for SSP example
 * @return	Nothing
 */
uint32_t ADC_read(uint8_t cs_pin_sel){
	xf_setup.length = 4;
	xf_setup.tx_data = Tx_Buf;
	xf_setup.rx_data = Rx_Buf;
	xf_setup.rx_cnt = xf_setup.tx_cnt = 0;

	set_chip_select(cs_pin_sel, false);
	Tx_Buf[0] = SETUP_FLAG | 1 << 3 | SCAN_MODE_0_N;
	Tx_Buf[1] = 0;
	Tx_Buf[2] = 0;
	Tx_Buf[3] = 0;

	uint32_t error = Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);

	set_chip_select(cs_pin_sel, true);

	uint8_t *rxbuf = xf_setup.rx_data;
	return rxbuf[0] << 24 | rxbuf[1] << 16 | rxbuf[2] << 8 | rxbuf[3];
}


/* void set_cspins(){
	for (int i = 0; i < 5; ++i) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 1, cs_pin[i]);
	}
}*/



void delay_ticks(uint32_t mills) {
	uint32_t start = tick_ct;
	uint32_t stop = tick_ct + mills; // fix if changing systick
	while (tick_ct < stop) {
		__WFI();
	}
}

uint32_t sample_num = 0;

void read_accel() {

	start_conversion();
	delay_ticks(100);
	finish_conversion();
//	wait_for_conversion(1);

	uint32_t timestamp = tick_ct;
	accel_readings[0] = sample_num;
	for (int i = 1; i <= NUM_ACCEL-1; i++) {
		accel_readings[i] = ADC_read(i);
	}
	accel_readings[NUM_ACCEL] = 0;
	sample_num++;

}

enum color {BLUE, RED, GREEN};

void set_LED_color(enum color c) {
	switch (c) {
	case BLUE:
		Board_LED_Set(0, true);
		Board_LED_Set(1, false);
		Board_LED_Set(2, false);
		break;
	case RED:
		Board_LED_Set(0, false);
		Board_LED_Set(1, true);
		Board_LED_Set(2, false);
		break;
	case GREEN:
		Board_LED_Set(0, false);
		Board_LED_Set(1, false);
		Board_LED_Set(2, true);
		break;
	default:
		Board_LED_Set(0, false);
		Board_LED_Set(1, false);
		Board_LED_Set(2, false);
	}
}

void finished_usb() {
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, debugging_pin, false);
}



int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);


	Chip_GPIO_Init(LPC_GPIO_PORT);
//	set_cspins();

	/* SSP initialization */
	Board_SSP_Init(LPC_SSP);

	Chip_SSP_Init(LPC_SSP);
	Chip_SSP_SetBitRate(LPC_SSP, 9000000);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE0;
        Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_Enable(LPC_SSP);

	/* Initialize GPDMA controller */
	Chip_GPDMA_Init(LPC_GPDMA);

	/* Setting GPDMA interrupt */
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);

	/* Setting SSP interrupt */
	NVIC_EnableIRQ(SSP_IRQ);

#if (defined(BOARD_HITEX_EVA_1850) || defined(BOARD_HITEX_EVA_4350))
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0x6, 10);	/* SSEL_MUX_A */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0x6, 11);	/* SSEL_MUX_B */
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0x6, 10, true);
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0x6, 11, false);
#endif

/** USB THINGS **/
USBD_API_INIT_PARAM_T usb_param;
USB_CORE_DESCS_T desc;
ErrorCode_t ret = LPC_OK;
uint32_t prompt = 0, rdCnt = 0;
USB_CORE_CTRL_T *pCtrl;

/* enable clocks and pinmux */
USB_init_pin_clk();

/* Init USB API structure */
g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

/* initialize call back structures */
memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
usb_param.usb_reg_base = LPC_USB_BASE;
usb_param.max_num_ep = 4;
usb_param.mem_base = USB_STACK_MEM_BASE;
usb_param.mem_size = USB_STACK_MEM_SIZE;

/* Set the USB descriptors */
desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
desc.string_desc = (uint8_t *) USB_StringDescriptor;
#ifdef USE_USB0
desc.high_speed_desc = USB_HsConfigDescriptor;
desc.full_speed_desc = USB_FsConfigDescriptor;
desc.device_qualifier = (uint8_t *) USB_DeviceQualifier;
#else
/* Note, to pass USBCV test full-speed only devices should have both
 * descriptor arrays point to same location and device_qualifier set
 * to 0.
 */
desc.high_speed_desc = USB_FsConfigDescriptor;
desc.full_speed_desc = USB_FsConfigDescriptor;
desc.device_qualifier = 0;
#endif

/* USB Initialization */
ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
if (ret == LPC_OK) {

	/*	WORKAROUND for artf45032 ROM driver BUG:
			Due to a race condition there is the chance that a second NAK event will
			occur before the default endpoint0 handler has completed its preparation
			of the DMA engine for the first NAK event. This can cause certain fields
			in the DMA descriptors to be in an invalid state when the USB controller
			reads them, thereby causing a hang.
	 */
	pCtrl = (USB_CORE_CTRL_T *) g_hUsb;	/* convert the handle to control structure */
	g_Ep0BaseHdlr = pCtrl->ep_event_hdlr[0];/* retrieve the default EP0_OUT handler */
	pCtrl->ep_event_hdlr[0] = EP0_patch;/* set our patch routine as EP0_OUT handler */

	/* Init VCOM interface */
	ret = vcom_init(g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {
		/*  enable USB interrupts */
		NVIC_EnableIRQ(LPC_USB_IRQ);
		/* now connect */
		USBD_API->hw->Connect(g_hUsb, 1);
	}

}

DEBUGSTR("USB CDC class based virtual Comm port example!\r\n");

//Chip_SCU_PinMuxSet(debugging_pin, SCU_MODE_FUNC4);
//Chip_SCU_PinMuxSet(2,2, SCU_MODE_FUNC4);
//
//Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, debugging_pin); // P2_2

SIT_Pin_Setup();

//while (1) {
//	delay_ticks(500);
//	set_LED_color(RED);
//	delay_ticks(500);
//	set_LED_color(GREEN);
//	delay_ticks(500);
//	set_LED_color(BLUE);
//}

	while (1) {
		if (vcom_connected() != 0) {
			set_LED_color(RED);
			read_accel();
			vcom_write(accel_readings, (NUM_ACCEL+1)*4);
			Chip_GPIO_SetPinState(LPC_GPIO_PORT, debugging_pin, true);
			delay_ticks(1000);
		} else {
			set_LED_color(BLUE);
		}
	}

	return 0;
}

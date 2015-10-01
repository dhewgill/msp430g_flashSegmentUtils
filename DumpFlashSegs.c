#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "flash_utils.h"

// #########################
// Defines
#define USE_DCO		1
#define NEWLINE		putStr("\r\n")


// #########################
// Function Prototypes
char* itoa(int value, char* result, int base);
char* utoa(unsigned int value, char* result, int base);
void putStr(const char *outStr);
static inline void putChar(char theChar);


// #########################
// Global Variables
volatile unsigned int gStartAddr;
const char gSegAHdr[] = "Segment A:\r\n";
const char gSegBHdr[] = "Segment B:\r\n";
const char gSegCHdr[] = "Segment C:\r\n";
const char gSegDHdr[] = "Segment D:\r\n";

// #########################
// Start Main Program
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    DCOCTL = 0;					// Run at 16 MHz
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;		// Set clock to 16MHz
	BCSCTL3 |= XCAP_3;			// 12.5 pF for LaunchPad crystal.
	__delay_cycles(1000000);	// Wait for crystal to stabilize.

	//Local variable declaration.
	unsigned char *Flash_ptr;
	char tmpStr[40];
	int i,j;
	
	//Set up the IO.
	P1DIR = BIT0;						// P1.0 output, all others input.
	P1OUT = BIT0;						// P1.0 high.
	P1SEL = BIT1 | BIT2;				// P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 | BIT2;				// P1.1 = RXD, P1.2=TXD

	//Set up the serial port for 9600b:
#if USE_DCO == 1
	UCA0CTL1 |= UCSSEL_2;				// CLK = SMCLK
	UCA0BR0 = 0x82;						// 16MHz/9600 = 1666 = 0x0682
	UCA0BR1 = 0x06;						//
	UCA0MCTL = UCBRS_6;					// Modulation UCBRSx = 6, no oversampling. <- datasheet setting!
#else
	UCA0CTL1 |= UCSSEL_1;				// CLK = ACLK
	UCA0BR0 = 0x03;						// 32kHz/9600 = 3.41
	UCA0BR1 = 0x00;						//
	UCA0MCTL = UCBRS_3;					// Modulation UCBRSx = 3
#endif
	IFG2 &= ~UCA0RXIFG;
	UCA0CTL1 &= ~UCSWRST;				// **Initialize USCI state machine**

	IE2 |= UCA0RXIE;					// Enable USCI_A0 RX interrupt

	//copy_segA_to_segB();
	erase_segB();

	for (;;)
	{
		__bis_SR_register(LPM3_bits + GIE);		// LPM3 with interrupts enabled
		__no_operation();

		//Dump all of the requested flash segment data to the serial port:
		Flash_ptr = (unsigned char *)gStartAddr;
		switch (gStartAddr)
		{
		case SEGA_HEAD:
			putStr(gSegAHdr);
			break;
		case SEGB_HEAD:
			putStr(gSegBHdr);
			break;
		case SEGC_HEAD:
			putStr(gSegCHdr);
			break;
		case SEGD_HEAD:
			putStr(gSegDHdr);
			break;
		default:
			putStr("Unknown:\r\n");
			break;
		}

		for (i = gStartAddr; i < gStartAddr + SEG_LEN; i+=8)
		{
			putStr("0x");
			utoa(i, tmpStr, 16);
			putStr(tmpStr);
			putStr(":\t");
			for (j = 0; j < 8; j++)
			{
				utoa(*Flash_ptr++, tmpStr, 16);
				putStr(tmpStr);
				if (j < 7) putStr("\t");
			}
			NEWLINE;
		}
		NEWLINE;

		P1OUT ^= BIT0;
	}
	return 0;
}

// #########################
// Utility Functions
char* itoa(int value, char* result, int base)
{
	// check that the base if valid
	if (base < 2 || base > 16) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do
	{
		tmp_value = value;
		value /= base;
		*ptr++ = "fedcba9876543210123456789abcdef" [15 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr)
	{
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

char* utoa(unsigned int value, char* result, int base)
{
	// check that the base if valid
	if (base < 2 || base > 36)
	{
		*result = '\0';
		return result;
	}

	char* ptr = result, *ptr1 = result, tmp_char;
	unsigned int n, tmp_value;

	do
	{
		tmp_value = value;
		value /= base;
		n = (tmp_value - value * base);
		*ptr++ = (n < 10)? '0' + n: ('a' - 10) + n;	//lowercase only.
	} while ( value );

	*ptr-- = '\0';
	while(ptr1 < ptr)
	{
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

void putStr(const char *outStr)
{
	while (*outStr)
		putChar(*outStr++);
}

static inline void putChar(char theChar)
{
	while (!(IFG2 & UCA0TXIFG));	//Wait for transmit buffer ready.
	UCA0TXBUF = theChar;
}


// #########################
// Interrupt Service Routines.
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIA0RX_ISR(void)
{
	switch ( UCA0RXBUF)
	{
	case 'a':							// Receive 'a'.
		gStartAddr = SEGA_HEAD;
		break;
	case 'b':							// Receive 'b'.
		gStartAddr = SEGB_HEAD;
		break;
	case 'c':							// Receive 'c'.
		gStartAddr = SEGC_HEAD;
		break;
	case 'd':							// Receive 'd'.
		gStartAddr = SEGD_HEAD;
		break;
	default:
		gStartAddr = SEGA_HEAD;
		break;
	}
	__bic_SR_register_on_exit(LPM3_bits);
}

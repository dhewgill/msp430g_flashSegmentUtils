/*
 * flash_utils.c
 *
 *  Created on: Oct 31, 2013
 *      Author: Dale Hewgill
 */

#include <stdint.h>
#include <msp430.h>
#include "flash_utils.h"


uint16_t generate_crc(const uint16_t * const begin, const uint16_t * const end)
{
    const uint16_t *p = begin + 1;                      // Begin at word after checksum
    uint16_t chk = 0;                                   // Init checksum
    while(p < end) chk ^= *p++;                         // XOR all words in segment
    return ~chk + 1;									// Return 2's complement.
}

uint8_t compare_crc_segA()
{
	uint16_t *f_ptr = (uint16_t *)SEGA_HEAD;
	uint16_t crc = *f_ptr;	// Existing crc.
	uint16_t calc = generate_crc((uint16_t *)SEGA_HEAD, (uint16_t *)(SEGA_HEAD + SEG_LEN/2));
	__no_operation();
	return (crc == calc);
}


// Will not copy to segA!
// Copies 16bits at a time.
// Only copies whole segments.
void copy_seg_to_seg(uint16_t * pSrc, uint16_t * pDst)
{
	uint16_t * pSrcEnd = pSrc + SEG_LEN/2;

	FCTL2 = FWKEY | FSSEL0 | FN1;		// MCLK/3 for Flash Timing Generator
	FCTL1 = FWKEY | ERASE;				// Set Erase bit
	FCTL3 = FWKEY;						// Clear LOCK  bit

	*pDst = 0x00;						// Erase destination segment

	FCTL1 = FWKEY | WRT;				// Set WRT bit for write operation

	while (pSrc < pSrcEnd)
		*pDst++ = *pSrc++;

	FCTL1 = FWKEY;						// Clear WRT bit
	FCTL3 = FWKEY | LOCK;				// Set LOCK bit
	__no_operation();
}

void copy_segA_to_segB()
{
	uint16_t *f_ptrA = (uint16_t *)SEGA_HEAD;
	uint16_t *f_ptrB = (uint16_t *)SEGB_HEAD;
	uint16_t *f_ptrEnd = f_ptrA + SEG_LEN/2;

	FCTL2 = FWKEY | FSSEL0 | FN1;		// MCLK/3 for Flash Timing Generator
	FCTL1 = FWKEY | ERASE;				// Set Erase bit
	FCTL3 = FWKEY;						// Clear LOCK  bit

	*f_ptrB = 0x00;						// Erase SegmentB

	FCTL1 = FWKEY | WRT;				// Set WRT bit for write operation

	while (f_ptrA < f_ptrEnd)
		*f_ptrB++ = *f_ptrA++;

	FCTL1 = FWKEY;						// Clear WRT bit
	FCTL3 = FWKEY | LOCK;				// Set LOCK bit
	__no_operation();
}


// Will not erase segment A.
void erase_segment(uint16_t * pHead)
{
	FCTL2 = FWKEY | FSSEL0 | FN1;		// MCLK/3 for Flash Timing Generator
	FCTL1 = FWKEY | ERASE;				// Set Erase bit
	FCTL3 = FWKEY;						// Clear LOCK  bit

	*pHead = 0x00;						// Erase Segment

	FCTL1 = FWKEY;						// Clear WRT bit
	FCTL3 = FWKEY | LOCK;				// Set LOCK bit
	__no_operation();
}

// It appears that segX values persist over device reprogramming!
void erase_segB(void)
{
	uint16_t *f_ptrB = (uint16_t *)SEGB_HEAD;

	FCTL2 = FWKEY | FSSEL0 | FN1;		// MCLK/3 for Flash Timing Generator
	FCTL1 = FWKEY | ERASE;				// Set Erase bit
	FCTL3 = FWKEY;						// Clear LOCK  bit

	*f_ptrB = 0x00;						// Erase SegmentB

	FCTL1 = FWKEY;						// Clear WRT bit
	FCTL3 = FWKEY | LOCK;				// Set LOCK bit
	__no_operation();
}
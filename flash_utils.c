/*
 * flash_utils.c
 *
 *  Created on: Oct 31, 2013
 *      Author: Dale Hewgill
 */

#include <stdint.h>
#include <msp430.h>
#include "flash_utils.h"


// Generic/base functions.
uint16_t generate_crc(const uint16_t * const begin, const uint16_t * const end)
{
    const uint16_t *p = begin + 1;                      // Begin at word after checksum
    uint16_t chk = 0;                                   // Init checksum
    while(p < end) chk ^= *p++;                         // XOR all words in segment
    return ~chk + 1;									// Return 2's complement.
}

// Will erase segment A.
void erase_segment(uint16_t * pHead, uint8_t lock)
{
	FCTL2 = FWKEY | FSSEL0 | FN1;			// MCLK/3 for Flash Timing Generator
	FCTL1 = FWKEY | ERASE;					// Set Erase bit
	if (pHead == (uint16_t *)SEGA_HEAD)
		FCTL3 = FWKEY | LOCKA;				// Clear LOCK and LOCKA  bits
	else
		FCTL3 = FWKEY;						// Clear LOCK  bit

	*pHead = 0x00;							// Erase Segment

	if (lock)
	{
		FCTL1 = FWKEY;						// Clear WRT bit
		if (pHead == (uint16_t *)SEGA_HEAD)
			FCTL3 = FWKEY | LOCK;			// Set LOCK bit
		else
			FCTL3 = FWKEY | LOCKA | LOCK;	// Set LOCKA, LOCK bit
	}
	__no_operation();
}

// Copies 16bits at a time.
// Only copies whole segments.
void copy_seg_to_seg(uint16_t * pSrc, uint16_t * pDst)
{
	uint16_t * pSrcEnd = pSrc + SEG_LEN/2;
	erase_segment(pDst, 0);				// Erase destination segment and keep it unlocked.

	FCTL1 = FWKEY | WRT;				// Set WRT bit for write operation

	while (pSrc < pSrcEnd)
		*pDst++ = *pSrc++;

	FCTL1 = FWKEY;						// Clear WRT bit
	FCTL3 = FWKEY | LOCK;				// Set LOCK bit
	__no_operation();
}


// User access functions.

void copy_segA_to_segB()
{
	uint16_t *f_ptrA = (uint16_t *)SEGA_HEAD;
	uint16_t *f_ptrB = (uint16_t *)SEGB_HEAD;

	copy_seg_to_seg(f_ptrA, f_ptrB);	// Copy segmentA to segmentB.
	__no_operation();
}

// It appears that segX values persist over device reprogramming!
void erase_segB(void)
{
	uint16_t *f_ptrB = (uint16_t *)SEGB_HEAD;

	erase_segment(f_ptrB, 1);			// Erase segmentB and lock.
	__no_operation();
}

uint8_t compare_crc_segA()
{
	uint16_t *f_ptr = (uint16_t *)SEGA_HEAD;
	uint16_t crc = *f_ptr;	// Existing crc.
	uint16_t calc = generate_crc((uint16_t *)SEGA_HEAD, (uint16_t *)(SEGA_HEAD + SEG_LEN/2));
	__no_operation();
	return (crc == calc);
}


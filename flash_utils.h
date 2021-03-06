/*
 * flash_utils.h
 *
 *  Created on: Oct 31, 2013
 *      Author: Dale Hewgill
 */

#ifndef MSP430GXX_DUMP_FLASHSEGS_FLASH_UTILS_H_
#define MSP430GXX_DUMP_FLASHSEGS_FLASH_UTILS_H_

#include <stdint.h>

#define SEGA_HEAD	0x10c0
#define SEGB_HEAD	0x1080
#define SEGC_HEAD	0x1040
#define SEGD_HEAD	0x1000
#define SEG_LEN		64				//Data segment length in bytes!  Remember to scale if word access is needed.


//Provided Functions
uint16_t generate_crc(const uint16_t * const begin, const uint16_t * const end);
uint8_t compare_crc_segA();
void copy_seg_to_seg(uint16_t * pSrc, uint16_t * pDst);
void copy_segA_to_segB();
void erase_segment(uint16_t * pHead, uint8_t lock);
void erase_segB(void);


#endif /* MSP430GXX_DUMP_FLASHSEGS_FLASH_UTILS_H_ */

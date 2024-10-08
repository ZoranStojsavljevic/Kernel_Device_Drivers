/*
 * hw_access.h -- prototypes used to access hardware registers
 */

#ifndef _HW_ACCESS_H_
#define _HW_ACCESS_H_

unsigned char read8 ( void *address );
unsigned short read16 ( void *address );
unsigned read32 ( void *address );
void write8 ( void *address, unsigned char data );
void write16 ( void *address, unsigned short data );
void write32 ( void *address, unsigned data );
// unsigned copy16bit_block ( short *da, const short *sa, unsigned n );

#endif // _HW_ACCESS_H_

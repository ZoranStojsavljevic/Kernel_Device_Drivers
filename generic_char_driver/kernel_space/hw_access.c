/*
 * hw_gw.c -- functions used to access the hardware registers
 */

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* printk(), min() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "hw_access.h"		/* local definitions */

unsigned char read8 ( void *address )
{
	return *( ( unsigned char * )( address) );
}

unsigned short read16 ( void *address )
{
	return *( ( unsigned short * )( address ) );
}

unsigned read32 ( void *address )
{
	return *( ( unsigned * )( address ) );
}

void write8 ( void *address, unsigned char data )
{
	*( ( unsigned char * )( address ) ) = data;
	// printk( "Kernel mode: write8: Address [%p], Data %x\n", address, data );
}

void write16 ( void *address, unsigned short data )
{
	*( ( unsigned short * )( address ) ) = data;
	// printk( "Kernel mode write16: Address [%p], Data %4x\n", address, data );
}

void write32 ( void *address, unsigned data )
{
	*( ( unsigned * )( address ) ) = data;
	// printk( "Kernel mode write32: Address [%p], Data %8x\n", address, data );
}

// For now the decision is made to make this copying straight in the main code
#if 0
unsigned copy16bit_block ( short *da, const short *sa, unsigned n )
{
	unsigned	i;

	for ( i=0; i<n; i++ ) da[i] = sa[i];

	return i;
}
#endif

MODULE_LICENSE("GPL");

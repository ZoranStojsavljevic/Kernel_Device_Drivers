/*
 * FILENAME:	mdevx.c
 * AUTHOR:	Zoran Stojsavljevic
 * CREATED:	Y2008
 *
 * Copyright (c) 2008 - 2025 Zoran Stojsavljevic.
 * All rights reserved.
 *
 * GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *
 * DESCRIPTION: This is a standalone TEST application/CLI
 * tool to test drivers.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// 4096 bytes mapped (PHY page size)
#define PHY_PAGE_SIZE 0x1000

// Prototypes
unsigned convert(char* argS, int charFlag, int *count);
char *sgets(char *line, size_t size);
unsigned char HexToBin(char in);
unsigned int hex_to_int(char c);
unsigned int hex_to_ascii(char c, char d);
char *intochar(unsigned int x, char *buf);

/*******************************************************************************
*   function   int main ( int argc, char **argv )
*   author     Zoran Stojsavljevic
*
*   Copyright (c) 2008 - 2025 Zoran Stojsavljevic. All rights reserved.
*
*   GNU General Public License v3.0
*
*   return int32 Error code
*******************************************************************************/
int main ( int argc, char** argv )
{
	unsigned	address = 0;
	int		dataSize, i;
	unsigned	data = 0xfead;
	char		input[32], arg1[32], arg2[32];
	char		*argPtr, *inputPtr;

	static int	mem_fd, numb_char;
	unsigned char	*virtualAddress, *localAddress;
	unsigned	h_address, l_address;
	char		l_option='\0';

	char help_text[] = \
	    "--------------------------------------------------------------------------------\n"
	    " Mdevx Read/Write Utility\n"
	    " Copyright (c) 2008 - 2025 Zoran Stojsavljevic. All rights reserved.\n"
	    "--------------------------------------------------------------------------------\n"
	    " Usage: %s <addr> <data>       (for writing tool expects 2, 4 or 8 nibbles exact)\n"
	    " Usage: %s <addr> <format> <#> (for # (default 1) reading, format is b/h/w)\n"
	    " Usage: %s <addr>              (interactive mode, 8 nibbles exact for <addr>)\n"
	    " Interactive mode: <offset> <data/format> (description for data/format above)\n"
	    " Interactive mode: <cr>             (repeat previous format by entering <cr>)\n"
	    "--------------------------------------------------------------------------------\n";

	if ( (4 < argc) || (2 > argc) ) {
	    printf ( help_text, argv[0], argv[0], argv[0] );
	    return -1;
	}

	argPtr = argv[1];
	address = convert ( argPtr, 0, &numb_char);
	if ( 8 != numb_char ) {
	    printf ( help_text, argv[0], argv[0], argv[0] );
	    return -2;
	}

	// Adjusting high & low addresses to PHY page size (4096 bytes)
	h_address = address & 0xfffff000;
	l_address = address & 0x00000fff;

	if ( 3 > argc ) {
	    if (2 == argc) {
		mem_fd = open ("/dev/mem", O_RDWR);

		if ( -1 == mem_fd ) {
		    printf ( "Cannot open %s for reading/writing.\n", "/dev/mem" );
		    return -3;
		}

		/* Map physical address to user space */
		virtualAddress = localAddress = (unsigned char *) mmap (0, PHY_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, h_address);
		if ( MAP_FAILED == localAddress ) {
		    if (mem_fd >= 0)
			close (mem_fd);
		    printf ( "Cannot access MBAR memory address range.\n" );
		    return -4;
		}

		while(1) {
		    printf ("[base address 0x%x], <offset> <data|b/h/w>? ", h_address);
		    inputPtr = sgets(&input[0], sizeof(input));

		    if ( '\0' == inputPtr[0] ) {
		        l_option = arg2[0];
			if ( 'w' == l_option ) {
			    virtualAddress += 4;
			    address += 4;
			    printf("32 bit data read [%p] = 0x%08x\n", (void *)address, *(unsigned *)virtualAddress);
			}
			else if ( 'h' == l_option ) {
			    virtualAddress +=2;
			    address += 2;
			    printf("16 bit data read [%p] = 0x%04x\n", (void *)address, *(unsigned short *)virtualAddress);
			}
			else if ( 'b' == l_option ) {
			    virtualAddress += 1;
			    address += 1;
			    printf(" 8 bit data read [%p] = 0x%02x\n", (void *)address, *(unsigned char *)virtualAddress);
			}
		    }
		    else {
		        argPtr = arg1;

			while (' ' != *inputPtr) {
			    if ( '\0' == *inputPtr ) {
			        printf("Terminating interactive mode\n");
				munmap(localAddress, PHY_PAGE_SIZE);
				if (mem_fd >= 0)
				    close (mem_fd);
				return 0;
			    }
			    *argPtr++ = *inputPtr++;
			}
			*argPtr = '\0';
			inputPtr++;
			// printf ("Test Printing arg1 %s\n", arg1);

			argPtr = arg2;
			while ('\0' != *inputPtr) {
			    *argPtr++ = *inputPtr++;
			}
			*argPtr = '\0';

			l_address = convert ( arg1, 1, &numb_char);
			virtualAddress = localAddress + l_address;
			address = h_address + l_address;

			l_option = arg2[0];

			if ( 'w' == l_option ) {
			    printf("32 bit data read [%p] = 0x%08x\n", (void *)address, *(unsigned *)virtualAddress);
			}
			else if ( 'h' == l_option ) {
		            printf("16 bit data read [%p] = 0x%04x\n", (void *)address, *(unsigned short *)virtualAddress);
			}
			else if ( 'b' == l_option ) {
			    printf(" 8 bit data read [%p] = 0x%02x\n", (void *)address, *(unsigned char *)virtualAddress);
			}
			else {
			    data = convert ( arg2, 0, &numb_char );
			    if ( 2 == numb_char ) {
			        *(unsigned char *)virtualAddress = data;
				printf(" 8 bit data written [%p] = 0x%02x\n", (void *)address, data);
			    }
			    else if ( 8 == numb_char ) {
			        *(unsigned *)virtualAddress = data;
				printf("32 bit data written [%p] = 0x%08x\n", (void *)address, data);
			    }
			    else if ( 4 == numb_char ) {
			        *(unsigned short *)virtualAddress = data;
				printf("16 bit data written [%p] = 0x%04x\n", (void *)address, data);
			    }
			    else {
			        printf ( help_text, argv[0], argv[0], argv[0] );
				munmap(localAddress, PHY_PAGE_SIZE);
				if (mem_fd >= 0)
				    close (mem_fd);
				return -5;
			    }
			}
		    }
		}
		munmap(localAddress, PHY_PAGE_SIZE);
	    }
	    else {
	        printf ( help_text, argv[0], argv[0], argv[0] );
		return -6;
	    }
	}

	mem_fd = open ("/dev/mem", O_RDWR);

	if ( -1 == mem_fd ) {
	    printf ( "Cannot open %s for reading/writing.\n", "/dev/mem" );
	    return -7;
	}

	/* Map physical address to user space */
	virtualAddress = localAddress = (unsigned char *) mmap (0, PHY_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, h_address);
	if ( MAP_FAILED == localAddress ) {
	    if (mem_fd >= 0)
		close (mem_fd);
	    printf ( "Cannot access MBAR memory address range.\n" );
	    return -8;
	}
	virtualAddress += l_address;

	argPtr = argv[2];
	l_option = argPtr[0];

	if( 3 == argc ) {
	    if ( 'w' == l_option ) {
		printf("32 bit data read [%p] = 0x%08x\n", (void *)address, *(unsigned *)virtualAddress);
	    }
	    else if ( 'h' == l_option ) {
		printf("16 bit data read [%p] = 0x%04x\n", (void *)address, *(unsigned short *)virtualAddress);
	    }
	    else if ( 'b' == l_option ) {
		printf(" 8 bit data read [%p] = 0x%02x\n", (void *)address, *(unsigned char *)virtualAddress);
	    }
	    else {
		data = convert (argPtr, 0, &numb_char);
		if ( 2 == numb_char ) {
		    *(unsigned char *)virtualAddress = data;
		    printf(" 8 bit data written [%p] = 0x%02x\n", (void *)address, data);
		}
		else if ( 8 == numb_char ) {
		    *(unsigned *)virtualAddress = data;
		    printf("32 bit data written [%p] = 0x%08x\n", (void *)address, data);
		}
		else if ( 4 == numb_char ) {
		    *(unsigned short *)virtualAddress = data;
		    printf("16 bit data written [%p] = 0x%04x\n", (void *)address, data);
		}
		else {
		    munmap(localAddress, PHY_PAGE_SIZE);
		    if (mem_fd >= 0)
			close (mem_fd);
		    printf ( help_text, argv[0], argv[0], argv[0] );
		    return -9;
		}
	    }
	}
	else {
	    dataSize = atoi(argv[3]);
	    if (0 == dataSize) {
		munmap(localAddress, PHY_PAGE_SIZE);
		if (mem_fd >= 0)
		    close (mem_fd);
		printf ( help_text, argv[0], argv[0], argv[0] );
		return -10;
	    }
	    if ( 'w' == l_option ) {
		while (0 < dataSize) {
		    printf("\n%p  ", (void *)address); // , *(unsigned *)virtualAddress);
		    for(i=0; i < 4; i++) {
			printf("0x%08x ", *(unsigned *)virtualAddress);
			virtualAddress +=4;
		    }
		    address += 16;
		    dataSize -= 16;
		}
	    }
	    else if ( 'h' == l_option ) {
		while (0 < dataSize) {
		    printf("\n%p  ", (void *)address);
		    for(i=0; i < 4; i++) {
			printf("0x%04x ", *(unsigned short *)virtualAddress);
			virtualAddress +=2;
		    }

		    printf("- ");

		    for(; i < 8; i++) {
			printf("0x%04x ", *(unsigned short *)virtualAddress);
			virtualAddress +=2;
		    }
		    address += 16;
		    dataSize -= 16;
		}
	    }
	    else if ( 'b' == l_option ) {
		char buffer[128];

		while (0 < dataSize) {
		    printf("\n%p  ", (void *)address);
		    for(i=0; i < 8; i++) {
			printf("0x%02x ", *(unsigned char *)virtualAddress);
			intochar(*(unsigned char *)virtualAddress, buffer);
			virtualAddress++;
		    }

		    printf("- ");

		    for(; i < 16; i++) {
			printf("0x%02x ", *(unsigned char *)virtualAddress);
			intochar(*(unsigned char *)virtualAddress, buffer);
			virtualAddress++;
		    }
		    printf("%s", buffer);
		    buffer[0] = '\0';
		    address += 16;
		    dataSize -= 16;
		}
	    }
	    else {
		munmap(localAddress, PHY_PAGE_SIZE);
		if (mem_fd >= 0)
		    close (mem_fd);
		printf ( help_text, argv[0], argv[0], argv[0] );
		return -11;
	    }
	}
	printf("\n\n");

	munmap(localAddress, PHY_PAGE_SIZE);
	if (mem_fd >= 0)
	    close (mem_fd);

	return 0;
}

// Safe version of gets
char *sgets (char *line, size_t size)
{
	size_t	i;

	for (i = 0; i < size - 1; ++i) {
		int ch = fgetc(stdin);
		if (ch == '\n' || ch == EOF)
			break;

		// Added to handle <BS> in command line!
		if ('\b' == ch) {
			if (0 != i)
				i--;
		}
		else line[i] = ch;
	}
	line[i] = '\0';

	return line;
}

unsigned convert (char* argS, int charFlag, int *count)
{
	unsigned	k, j, data = 0;

	if (strstr (argS, "0x")) {        // hex
		for(j=0; '\0'!=argS[j]; j++);
			k = j;

		for(j=2; j<k; j++) {
			data |= HexToBin ( argS[j] ) << ((k-j-1) << 2);
			// printf ("data[%d] = [0x%x]\n", j, data);
		}
		*count = k-2;
	}
	else if (charFlag) {
		data = atoi(argS);
		*count = 8;
	}
	else *count = 0;

	return data;
}

unsigned char HexToBin (char in) {
	if (in >= 'a')
		in -= 'a' - 10;
	else if (in >= 'A')
		in -= 'A' - 10;
	else
		in -= '0';

	return((unsigned char)(in & 0xF));
}

char *intochar(unsigned int x, char *buf) {
	char	*local_buffer = buf;
	char	temp_buffer[8];

	if (32 > x)
		strcat(buf, ".");
	else if (x < 128) {
		sprintf(temp_buffer, "%c", x);
		strcat(buf, temp_buffer);
	}
	else
		strcat(buf, ".");

	return local_buffer;
}

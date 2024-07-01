#include <stdio.h>
#include <stdlib.h>

// Internet Checksum - RFC-1071

unsigned short
checksum(unsigned char buf[], int length)
{
	int i;
	unsigned short word16;
	unsigned int sum32;

	if (length % 2)	// if odd
		return 999;

	sum32 = 0;
	for (i = 0; i < length; i = i + 2) {
		word16 = (buf[i] << 8) + buf[i+1];
		sum32 = sum32 + word16;
		if (sum32 & 0x10000)
			sum32 = (sum32 & 0xffff) + 1;
	}

	sum32 = ~sum32;
	
	return (sum32 & 0xffff);
}

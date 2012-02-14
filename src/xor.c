#include <stdio.h>
#include <stdint.h>

/* xorshift.c
   This is a pseudorandom number generator that employs the XORShift
   method. It is much faster at generating numbers than Linux's
   builtin generator, and has a sizable period -- 1 << 128. */

/* released under the GPLv3 */

static uint32_t rs [4];

void xor_srand (uint32_t seed)
{
	if (!seed)
		rs [0] = 512;
	else
		rs [0] = seed;

	rs [1] = rs [0] << 1;
	rs [2] = rs [1] << 1;
	rs [3] = rs [2] << 1;

	return;
}

uint32_t xor_rand (void)
{
	uint32_t tmp;

	tmp = rs [0] ^ (rs [0] << 4);

	rs [0] = rs [1];
	rs [1] = rs [2];
	rs [2] = rs [3];

	return rs [3] = (rs [3] ^ (rs [3] >> 13)) ^ (tmp ^ (tmp >> 13));
}

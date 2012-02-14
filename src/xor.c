/*
   [dungen]
   [c] 2012 Kyle Davis (tm512), All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.
 
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Any product or service derived from this software, in part or in full, must
      either include the entirety of its source code in its distribution, or have
      the entirety of its source code available for no more than the cost of its
      distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdint.h>

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

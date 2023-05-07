/*
* Code adapted to Metal Shader
* Helio Tejedor <heltena@gmail.com>
*/
/*
* C++ Mersenne Twister wrapper class written by
* Jason R. Blevins <jrblevin@sdf.lonestar.org> on July 24, 2006.
* Based on the original MT19937 C code by
* Takuji Nishimura and Makoto Matsumoto.
*/
/*
A C-program for MT19937, with initialization improved 2002/1/26.
Coded by Takuji Nishimura and Makoto Matsumoto.

Before using, initialize the state by using init_genrand(seed)
or init_by_array(init_key, key_length).

Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The names of its contributors may not be used to endorse or promote
products derived from this software without specific prior written
permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Any feedback is very welcome.
 email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
 */
#include "mt.h"
mt19937::mt19937(void):
    mti_(mt19937_N+1)
{
    init_by_array(0x123, 0x234, 0x345, 0x456);
}
void mt19937::init_by_array(uint ikey0, uint ikey1, uint ikey2, uint ikey3)
{
    const int key_length = 4;
    uint init_key[] = { ikey0, ikey1, ikey2, ikey3 };
    // Store the key array
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (mt19937_N>key_length ? mt19937_N : key_length);
    for (; k; k--) {
        mt_[i] = (mt_[i] ^ ((mt_[i-1] ^ (mt_[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt_[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=mt19937_N) { mt_[0] = mt_[mt19937_N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=mt19937_N-1; k; k--) {
        mt_[i] = (mt_[i] ^ ((mt_[i-1] ^ (mt_[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt_[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=mt19937_N) { mt_[0] = mt_[mt19937_N-1]; i=1; }
    }
    mt_[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}
void mt19937::init_genrand(uint s)
{
    mt_[0]= s & 0xffffffffUL;
    for (mti_=1; mti_<mt19937_N; mti_++) {
        mt_[mti_] =
        (1812433253UL * (mt_[mti_-1] ^ (mt_[mti_-1] >> 30)) + mti_);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt_[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt_[mti_] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}
/*
 * Generates a random number on [0,0xffffffff]-interval
 *
 * \return random number on [0, 0xffffffff]
 */
uint mt19937::genrand_int32(void)
{
    uint y;
    const uint mag01[2] = {0x0UL, mt19937_MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
    if (mti_ >= mt19937_N) { /* generate N words at one time */
        int kk;
        if (mti_ == mt19937_N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */
        for (kk=0;kk<mt19937_N-mt19937_M;kk++) {
            y = (mt_[kk]&mt19937_UPPER_MASK)|(mt_[kk+1]&mt19937_LOWER_MASK);
            mt_[kk] = mt_[kk+mt19937_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<mt19937_N-1;kk++) {
            y = (mt_[kk]&mt19937_UPPER_MASK)|(mt_[kk+1]&mt19937_LOWER_MASK);
            mt_[kk] = mt_[kk+(mt19937_M-mt19937_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_[mt19937_N-1]&mt19937_UPPER_MASK)|(mt_[0]&mt19937_LOWER_MASK);
        mt_[mt19937_N-1] = mt_[mt19937_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        mti_ = 0;
    }
  
    y = mt_[mti_++];
    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}
float mt19937::genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

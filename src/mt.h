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
#pragma once
#include <metal_stdlib>
using namespace metal;
constant const int mt19937_N = 624;
constant const int mt19937_M = 397;
constant const uint mt19937_MATRIX_A = 0x9908b0dfUL;
constant const uint mt19937_UPPER_MASK = 0x80000000UL;
constant const uint mt19937_LOWER_MASK = 0x7fffffffUL;
class mt19937 {
public:
    mt19937();
    void srand(uint u) { init_genrand(u); }
    void srand(float f) { init_genrand(as_type<uint>(f)); }
    uint rand_uint() { return genrand_int32(); }
    float rand(void) { return genrand_real1(); }
    
private:
    void init_genrand(uint s);
    void init_by_array(uint ikey0, uint ikey1, uint ikey2, uint ikey3);
    
    uint genrand_int32(void);
    float genrand_real1(void);
    
    uint mt_[mt19937_N];                  // the state vector
    int mti_;                             // mti == N+1 means mt not initialized 
};

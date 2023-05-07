#ifndef VEC3_H
#define VEC3_H

#include <simd/simd.h>

// Type aliases for vec3
using vec3 = simd::float3;
using point3 = simd::float3; // used for 3D points
using color = simd::float3;  // used for RGB color

// vec3 Utility Functions

// inline std::ostream &operator<<(std::ostream &out, const vec3 &v) {
//   return out << v[0] << ' ' << v[1] << ' ' << v[2];
// }

vec3 my_reflect(thread const vec3 &, thread const vec3 &);
vec3 my_refract(thread const vec3 &, thread const vec3 &, float);
bool near_zero(thread const vec3 &);

#endif

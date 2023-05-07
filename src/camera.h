#ifndef CAMERA_H
#define CAMERA_H

#include <simd/simd.h>

using vec3 = simd::float3;

struct camera {
  vec3 origin;
  vec3 horizontal;
  vec3 vertical;
  vec3 lower_left;
  vec3 u, v, w;
  float lens_radius;
};


#endif

#ifndef MATERIAL_H
#define MATERIAL_H

#include <simd/simd.h>
using color = simd::float3;  // used for RGB color

struct hit_record;

enum material_type { LAMBERTIAN, METAL, DIELECTRIC };

struct material {
  material_type type;
  color albedo;
  float fuzz;
  float ir;
};

#endif

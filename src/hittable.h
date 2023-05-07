#ifndef HITTABLE_H
#define HITTABLE_H

#include <simd/simd.h>
#include "material.h"

enum hittable_type { SPHERE };

// hittable struct
struct hittable {
  hittable_type type;
  simd::float3 pos;
  simd::float3 dimension;
  material mat;
};

#endif

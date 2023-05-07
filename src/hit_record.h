#ifndef HIT_RECORD_H
#define HIT_RECORD_H

#include "vec3.h"
#include "ray.h"
#include "material.h"

struct material;

struct hit_record {
  point3 point;
  vec3 norm;
  device const material *mat_ptr;
  float t;
  bool front_face;

  inline void set_face_normal(thread const ray &r, thread const vec3 &out_norm) {
    front_face = simd::dot(r.direction, out_norm) < 0;
    norm = front_face ? out_norm : -1 * out_norm;
  }
};

#endif

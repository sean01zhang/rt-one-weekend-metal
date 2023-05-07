#include "random.h"
#include <metal_stdlib>

float random_in_range(float min, float max, thread mt19937 & rng) {
  return min + (max - min) * rng.rand();
}

vec3 random_in_unit_sphere(thread mt19937 & rng) {
  while (true) {
    auto p = float3(random_in_range(-1, 1, rng), random_in_range(-1, 1, rng),
                               random_in_range(-1, 1, rng));
    if (metal::length_squared(p) <= 1)
      return p;
  }
}

vec3 random_unit_vector(thread mt19937 & rng) { return simd::normalize(random_in_unit_sphere(rng)); }

vec3 random_in_hemisphere(thread const vec3 &normal, thread mt19937 & rng) {
  vec3 in_unit_sphere = random_in_unit_sphere(rng);
  if (simd::dot(in_unit_sphere, normal) >
      0.0) // In the same hemisphere as the normal
    return in_unit_sphere;
  else
    return -in_unit_sphere;
}

vec3 random_in_unit_disk(thread mt19937 & rng) {
  while (true) {
    auto p = float3(random_in_range(-1, 1, rng), random_in_range(-1, 1, rng), 0);
    return p;
    if (metal::length_squared(p) <= 1)
      return p;
  }
}

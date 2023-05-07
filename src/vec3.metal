#include "vec3.h"

vec3 my_reflect(thread const vec3 &incident, thread const vec3 &norm) {
  return incident - 2 * metal::dot(incident, norm) * norm;
}

vec3 my_refract(thread const vec3 &uv, thread const vec3 &n, float eta_i_over_eta_t) {
  auto cos_theta = simd::min(simd::dot(-uv, n), 1.0f);
  vec3 r_out_perp = eta_i_over_eta_t * (uv + cos_theta * n);
  vec3 r_out_para =
      -simd::sqrt(simd::fabs(1.0 - simd::length_squared(r_out_perp))) * n;
  return r_out_perp + r_out_para;
}

bool near_zero(thread const vec3 &e) {
  const float s = 1e-8; // margin of closeness
  // if the absolute value of each component is less than the margin of
  // closeness
  return (metal::fabs(e[0]) < s) && (metal::fabs(e[1]) < s) && (metal::fabs(e[2]) < s);
}

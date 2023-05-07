#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <simd/simd.h>

// usings
using vec3 = simd::float3;
using point3 = simd::float3; // used for 3D points
using color = simd::float3;  // used for RGB color

// constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// utility functions
inline double deg_to_rad(double degrees) { return degrees * pi / 180.0; }


// clamp a number within a range.
inline double clamp(double x, double min, double max) {
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}

// random doubles
inline double random_double() {
  // thread_local std::mt19937 generator;
  // static std::random_device rd{};
  // generator.seed(rd()); 
  // std::uniform_real_distribution<double> distribution(0.0, 0.9999999999);
  // // Returns a random real in [0,1).
  // return distribution(generator);
  return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
//   thread_local std::mt19937 generator;
//   static std::random_device rd{};
//   generator.seed(rd()); 
//   std::uniform_real_distribution<double> distribution(min, max);
//   // Returns a random real in [0,1).
//   return distribution(generator);
  return min + (max - min) * random_double();
}

inline simd::float3 random_float3(double min, double max) {
  return simd::make_float3(random_double(min, max),
                           random_double(min, max), random_double(min, max));
}

inline simd::float3 random_float3() {
  return simd::make_float3(random_double(), random_double(),
                           random_double());
}

// common headers
// #include "vec3.h"

#endif

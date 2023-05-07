#ifndef RANDOM_H
#define RANDOM_H

#include "vec3.h"
#include "mt.h"

float random_in_range(float min, float max, thread mt19937 & rng);
vec3 random_in_unit_sphere(thread mt19937 & rng);
vec3 random_unit_vector(thread mt19937 & rng);
vec3 random_in_hemisphere(thread const vec3 &, thread mt19937 & rng);
vec3 random_in_unit_disk(thread mt19937 & rng);

#endif

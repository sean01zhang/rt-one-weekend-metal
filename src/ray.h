#ifndef RAY_H
#define RAY_H

#include "vec3.h"

struct ray {
  point3 origin;
  vec3 direction;
};

point3 ray_extend(thread const ray &, float);

#endif

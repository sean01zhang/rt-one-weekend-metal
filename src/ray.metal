#include "ray.h"

point3 ray_extend(thread const ray & r, float t) {
  return r.origin + t * r.direction;
}

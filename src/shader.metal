#include <metal_stdlib>
#include "mt.h"
#include "ray.h"
#include "hit_record.h"
#include "hittable.h"
#include "camera.h"
#include "material.h"
#include "random.h"

using namespace metal;


thread float reflectance(float cosine, float ref_indx) {
  // use schlick approximation for reflectance
  auto r0 = (1 - ref_indx) / (1 + ref_indx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * metal::pow((1 - cosine), 5.0f);
}

thread bool scatter(device const material * mat, thread const ray &r_in, thread const hit_record &rec,
             thread color &attenuation, thread ray &scattered, thread mt19937 & rng) {
  switch (mat->type) {
  case LAMBERTIAN: {

    vec3 scatter_direction = rec.norm + random_unit_vector(rng);

    if (near_zero(scatter_direction)) {
      scatter_direction = rec.norm;
    }

    scattered = ray{.origin = rec.point, .direction = scatter_direction};
    attenuation = mat->albedo;
    return true;
  }
  case METAL: {
    vec3 reflection_direction = my_reflect(simd::normalize(r_in.direction), rec.norm);

    attenuation = mat->albedo;
    scattered = ray{.origin = rec.point,
                    .direction =
                        reflection_direction + mat->fuzz * random_unit_vector(rng)};
    return (simd::dot(scattered.direction, rec.norm) > 0);
  }
  case DIELECTRIC: {
    // attenuation should always be 1 - because the sphere doesn't absorb
    // anything.
    attenuation = float3(1, 1, 1);
    float refraction_ratio = rec.front_face ? (1.0 / mat->ir) : mat->ir;
    vec3 unit_dir = simd::normalize(r_in.direction);

    float cos_theta = simd::fmin(simd::dot(-unit_dir, rec.norm), 1.0f);
    float sin_theta = simd::sqrt(1.0 - cos_theta * cos_theta);

    // if you cannot refract
    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    vec3 direction;
    float random_number = rng.rand();
    if (cannot_refract ||
        reflectance(cos_theta, refraction_ratio) > random_number) {
      direction = my_reflect(unit_dir, rec.norm);
    } else {
      direction = my_refract(unit_dir, rec.norm, refraction_ratio);
    }

    scattered = ray{.origin = rec.point, .direction = direction};
    return true;
  }
  }
}

// hittables check hit
thread bool hittable_hit(device const hittable &h, thread const ray &r, float t_min, float t_max,
                  thread hit_record &rec) {
  switch (h.type) {
  case SPHERE:
    vec3 org_min_c = r.origin - h.pos;
    float a = metal::dot(r.direction, r.direction);
    float h_b = metal::dot(r.direction, org_min_c);
    float c = metal::dot(org_min_c, org_min_c) - h.dimension[0] * h.dimension[0];
    float discriminant = h_b * h_b - a * c;
    if (discriminant < 0) {
      return false;
    } else {
      float sqrtd = metal::sqrt(discriminant);
      float root = (-h_b - sqrtd) / a;
      if (t_min > root || t_max < root) {
        root = (-h_b + sqrtd) / a;
        if (t_min > root || t_max < root) {
          return false;
        }
      }
      rec.t = root;
      rec.point = ray_extend(r, rec.t);
      // rec.norm = normalize(rec.point - center); // this will always be
      // radius
      rec.set_face_normal(r, (rec.point - h.pos) / h.dimension[0]);
      rec.mat_ptr = &(h.mat);
      return true;
    }
    break;
  }
}

bool hittable_list_hit(device const hittable *hittable_list, const size_t len,
                       thread const ray &r, float t_min, float t_max,
                       thread hit_record &rec) {
  hit_record rec_temp;
  bool is_hit = false;
  float closest_hit = t_max;

  for (uint i = 0; i < len; ++i) {
    if (hittable_hit(hittable_list[i], r, t_min, closest_hit, rec_temp)) {
      is_hit = true;
      closest_hit = rec_temp.t;
      rec = rec_temp;
    }
  }

  return is_hit;
}

thread ray get_ray_blur(device camera * cam, float s, float t, thread mt19937 & rng) {
  /* return {.origin=point3(0, 0, 0), .direction=point3(0, 0, 0)}; */
  
  // random point on lens radius
  vec3 rd = cam->lens_radius * random_in_unit_disk(rng);
  // orient lens radius on uv plane.
  vec3 offset = cam->u * rd[0] + cam->v * rd[1];

  return {.origin=cam->origin + offset,
          .direction=cam->lower_left + cam->horizontal * s +
          cam->vertical * t - (cam->origin + offset)};
}

// give it a ray and an object and it'll tell you what to color it
thread color ray_color(thread const ray &r, device hittable * world, int world_length, int depth, thread mt19937 & rng) {

  ray temp_ray = r;
  // base is blue sky portion
  float blue_portion = 0.5 * (normalize(r.direction)[2] + 1.0);
  color final = blue_portion * float3(0.5, 0.7, 1.0) +
         (1 - blue_portion) * float3(1, 1, 1);

  for (int i = 0; i < depth; ++i) {
    hit_record rec;
    if (hittable_list_hit(world, world_length, temp_ray, 0.0001, INFINITY, rec)) {
      color attenuation;
      if (scatter(rec.mat_ptr, temp_ray, rec, attenuation, temp_ray, rng)) {
        final = attenuation * final;
      } else {
        return float3(0, 0, 0);
      }
    } else {
      // hits nothing, return sky.
      return final;
    }
  }

  return float3(0, 0, 0);
}

kernel void trace_image(texture2d<float, access::write> tex [[texture(0)]],
                   device float * seeds [[buffer(0)]],
                   device hittable * world [[buffer(1)]],
                   device int * world_size [[buffer(2)]],
                   device camera * cam [[buffer(3)]],
                   device int * max_depth [[buffer(4)]],
                   device int * samples_per_px [[buffer(5)]],
                   uint2 index [[thread_position_in_grid]]) {
  // seed the rng
  mt19937 mt;
  mt.srand(seeds[tex.get_width() * index[1] + index[0]]);
  /* mt.srand((uint)1); */

  float3 pixelColor = float3(0, 0, 0);

  /* vec3 thing = (random_in_unit_disk(mt) + 1) / 2; */
  /* tex.write(float4(thing[0], thing[1], thing[2], 1), index, 0); */
  /* return; */

  // is this for loop bad?
  for (int s = 0; s < samples_per_px[0]; ++s) {
    float u = (index[0] + mt.rand()) / (tex.get_width() - 1);
    float v = (tex.get_height() - index[1] + mt.rand()) / (tex.get_height() - 1);
    pixelColor += ray_color(
      get_ray_blur(cam, u, v, mt),
      world, world_size[0], max_depth[0],
      mt
    );
  }

  // do I need to square root this?
  // write to the texture
  tex.write(float4(
                   metal::sqrt(pixelColor[0] / samples_per_px[0]),
                   metal::sqrt(pixelColor[1] / samples_per_px[0]),
                   metal::sqrt(pixelColor[2] / samples_per_px[0])
                   , 1), index, 0);

}


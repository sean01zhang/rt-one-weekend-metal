#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "hittable.h"
#include "material.h"
#include "rtweekend.h"
#include "camera.h"

#include <simd/simd.h>


material make_lambertian(color & albedo) {
  return {.type=LAMBERTIAN, .albedo=albedo, .fuzz=0, .ir=0};
}

material make_metal(color & albedo, float fuzz) {
  return {.type=METAL, .albedo=albedo, .fuzz=fuzz, .ir=0};
}

material make_dielectric(float ir) {
  return {.type=DIELECTRIC, .ir=ir, .albedo=0, .fuzz=0};
}

hittable make_sphere(point3 pos, float radius, material mat) {
  return {.type = SPHERE,
          .pos = pos,
          .dimension = simd::make_float3(radius, radius, radius),
          .mat = mat};
}

std::vector<hittable> random_scene() {
  std::vector<hittable> world;

  // generate the ground first.
  material material_ground = {.type = LAMBERTIAN,
                              .albedo = simd::make_float3(0.5, 0.5, 0.5)};
  world.push_back(
      make_sphere(simd::make_float3(0, -1000, 0), 1000, material_ground));

  // generate all the random spheres
  for (int a = -11; a < 11; ++a) {
    for (int b = -11; b < 11; ++b) {
      double choose_mat = random_double();
      double radius = 0.2;
      point3 center = simd::make_float3(a + 0.9 * random_double(), 0.2,
                                        b + 0.9 * random_double());

      if (simd::length(center - simd::make_float3(4, 0.2, 0)) > 0.9) {
        material sphere_material;

        if (choose_mat < 0.8) {
          // diffuse
          auto factor = random_float3();
          auto albedo = random_float3();
          for (int i = 0; i < 3; ++i) {
            albedo[i] *= factor[i];
          }
          sphere_material = make_lambertian(albedo);
          world.push_back(make_sphere(center, radius, sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = random_float3(0.5f, 1.0f);
          auto fuzz = random_double(0, 0.5f);
          sphere_material = make_metal(albedo, fuzz);
          world.push_back(make_sphere(center, radius, sphere_material));
        } else {
          // glass
          sphere_material = make_dielectric(1.5);
          world.push_back(make_sphere(center, radius, sphere_material));
        }
      }
    }
  }

  material material1 = {.type = DIELECTRIC, .ir = 1.5};
  world.push_back(make_sphere(simd::make_float3(0, 1, 0), 1.0, material1));

  // auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  material material2 = {.type = LAMBERTIAN, .albedo = simd::make_float3(0.4, 0.2, 0.1)};
  world.push_back(make_sphere(simd::make_float3(-4, 1, 0), 1.0, material2));

  material material3 = {
      .type = METAL, .albedo = simd::make_float3(0.7, 0.6, 0.5), .fuzz = 0};
  world.push_back(make_sphere(simd::make_float3(4, 1, 0), 1.0, material3));

  return world;
}

inline camera make_camera(point3 lookfrom, point3 lookat, vec3 vup, float vfov, float aspect,
         float aperture, float focus_dist) { // vertical FOV in degrees
    float theta = vfov * pi / 180.0;
    float vp_height = 2.0 * tan(theta / 2);

    float vp_width = vp_height * aspect;

    point3 w = simd::normalize(lookfrom - lookat);
    point3 u = simd::normalize(simd::cross(vup, w));
    point3 v = simd::cross(w, u);

    vec3 horizontal = (vp_width * u) * focus_dist;
    vec3 vertical = (vp_height * v) * focus_dist;
    
    return {
      .w=w,
      .u=u,
      .v=simd::cross(w, u),
      .origin=lookfrom,
      .horizontal=horizontal,
      .vertical=vertical,
      .lower_left=lookfrom - w * focus_dist - horizontal / 2 - vertical / 2,
      .lens_radius=aperture / 2
    };
}

int main() {
  // perf 
  auto start = std::chrono::steady_clock::now();

  // ================================================================================
  // Pipeline Setup
  // create device
  MTL::Device *device = MTL::CreateSystemDefaultDevice();
  NS::Error *error;

  // create command queue
  MTL::CommandQueue *command_queue = device->newCommandQueue();
  // create command buffer
  MTL::CommandBuffer *command_buffer = command_queue->commandBuffer();
  // create command encoder
  MTL::ComputeCommandEncoder *command_encoder =
      command_buffer->computeCommandEncoder();

  // ** Create pipeline state object
  NS::String *libPath =
      NS::String::string("./shader.metallib", NS::UTF8StringEncoding);
  auto default_library = device->newLibrary(libPath, &error);
  if (!default_library) {
    std::cerr << "Failed to load default library.";
    std::exit(-1);
  }

  auto trace_image_function_name =
      NS::String::string("trace_image", NS::ASCIIStringEncoding);
  auto trace_image_function = default_library->newFunction(trace_image_function_name);
  if (!trace_image_function) {
    std::cerr << "failed to find the adder function";
  }

  auto pso = device->newComputePipelineState(trace_image_function, &error);
  // free defualt library and add function
  trace_image_function_name->release();
  default_library->release();
  trace_image_function->release();

  // pass pipeline state object created
  // into the command encoder
  command_encoder->setComputePipelineState(pso);

  
  // ================================================================================ 
  // ** Create data buffers

  // image
  const double aspect = 3.0 / 2.0;
  const double img_width = 800;
  const double img_height = static_cast<int>(img_width / aspect);

  // image quality settings
  const int samples_per_px = 300;
  const int max_depth = 50;

  // world 
  std::vector<hittable> world = random_scene();
  
  // camera
  point3 lookfrom = simd::make_float3(13, 2, 3);
  point3 lookat = simd::make_float3(0, 0, 0);
  vec3 world_up = simd::make_float3(0, 1, 0);

  // set the focus distance to where lookat is.
  double focus_distance = 10.0;
  double aperture = 0.1;

  camera cam = make_camera(lookfrom, lookat, world_up, 20, aspect, aperture, focus_distance);

  // render
  // create buffer for random seeds
  float * seeds = new float[img_width * img_height];
  for (int i = 0; i < img_width * img_height; ++i) {
    seeds[i] = random_double();
  }
  size_t seedsBufferSize = img_width * img_height * sizeof(float);
  MTL::Buffer *seedsBuffer =
    device->newBuffer(seedsBufferSize, MTL::ResourceStorageModeShared);
  memcpy(seedsBuffer->contents(), seeds, seedsBufferSize);

  // create texture for output
  MTL::TextureDescriptor *desc = MTL::TextureDescriptor::alloc()->init();
  desc->setWidth(img_width);
  desc->setHeight(img_height);
  desc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
  desc->setTextureType(MTL::TextureType2D);
  desc->setStorageMode(MTL::StorageModeManaged);
  desc->setUsage(MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
  MTL::Texture *out = device->newTexture(desc);
  desc->release();


  // create buffer to store tracing props
  size_t worldBufferSize = world.size() * sizeof(hittable);
  MTL::Buffer *worldBuffer =
    device->newBuffer(worldBufferSize, MTL::ResourceStorageModeShared);
  memcpy(worldBuffer->contents(), world.data(), worldBufferSize);
    
  MTL::Buffer *worldSizeBuffer =
    device->newBuffer(sizeof(size_t), MTL::ResourceStorageModeShared);
  size_t world_size = world.size();
  memcpy(worldSizeBuffer->contents(), &world_size, sizeof(size_t));

  MTL::Buffer *camBuffer =
    device->newBuffer(sizeof(cam), MTL::ResourceStorageModeShared);
  memcpy(camBuffer->contents(), &cam, sizeof(camera));

  MTL::Buffer *maxDepthBuffer =
    device->newBuffer(sizeof(max_depth), MTL::ResourceStorageModeShared);
  memcpy(maxDepthBuffer->contents(), &max_depth, sizeof(max_depth));

  MTL::Buffer *samplesPerPxBuffer=
    device->newBuffer(sizeof(samples_per_px), MTL::ResourceStorageModeShared);
  memcpy(samplesPerPxBuffer->contents(), &samples_per_px, sizeof(samples_per_px));

  
  // add argument to cmd encoder
  command_encoder->setTexture(out, 0);
  command_encoder->setBuffer(seedsBuffer, 0, 0);
  command_encoder->setBuffer(worldBuffer, 0, 1);
  command_encoder->setBuffer(worldSizeBuffer, 0, 2);
  command_encoder->setBuffer(camBuffer, 0, 3);
  command_encoder->setBuffer(maxDepthBuffer, 0, 4);
  command_encoder->setBuffer(samplesPerPxBuffer, 0, 5);

  // ================================================================================ 
  // set thread count and organization, then run the damn thing
  MTL::Size gridSize = MTL::Size(out->width(), out->height(), 1);

  NS::UInteger threadsPerThreadgroup = pso->maxTotalThreadsPerThreadgroup();
  MTL::Size threadgroupSize(20, 1, 1);

  command_encoder->dispatchThreads(gridSize, threadgroupSize);
  command_encoder->endEncoding();

  // auto sharedCapturer = MTL::CaptureManager::sharedCaptureManager();
  // auto customScope = sharedCapturer->newCaptureScope(device);
  // NS::String *labelName =
  //     NS::String::string("debug pls", NS::UTF8StringEncoding);
  // customScope->setLabel(labelName);
  // sharedCapturer->setDefaultCaptureScope(customScope);

  // customScope->beginScope();
  command_buffer->commit();
  // customScope->endScope();

  // wait for the GPU work is done
  command_buffer->waitUntilCompleted();

  // ================================================================================ 
  // read results from buffer
  std::cout << "P3\n" << img_width << " " << img_height << "\n255\n";

  int bytesPerRow = out->width() * sizeof(simd::uchar4);
  int bytesPerImage = out->height() * bytesPerRow;

  MTL::Region destinationRegion = MTL::Region::Make2D(0, 0, out->width(), out->height());

  simd::uchar4 *pixelBytes = (simd::uchar4 *)malloc(bytesPerImage);
  out->getBytes(pixelBytes, bytesPerRow, destinationRegion, 0);

  // TODO: Check if this is gamma corrected
  int length = out->width() * out->height();
  for (size_t i = 0; i < length; ++i) {
    std::cout << (int)pixelBytes[i][0] << " "
              << (int)pixelBytes[i][1] << " "
              << (int)pixelBytes[i][2] << "\n";
  }
  std::cout << std::endl;
  
  // ================================================================================ 
  // free stuff
  delete[] seeds;
  seedsBuffer->release();
  worldBuffer->release();
  worldSizeBuffer->release();
  camBuffer->release();
  maxDepthBuffer->release();
  samplesPerPxBuffer->release();
  out->release();

  // labelName->release();

  pso->release();
  command_queue->release();
  device->release();

  // finish
  auto finish = std::chrono::steady_clock::now();
  double elapsed_seconds = std::chrono::duration_cast<
    std::chrono::duration<double> >(finish - start).count();

  std::cerr << "Elapsed Time: " << elapsed_seconds << std::endl;

  return 0;
}

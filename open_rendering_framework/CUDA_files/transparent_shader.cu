// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix.h>
#include <optix_math.h>
#include "../random.h"
#include "../structs.h"
#include "../sampler.h"
#include "../LightSampler.h"
#include "../Fresnel.h"
//#define DIFFUSE_PART

using namespace optix;

// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(int, max_depth, , );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );

// Material properties (corresponding to OBJ mtl params)
rtTextureSampler<float4, 2> diffuse_map;
rtDeclareVariable(float, ior, , );
rtDeclareVariable(float3, glass_absorption, , );

// Recursive ray tracing variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
//rtDeclareVariable(unsigned int, radiance_ray_type, , );

// Any hit program for shadows
RT_PROGRAM void any_hit()
{
	// this material is opaque, so it fully attenuates all shadow rays
	prd_shadow.attenuation = 0.0f;
	rtTerminateRay();
}


// Closest hit program for drawing shading normals
RT_PROGRAM void closest_hit()
{
  prd_radiance.result = make_float3(0.0f);
  if(prd_radiance.depth > max_depth) return;
  ++prd_radiance.depth;
  // Initialize variables
  float3 hit_pos = ray.origin + t_hit*ray.direction; 
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); 
  uint& t = prd_radiance.seed;

#ifdef DIFFUSE_PART
  // Diffuse part
  float3 ffnormal = faceforward(normal, -ray.direction, normal);
  float3 rho_d = make_float3(tex2D(diffuse_map, texcoord.x, texcoord.y));
  float prob_d = (rho_d.x + rho_d.y + rho_d.z)/3.0f;
  if(rnd_tea(t) < prob_d)
  {
    float3 new_dir = sample_cosine_weighted(ffnormal, t);
    ++prd_radiance.depth;
    prd_radiance.emit = 0;
    Ray new_ray(hit_pos, new_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
    rtTrace(top_object, new_ray, prd_radiance);
    prd_radiance.result *= rho_d/prob_d;
    return;
  }
#endif

  // Russian roulette with absorption if inside
  float3 beam_T = make_float3(1.0f);
  float n1_over_n2 = 1.0f/ior;
  float cos_theta = dot(-ray.direction, normal);
  if(cos_theta < 0.0f)
  {
    beam_T = expf(-t_hit*glass_absorption);
    float prob = (beam_T.x + beam_T.y + beam_T.z)/3.0f;
    if(rnd_tea(t) >= prob) return;
    beam_T /= prob;
    n1_over_n2 = ior;
    normal = -normal;
    cos_theta = -cos_theta;
  }

  //// Compute Fresnel reflectance (R) and trace compute reflected and refracted directions
  float R = 1.0f;
  float sin_theta_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta*cos_theta);
  float cos_theta_t = 0;
  if(sin_theta_t_sqr < 1.0f)
  {
    cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
    R = fresnel_R(cos_theta, cos_theta_t, n1_over_n2);
  }
  float reflect_xi = rnd_tea(t);


  //prd_radiance.emit = 1;
 
  //Russian Roulette to choose between reflection and refraction
  float3 dir;
  if(reflect_xi < R)
    dir = reflect(ray.direction, normal);
  else
    dir = n1_over_n2*ray.direction + normal*(n1_over_n2*cos_theta - cos_theta_t);

  prd_radiance.emit_light = 1;
  Ray to_trace = make_Ray(hit_pos, dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
  rtTrace(top_object, to_trace, prd_radiance);
  prd_radiance.result *= beam_T;
 
}

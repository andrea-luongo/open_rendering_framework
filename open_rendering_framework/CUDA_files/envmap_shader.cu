#include <optix_world.h>
#include "envmap.h"
#include "structs.h"
#include "sampler.h"

using namespace optix;

#define INDIRECT

// Standard ray variables
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 

// Material properties (corresponding to OBJ mtl params)
rtDeclareVariable(float3, emissive, , );
rtTextureSampler<float4, 2> diffuse_map; 

// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
rtDeclareVariable(unsigned int, shadow_ray_type, , );

#ifdef INDIRECT
// Recursive ray tracing variables
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable(int, max_depth, , );
#endif

// Any hit program for shadows
RT_PROGRAM void any_hit_shadow()
{
  // this material is opaque, so it fully attenuates all shadow rays
  prd_shadow.attenuation = 0.0f;
  rtTerminateRay();
}

// Closest hit program for Lambertian shading using a triangle mesh as an area source.
// This one includes shadows.
RT_PROGRAM void envmap_shader() 
{ 
  float3 hit_pos = ray.origin + t_hit*ray.direction; 
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); 
  float3 ffnormal = faceforward(normal, -ray.direction, normal); 
  float3 rho_d = make_float3(tex2D(diffuse_map, texcoord.x, texcoord.y));
  uint& t = prd_radiance.seed;
  float3 color = prd_radiance.emit ? emissive : make_float3(0.0f);
  
  // Direct illumination
  float3 w_i;
  float3 L_i;
  sample_environment(hit_pos, w_i, L_i, t);

  float cos_theta = dot(ffnormal, w_i);
  if(cos_theta > 0.0)
  { 
    PerRayData_shadow shadow_prd;
    shadow_prd.attenuation = 1.0f;
    Ray shadow_ray = make_Ray(hit_pos, w_i, shadow_ray_type, scene_epsilon, RT_DEFAULT_MAX);
    rtTrace(top_shadower, shadow_ray, shadow_prd);
    float V = shadow_prd.attenuation;
    color += V*L_i*rho_d*M_1_PIf*cos_theta;
  }
#ifdef INDIRECT
  // Indirect illumination
  float prob = (rho_d.x + rho_d.y + rho_d.z)/3.0f;
  if(rnd(t) < prob && prd_radiance.depth < max_depth)
  {
    float3 new_dir = sample_cosine_weighted(ffnormal, t);
    ++prd_radiance.depth;
    prd_radiance.emit = 0;
    Ray new_ray(hit_pos, new_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
    rtTrace(top_object, new_ray, prd_radiance);
    color += rho_d*prd_radiance.result/prob;
  }
#endif
  prd_radiance.result = color; 
}

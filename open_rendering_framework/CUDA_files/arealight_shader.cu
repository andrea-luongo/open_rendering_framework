#include <optix_world.h>
//#include "LightSampler.h"
#include "../structs.h"


using namespace optix;

#define INDIRECT

// Standard ray variables
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, radiance, , );

// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
//rtDeclareVariable(unsigned int, shadow_ray_type, , );


// Any hit program for shadows
RT_PROGRAM void any_hit()
{
  // this material is opaque, so it fully attenuates all shadow rays
  prd_shadow.attenuation = 0.0f;
  rtTerminateRay();
}

// Closest hit program for Lambertian shading using a triangle mesh as an area source.
// This one includes shadows.
RT_PROGRAM void closest_hit() 
{ 
  float3 hit_pos = ray.origin + t_hit*ray.direction; 
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); 
  float3 ffnormal = faceforward(normal, -ray.direction, normal); 
  uint& t = prd_radiance.seed;

  
  float3 result = make_float3(0.0f);
  if (dot(normal, -ray.direction) > 0.0f && prd_radiance.emit_light) {
	  result = radiance;
  }
  
  prd_radiance.result = result; 
}

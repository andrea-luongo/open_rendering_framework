// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "../structs.h"
#include "../Fresnel.h"
#include "../Microfacet.h"
#include "../random.h"
using namespace optix;

// Standard ray variables
rtDeclareVariable(PerRayData_depth, prd_depth, rtPayload, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
//
//// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(rtObject, top_object, , );

rtDeclareVariable(uint, normal_distribution, , );
rtDeclareVariable(uint, microfacet_model, , );
rtDeclareVariable(float2, roughness, , );

// Closest hit program for drawing shading normals
RT_PROGRAM void closest_hit()
{
	float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(normal, -ray.direction, normal);
	uint& t = prd_depth.seed;
	float3 w_i = -ray.direction;
	float3 microfacet_normal;
	float a_x = roughness.x;
	float a_y = roughness.y;
	float z1 = rnd_tea(t);
	float z2 = rnd_tea(t);
	microfacet_sample_visible_normal(w_i, ffnormal, microfacet_normal, a_x, a_y, z1, z2, normal_distribution);

	prd_depth.normal= microfacet_normal;
	prd_depth.ray_depth = t_hit;

}
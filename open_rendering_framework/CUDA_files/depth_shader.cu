// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "../structs.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(PerRayData_depth, prd_depth, rtPayload, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
//
//// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(rtObject, top_object, , );


// Closest hit program for drawing shading normals
RT_PROGRAM void closest_hit()
{
	float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(normal, -ray.direction, normal);

	prd_depth.normal= ffnormal;
	prd_depth.ray_depth = t_hit;

}
// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011


#include <optix_world.h>
#include "../structs.h"
#include "../envmap.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_depth, prd_depth, rtPayload, );

// Variables for shading
rtDeclareVariable(Ray, ray, rtCurrentRay, );

// Miss program returning background color
RT_PROGRAM void miss()
{
    prd_radiance.result = env_lookup(ray.direction);
}

RT_PROGRAM void depth_miss()
{
	prd_depth.ray_depth = RT_DEFAULT_MAX;
	prd_depth.normal = make_float3(-1.0f);
}
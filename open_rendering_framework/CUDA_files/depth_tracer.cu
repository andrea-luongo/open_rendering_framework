// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "../structs.h"
#include "../random.h"

using namespace optix;

// Camera variables
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
//
// Ray generation variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(uint, frame, , );

// Window variables
rtBuffer<float4, 2> output_buffer;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );
// Exception and debugging variables
rtDeclareVariable(float3, exception_color, , );

RT_PROGRAM void depth_tracer()
{
	
	float3 normal = make_float3(0.0f);
	float ray_depth = 0.0f;

	PerRayData_depth prd;
	prd.normal = make_float3(0.0f);
	prd.ray_depth = 0.0f;
	prd.seed = tea<16>(launch_dim.x*launch_index.y + launch_index.x, frame);
	
	float2 jitter = make_float2(rnd_tea(prd.seed), rnd_tea(prd.seed));
	float2 ip_coords = (make_float2(launch_index) + jitter) / make_float2(launch_dim) * 2.0f - 1.0f;
	float3 origin = eye;
	float3 direction = normalize(ip_coords.x*U + ip_coords.y*V + W);
	Ray ray(origin, direction, depth_ray_type, scene_epsilon, RT_DEFAULT_MAX);
	rtTrace(top_object, ray, prd);
	ray_depth += prd.ray_depth;
	normal += (1.0f + prd.normal) * 0.5f;

	if (isfinite(ray_depth))
	{
		float4 curr_sum = (frame != 0) ? output_buffer[launch_index] * ((float)frame) : make_float4(0.0f);
		output_buffer[launch_index] = (make_float4(normal, ray_depth) + curr_sum) / ((float)(frame + 1));
		
	}

	/*float4 curr_sum = (frame != 0) ? output_buffer[launch_index] * ((float)frame) : make_float4(0.0f);
	output_buffer[launch_index] = (make_float4(result, 0.0f) + curr_sum) / ((float)(frame + 1));*/
}

RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_float4(exception_color, 1.0f);

}

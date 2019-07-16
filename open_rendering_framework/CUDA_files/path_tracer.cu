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
//rtDeclareVariable(uint, radiance_ray_type, , );
rtDeclareVariable(uint, frame, , );
rtDeclareVariable(uint2, patch_origin, , );
rtDeclareVariable(uint2, patch_dims, , );
// Window variables
rtBuffer<float4, 2> output_buffer;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );
// Exception and debugging variables
rtDeclareVariable(float3, exception_color, , );

RT_PROGRAM void path_tracer()
{
	float3 result = make_float3(0.0f);

	if (launch_index.x < patch_origin.x || launch_index.x > patch_origin.x + patch_dims.x) {
		output_buffer[launch_index] = (make_float4(result, 1.0f)) ;
		return;
	}
	if(launch_index.y < patch_origin.y || launch_index.y > patch_origin.y+ patch_dims.y) {
		output_buffer[launch_index] = (make_float4(result, 1.0f));
		return;
	}

	PerRayData_radiance prd;
	prd.emit_light = 1;
	prd.depth = 0;
	prd.seed = tea<16>(launch_dim.x*launch_index.y + launch_index.x, frame);
	prd.seed64.seed = make_uint2(tea<16>(launch_dim.x*launch_index.y + launch_index.x, frame), tea<16>(launch_dim.x*launch_index.y + launch_index.x, frame));
	/*prd.seed64.l = tea<16>(launch_dim.x*launch_index.y + launch_index.x, frame);*/
	float2 jitter = make_float2(rnd_tea(prd.seed), rnd_tea(prd.seed));
	float2 ip_coords = (make_float2(launch_index) + jitter) / make_float2(launch_dim) * 2.0f - 1.0f;
	float3 origin = eye;
	float3 direction = normalize(ip_coords.x*U + ip_coords.y*V + W);
	Ray ray(origin, direction, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);

	prd.result = make_float3(0.0f);
;

	rtTrace(top_object, ray, prd);
	result += prd.result;


	if (isfinite(result.x) && isfinite(result.y) && isfinite(result.z))
	{
		float4 curr_sum = (frame != 0) ? output_buffer[launch_index] * ((float)frame) : make_float4(0.0f);
		output_buffer[launch_index] = (make_float4(result, 1.0f) + curr_sum) / ((float)(frame + 1));
	
	}
	//if (launch_index.x == launch_dim.x/2 && launch_index.y == launch_dim.y / 2)
	//	output_buffer[launch_index] = (make_float4(0.0f, 100.0f, 0.0f, 1.0f));
	/*float4 curr_sum = (frame != 0) ? output_buffer[launch_index] * ((float)frame) : make_float4(0.0f);
	output_buffer[launch_index] = (make_float4(result, 0.0f) + curr_sum) / ((float)(frame + 1));*/
}

RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_float4(exception_color, 1.0f);

}

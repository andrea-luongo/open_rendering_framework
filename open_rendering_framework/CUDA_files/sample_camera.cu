// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include <optix.h>
#include <optix_math.h>
#include "../helpers.h"
#include "../random.h"
#include "../structs.h"
#include "../sampler.h"
#include "../Fresnel.h"
#include "../LightSampler.h"
#include "../Microfacet.h"
using namespace optix;
#define GLOBAL
#define RND_64

// Triangle mesh data
rtBuffer<float3> vertex_buffer;
rtBuffer<float3> normal_buffer;
rtBuffer<int3>   vindex_buffer;
rtBuffer<int3>   nindex_buffer;

// Ray generation variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
//rtDeclareVariable(unsigned int, shadow_ray_type, , );
#ifdef GLOBAL
rtDeclareVariable(rtObject, top_object, , );
#endif
// SS properties
rtDeclareVariable(ScatteringMaterialProperties, current_scattering_properties, , );
rtDeclareVariable(float2, current_roughness, , );
rtDeclareVariable(uint, current_microfacet_model, , );
rtDeclareVariable(uint, current_normal_distribution, , );
rtDeclareVariable(uint, current_material_type, , );

// Window variables
rtBuffer<PositionSample> samples_output_buffer;
rtDeclareVariable(uint, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint, current_translucent_obj, , );
rtDeclareVariable(uint, samples, , );
rtDeclareVariable(uint, frame, , );
rtDeclareVariable(Matrix4x4, transform_matrix, , );
rtDeclareVariable(Matrix4x4, normal_matrix, , );

RT_PROGRAM void sample_camera()
{
	uint idx = launch_index + current_translucent_obj*samples;
	PositionSample& sample = samples_output_buffer[idx];

	uint triangles = vindex_buffer.size();
	uint t = tea<16>(idx, frame);
#ifdef RND_64
	Seed64 t64;
	t64.seed = make_uint2(tea<16>(idx, frame), tea<16>(idx, frame));
	uint triangle_id = (int)(rnd_accurate(t64) * triangles);
#else
	uint triangle_id = (int)(rnd_tea(t) * triangles);
#endif

	int3 idx_vxt = vindex_buffer[triangle_id];
	float3 v0 = vertex_buffer[idx_vxt.x];
	float3 v1 = vertex_buffer[idx_vxt.y];
	float3 v2 = vertex_buffer[idx_vxt.z];

	v0 = make_float3(transform_matrix * optix::make_float4(v0, 1.0f));
	v1 = make_float3(transform_matrix * optix::make_float4(v1, 1.0f));
	v2 = make_float3(transform_matrix * optix::make_float4(v2, 1.0f));

	float3 perp_triangle = cross(v1 - v0, v2 - v0);
	float area = 0.5f*length(perp_triangle);
	// sample a point in the triangle

#ifdef RND_64
	float xi1 = sqrt(rnd_accurate(t64));
	float xi2 = rnd_accurate(t64);
#else
	float xi1 = sqrt(rnd_tea(t));
	float xi2 = rnd_tea(t);
#endif
	float u = 1.0f - xi1;
	float v = (1.0f - xi2)*xi1;
	float w = xi1*xi2;
	float3 pos = u*v0 + v*v1 + w*v2;
	sample.pos = u*v0 + v*v1 + w*v2;
	//sample.pos = make_float3(transform_matrix * optix::make_float4(pos, 1.0f));
	float3 n;
	// compute the sample normal
	if (normal_buffer.size() > 0)
	{
		int3 nidx_vxt = nindex_buffer[triangle_id];
		float3 n0 = normal_buffer[nidx_vxt.x];
		float3 n1 = normal_buffer[nidx_vxt.y];
		float3 n2 = normal_buffer[nidx_vxt.z];
		n = normalize(u*n0 + v*n1 + w*n2);
		n = make_float3(normal_matrix * optix::make_float4(n, 0.0f));
		n = normalize(n);
	}
	else {
		n = normalize(perp_triangle);
	}

	sample.normal = n;

	float ior = current_scattering_properties.relative_ior;
	float recip_ior = 1.0f / ior;
	// evaluate incoming light

	float3 Le, w_i;
	float r;
	float cos_theta_i;
	float normal_pdf = 1.0f;
#ifdef GLOBAL 
	float indirect_prob = 0.5f;
	if (rnd(t) < indirect_prob)
	{
#endif

#ifdef RND_64 
	uint light_idx = light_buffer.size()*rnd_accurate(t64);
#else
	uint light_idx = light_buffer.size()*rnd_tea(t);
#endif
	float light_pdf = 1.0f / light_buffer.size();
	LightStruct direct_light = light_buffer[light_idx];
	evaluate_direct_illumination(sample.pos, &direct_light, w_i, Le, r, t);
	sample.dir = w_i;

	if (current_material_type != TRANSLUCENT_SHADER)
	{
		float3 microfacet_normal;
#ifdef RND_64
		float z1 = rnd_accurate(t64);
		float z2 = rnd_accurate(t64);
#else
		float z1 = rnd_tea(t);
		float z2 = rnd_tea(t);
#endif
		float a_x = current_roughness.x;
		float a_y = current_roughness.y;
		if (current_microfacet_model == WALTER_MODEL)
		{
			microfacet_sample_normal(n, microfacet_normal, a_x, z1, z2, current_normal_distribution);
		}
		else
		{
			microfacet_sample_visible_normal(w_i, n, microfacet_normal, a_x, a_y, z1, z2, current_normal_distribution);

		}
		sample.normal = microfacet_normal;
	}

	cos_theta_i = max(dot(w_i, n), 0.0f);
	if (cos_theta_i > 0.0)
	{
		PerRayData_shadow shadow_prd;
		shadow_prd.attenuation = 1.0f;
		Ray shadow_ray(sample.pos, w_i, shadow_ray_type, scene_epsilon, r);
		rtTrace(top_shadower, shadow_ray, shadow_prd);
		Le *= shadow_prd.attenuation* cos_theta_i / light_pdf;
	}
	else
	{
		Le = make_float3(0.0f);
	}
#ifdef GLOBAL
	}
	else
	{
		w_i = sample_cosine_weighted(n, t);

		if (current_material_type != TRANSLUCENT_SHADER)
		{
			float3 microfacet_normal;
#ifdef RND_64
			float z1 = rnd_accurate(t64);
			float z2 = rnd_accurate(t64);
#else
			float z1 = rnd_tea(t);
			float z2 = rnd_tea(t);
#endif
			float a_x = current_roughness.x;
			float a_y = current_roughness.y;
			if (current_microfacet_model == WALTER_MODEL)
			{
				microfacet_sample_normal(n, microfacet_normal, a_x, z1, z2, current_normal_distribution);
			}
			else
			{
				microfacet_sample_visible_normal(w_i, n, microfacet_normal, a_x, a_y, z1, z2, current_normal_distribution);
			}
			sample.normal = microfacet_normal;
		}

		PerRayData_radiance prd_new;
		prd_new.depth = 0;
		prd_new.seed = t;
		prd_new.seed64 = t64;
		Ray new_ray(sample.pos, w_i, radiance_ray_type, scene_epsilon);
		rtTrace(top_object, new_ray, prd_new);
		t = prd_new.seed;
		t64 = prd_new.seed64;
		Le = prd_new.result*M_PIf;
		//cos_theta_i = max(dot(w_i, sample.normal), 0.0f);
	}
	Le /= indirect_prob;
#endif
	cos_theta_i = max(dot(w_i, sample.normal), 0.0f);
	float3 weight = make_float3(1.0f);

//---------------SMOOTH TRANSLUCENT MATERIAL----------------
	if (current_material_type == TRANSLUCENT_SHADER) {
		
		// compute direction of the transmitted lights
		float cos_theta_i_sqr = cos_theta_i*cos_theta_i;
		float sin_theta_t_sqr = recip_ior*recip_ior*(1.0f - cos_theta_i_sqr);
		float cos_theta_t = sqrt(1.0f - sin_theta_t_sqr);
		sample.transmitted = recip_ior*(cos_theta_i*sample.normal - w_i) - sample.normal*cos_theta_t;
		float3 T12 = make_float3(1.0f - fresnel_R(cos_theta_i, cos_theta_t, recip_ior));
		//T12 *= current_scattering_properties.C_phi * 4.0f;
		weight *= T12;
		sample.weight = weight;
		sample.L = Le*(triangles*area);

	} 
//---------------ROUGH TRANSLUCENT MATERIAL----------------
	else if (current_material_type == ROUGH_TRANSLUCENT_SHADER) {

		float a_x = current_roughness.x;
		float a_y = current_roughness.y;
		// compute direction of the transmitted lights
		float cos_theta_i_sqr = cos_theta_i*cos_theta_i;
		float sin_theta_t_sqr = recip_ior*recip_ior*(1.0f - cos_theta_i_sqr);
		float cos_theta_t = sqrt(1.0f - sin_theta_t_sqr);
		sample.transmitted = recip_ior*(cos_theta_i*sample.normal - w_i) - sample.normal*cos_theta_t;
		float3 T12 = make_float3(1.0f - fresnel_R(cos_theta_i, cos_theta_t, recip_ior));
		
		if (current_microfacet_model == WALTER_MODEL) {
			float G_i_m = masking_G1(w_i, sample.normal, n, a_x, a_y, current_normal_distribution);
			float abs_i_m = fabsf(dot(w_i, sample.normal));
			float abs_i_n = fabsf(dot(w_i, n));
			float abs_n_m = fabsf(dot(n, sample.normal));
			float G_o_m = masking_G1(sample.transmitted, sample.normal, n, a_x, a_y, current_normal_distribution);
			weight *= abs_i_m * G_i_m / (abs_i_n * abs_n_m) * G_o_m;
			weight *= T12;
		}
		else if (current_microfacet_model == VISIBLE_NORMALS_MODEL)
		{
			float G_o_m = masking_G1(sample.transmitted, sample.normal, n, a_x, a_y, current_normal_distribution);
			weight *= G_o_m;
			weight *= T12;
		}
		else if (current_microfacet_model == MULTISCATTERING_MODEL) {
			//weight *= microfacet_multiscattering_dielectric_BSDF_eval(w_i, sample.transmitted, n, recip_ior, a_x, a_y, t, 0, current_normal_distribution);
			float G_i_m = masking_G1(w_i, sample.normal, n, a_x, a_y, current_normal_distribution);
			float D = microfacet_eval_visible_normal(w_i, sample.normal, n, a_x, a_y, current_normal_distribution);
			float microfacet_pdf = G_i_m  * D  *fabsf(dot(w_i, sample.normal)) / fabsf(dot(w_i, n));
			weight *=  (T12);

		}
		//rtPrintf("weight %f %f %f \n", weight.x, weight.y, weight.z);
		//weight *= current_scattering_properties.C_phi * 4.0f;	
		sample.weight = weight;
		sample.L = Le*(triangles*area);
	}
}


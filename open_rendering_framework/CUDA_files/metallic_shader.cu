// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix.h>
#include <optix_math.h>
#include "../structs.h"
#include "../Microfacet.h"
#include "../MyComplex.h"
#include "../random.h"


using namespace optix;
// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(int, max_depth, , );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

// Recursive ray tracing variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );
//rtDeclareVariable(unsigned int, shadow_ray_type, , );
//rtDeclareVariable(unsigned int, radiance_ray_type, , );

// Material properties 
rtDeclareVariable(MyComplex3, ior, , );
rtDeclareVariable(float2, roughness, , );
rtDeclareVariable(uint, microfacet_model, , );
rtDeclareVariable(uint, normal_distribution, , );

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
	if (prd_radiance.depth >= max_depth)
	{
		prd_radiance.result = make_float3(0.0f);
		return;
	}
	float3 result = make_float3(0.0f);
	// Compute cosine to angle of incidence
	float3 hit_point = ray.origin + t_hit * ray.direction;
	float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(normal, -ray.direction, normal);
	float3 w_i = -ray.direction;

	float n1 = 1.0f;
	
	MyComplex3 eta = n1 / ior;
	uint& seed = prd_radiance.seed;

	float a_x = roughness.x;
	float a_y = roughness.y;
	if (microfacet_model == MULTISCATTERING_MODEL)
	{
		uint scatteringOrder = 0;
		float3 w_o;
		float3 w_m;
		float3 weight;
		microfacet_multiscattering_conductor_BSDF_sample(w_i, w_o, w_m, normal, eta, a_x, a_y, seed, scatteringOrder, weight, normal_distribution);
		PerRayData_radiance prd_new_ray;
		prd_new_ray.depth = prd_radiance.depth + 1;
		prd_new_ray.result = make_float3(0.0f);
		prd_new_ray.seed = seed;
		prd_new_ray.emit_light = 1;
		optix::Ray new_ray = optix::make_Ray(hit_point, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		rtTrace(top_object, new_ray, prd_new_ray);

		result += prd_new_ray.result * weight;

	}
	else
	{
		float z1 = rnd_tea(seed);
		float z2 = rnd_tea(seed);
		float3 microfacet_normal;
		float G_i_m;
		float G_o_m_refl;
		float G_o_m_refr;
		if (microfacet_model == WALTER_MODEL)
		{
			microfacet_sample_normal(ffnormal, microfacet_normal, z1, z2, a_x, normal_distribution);
		}
		else if (microfacet_model == VISIBLE_NORMALS_MODEL)
		{
			microfacet_sample_visible_normal(w_i, ffnormal, microfacet_normal, a_x, a_y, z1, z2, normal_distribution);
		}


		if (microfacet_model == WALTER_MODEL)
		{
			G_i_m = masking_G1(w_i, microfacet_normal, ffnormal, a_x, normal_distribution);
		}
		else if (microfacet_model == VISIBLE_NORMALS_MODEL)
		{
			G_i_m = masking_G1(w_i, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
		}

		//stop ray tracing if the microfacet is not visible from direction i
		if (G_i_m <= 0.0f) {
			prd_radiance.result = result;
			return;
		}
		// Compute Fresnel reflectance (R) and reflected and refracted rays
		float3 F = fresnel_MyComplex_R(w_i, microfacet_normal, eta);
		float3 w_o =  -w_i + 2.0f*microfacet_normal*dot(w_i, microfacet_normal);

		float abs_i_m = fabsf(dot(w_i, microfacet_normal));
		float abs_i_n = fabsf(dot(w_i, ffnormal));
		float abs_n_m = fabsf(dot(normal, microfacet_normal));
		PerRayData_radiance prd_new_ray;
		prd_new_ray.depth = prd_radiance.depth + 1;
		prd_new_ray.result = make_float3(0.0f);
		prd_new_ray.seed = seed;
		prd_new_ray.emit_light = 1;
		optix::Ray reflected_ray = optix::make_Ray(hit_point, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		float3 weight = F;
		{
			// Trace reflected ray
			if (microfacet_model == WALTER_MODEL)
			{
				G_o_m_refl = masking_G1(w_o, microfacet_normal, ffnormal, a_x, normal_distribution);
			}
			else if (microfacet_model == VISIBLE_NORMALS_MODEL)
			{
				G_o_m_refl = masking_G1(w_o, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
			}

			if (G_o_m_refl <= 0.0f) {
				prd_radiance.result = result;
				return;
			}
			rtTrace(top_object, reflected_ray, prd_new_ray);
			if (microfacet_model == WALTER_MODEL)
			{
				weight *= abs_i_m * G_i_m * G_o_m_refl / (abs_i_n * abs_n_m);
			}
			else if (microfacet_model == VISIBLE_NORMALS_MODEL)
			{
				weight *= G_o_m_refl;
			}

		}
		prd_radiance.seed = prd_new_ray.seed;
		result += prd_new_ray.result * weight;
	}

	prd_radiance.result = result;

}
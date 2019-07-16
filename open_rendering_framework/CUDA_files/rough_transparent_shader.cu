//written by Andrea Luongo

#include <optix.h>
#include <optix_math.h>
#include "../random.h"
#include "../helpers.h"
#include "../structs.h"
#include "../sampler.h"
#include "../Microfacet.h"
#include "../fresnel.h"
#include "../LightSampler.h"


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
rtDeclareVariable(float, ior, , );
rtDeclareVariable(float, roughness, , );
rtDeclareVariable(float3, glass_absorption, , );
rtDeclareVariable(uint, microfacet_model, , );
rtDeclareVariable(uint, normal_distribution, , );

__device__ __inline__ void get_rough_glass_rays(const optix::float3& hit_pos, optix::float3& microfacet_normal, const float& ior1_over_ior2,
	optix::Ray& reflected_ray, optix::Ray& refracted_ray, float& R)
{
	// Compute Fresnel reflectance
	float cos_theta = dot(microfacet_normal, -ray.direction);
	float eta = ior1_over_ior2;
	cos_theta = fabsf(cos_theta);
	float sin_theta_t_sqr = eta*eta*(1.0f - cos_theta*cos_theta);
	float cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
	R = sin_theta_t_sqr < 1.0f ? fresnel_R(cos_theta, cos_theta_t, eta) : 1.0f;

	float3 refr_dir = eta*ray.direction + microfacet_normal*(eta*cos_theta - cos_theta_t);
	refracted_ray = optix::make_Ray(hit_pos, refr_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);

	float3 reflected_dir = reflect(ray.direction, microfacet_normal);
	reflected_ray = optix::make_Ray(hit_pos, reflected_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
}

// Any hit program for shadows
RT_PROGRAM void any_hit()
{
	// this material is opaque, so it fully attenuates all shadow rays
	prd_shadow.attenuation = 0.0f;
	rtTerminateRay();
}


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
	uint& t = prd_radiance.seed;
	optix::Ray refracted_ray, reflected_ray;
	
	float3 beam_T = make_float3(1.0f);
	float cos_theta = dot(-ray.direction, normal);
	if (cos_theta < 0.0f)
	{
		beam_T = expf(-t_hit*glass_absorption);
		float prob = (beam_T.x + beam_T.y + beam_T.z) / 3.0f;
		if (rnd_tea(t) >= prob) return;
		beam_T /= prob;
	}

	float z1 = rnd_tea(t);
	float z2 = rnd_tea(t);
	float3 microfacet_normal;
	float microfacet_D;
	float G_i_m;
	float G_o_m_refl;
	float G_o_m_refr;
	if (microfacet_model == MULTISCATTERING_MODEL) 
	{
		float eta = 1.0f / ior;
		float a_x = roughness;
		float a_y = roughness;
		uint scatteringOrder = 0;
		float3 w_o;
		float3 weight = make_float3(1.0f);

		microfacet_multiscattering_dielectric_BSDF_sample(w_i, w_o, normal, eta, a_x, a_y, t, scatteringOrder, weight, normal_distribution);

		PerRayData_radiance prd_new_ray;
		prd_new_ray.depth = prd_radiance.depth + 1;
		prd_new_ray.result = make_float3(0.0f);
		prd_new_ray.seed = t;
		prd_new_ray.emit_light = 1;
		optix::Ray new_ray = optix::make_Ray(hit_point, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		rtTrace(top_object, new_ray, prd_new_ray);
		prd_radiance.seed = prd_new_ray.seed;
		result += prd_new_ray.result * weight;
	}
	else
	{
		if (microfacet_model == WALTER_MODEL)
		{
			microfacet_sample_normal(ffnormal, microfacet_normal, z1, z2, roughness, normal_distribution);
		}
		else if (microfacet_model == VISIBLE_NORMALS_MODEL)
		{
			microfacet_sample_visible_normal(w_i, ffnormal, microfacet_normal, roughness, roughness, z1, z2, normal_distribution);
		}
		// Compute relative index of refraction
		float cos_theta_in = dot(normal, w_i);
		float ior1_over_ior2;
		if (cos_theta_in < 0.0f) {
			ior1_over_ior2 = ior;
		}
		else {
			ior1_over_ior2 = 1.0f / ior;
		}

		if (microfacet_model == WALTER_MODEL)
		{
			G_i_m = masking_G1(w_i, microfacet_normal, ffnormal, roughness, normal_distribution);
		}
		else if (microfacet_model == VISIBLE_NORMALS_MODEL)
		{
			G_i_m = masking_G1(w_i, microfacet_normal, ffnormal, roughness, roughness, normal_distribution);
		}

		//stop ray tracing if the microfacet is not visible from direction i
		if (G_i_m <= 0.0f) {
			prd_radiance.result = result;
			return;
		}
		// Compute Fresnel reflectance (R) and reflected and refracted rays
		float R;
		get_rough_glass_rays(hit_point, microfacet_normal, ior1_over_ior2, reflected_ray, refracted_ray, R);
		float abs_i_m = fabsf(dot(w_i, microfacet_normal));
		float abs_i_n = fabsf(dot(w_i, ffnormal));
		float abs_n_m = fabsf(dot(normal, microfacet_normal));
		float russian_roulette_seed = rnd_tea(prd_radiance.seed);
		PerRayData_radiance prd_new_ray;
		prd_new_ray.depth = prd_radiance.depth + 1;
		prd_new_ray.result = make_float3(0.0f);
		prd_new_ray.seed = t;
		prd_new_ray.emit_light = 1;
		float weight = 1.0f;
		//Russian Roulette to choose between reflection and refraction
		if (russian_roulette_seed > R) {
			if (microfacet_model == WALTER_MODEL)
			{
				G_o_m_refr = masking_G1(refracted_ray.direction, microfacet_normal, ffnormal, roughness, normal_distribution);
			}
			else if (microfacet_model == VISIBLE_NORMALS_MODEL)
			{
				G_o_m_refr = masking_G1(refracted_ray.direction, microfacet_normal, ffnormal, roughness, roughness, normal_distribution);
			}

			if (G_o_m_refr <= 0.0f) {
				prd_radiance.result = result;
				return;
			}
			rtTrace(top_object, refracted_ray, prd_new_ray);
			if (microfacet_model == WALTER_MODEL)
			{
				weight = abs_i_m * G_i_m * G_o_m_refr / (abs_i_n * abs_n_m);
			}
			else if (microfacet_model == VISIBLE_NORMALS_MODEL)
			{
				weight = G_o_m_refr;
			}

		}
		else {
			// Trace reflected ray
			if (microfacet_model == WALTER_MODEL)
			{
				G_o_m_refl = masking_G1(reflected_ray.direction, microfacet_normal, ffnormal, roughness, normal_distribution);
			}
			else if (microfacet_model == VISIBLE_NORMALS_MODEL)
			{
				G_o_m_refl = masking_G1(reflected_ray.direction, microfacet_normal, ffnormal, roughness, roughness, normal_distribution);
			}

			if (G_o_m_refl <= 0.0f) {
				prd_radiance.result = result;
				return;
			}
			rtTrace(top_object, reflected_ray, prd_new_ray);
			if (microfacet_model == WALTER_MODEL)
			{
				weight = abs_i_m * G_i_m * G_o_m_refl / (abs_i_n * abs_n_m);
			}
			else if (microfacet_model == VISIBLE_NORMALS_MODEL)
			{
				weight = G_o_m_refl;
			}

		}
		prd_radiance.seed = prd_new_ray.seed;
		result += prd_new_ray.result * weight;
	}

	prd_radiance.result = result*beam_T;
}

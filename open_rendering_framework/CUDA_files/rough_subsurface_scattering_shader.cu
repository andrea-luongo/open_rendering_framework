// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#define REFLECT
#define TRANSMIT
#define DIRECTLIGHT
#define RND_64
#include <optix.h>
#include <optix_math.h>
#include "../helpers.h"
#include "../random.h"
#include "../dipoles/directional_dipole.h"
#include "../dipoles/standard_dipole.h"
#include "../Fresnel.h"
#include "../structs.h"
#include "../Microfacet.h"
#include "../LightSampler.h"
using namespace optix;

// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );
rtDeclareVariable(int, max_depth, , );

// SS properties
rtDeclareVariable(ScatteringMaterialProperties, scattering_properties, , );

// Variables for shading
rtBuffer<PositionSample> samples_output_buffer;
rtDeclareVariable(uint, translucent_index, , );
rtDeclareVariable(uint, samples, , );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(uint, dipole_model, , );
rtDeclareVariable(uint, normal_distribution, , );
rtDeclareVariable(uint, microfacet_model, , );
rtDeclareVariable(float2, roughness, , );


#if defined REFLECT || defined TRANSMIT
// Recursive ray tracing variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
//rtDeclareVariable(unsigned int, radiance_ray_type, , );
#endif

#if defined DIRECTLIGHT
rtDeclareVariable(rtObject, top_shadower, , );
//rtDeclareVariable(unsigned int, shadow_ray_type, , );
#endif

// Any hit program for shadows
RT_PROGRAM void any_hit()
{
	// this material is opaque, so it fully attenuates all shadow rays
	prd_shadow.attenuation = 0.0f;
	rtTerminateRay();
}

// Closest hit program for Lambertian shading using the basic light as a directional source
RT_PROGRAM void closest_hit()
{
	prd_radiance.result = make_float3(0.0f);
	if (prd_radiance.depth > max_depth) return;
	float3 result = make_float3(0.0f);

	float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(normal, -ray.direction, normal);
	float3 hit_pos = ray.origin + t_hit*ray.direction;
	float3 w_i = -ray.direction;

	ScatteringMaterialProperties& props = scattering_properties;
	float recip_ior = 1.0f / props.relative_ior;
	float3 beam_T = make_float3(1.0f);
	uint& t = prd_radiance.seed;
	Seed64& t64 = prd_radiance.seed64;
	//
#ifdef RND_64
	float reflect_xi = rnd_accurate(t64);
	float z1 = rnd_accurate(t64);
	float z2 = rnd_accurate(t64);
#else
	float reflect_xi = rnd_tea(t);
	float z1 = rnd_tea(t);
	float z2 = rnd_tea(t);
#endif

	float3 microfacet_normal;
	float a_x = roughness.x;
	float a_y = roughness.y;
	if (microfacet_model == WALTER_MODEL)
	{
		microfacet_sample_normal(ffnormal, microfacet_normal, a_x, z1, z2, normal_distribution);
	}
	else
	{
		microfacet_sample_visible_normal(w_i, ffnormal, microfacet_normal, a_x, a_y, z1, z2, normal_distribution);
	}
	//microfacet_normal = ffnormal;
	float cos_theta_i = dot(w_i, microfacet_normal);
	bool inside = dot(w_i, normal) < 0.0f;

#ifdef DIRECTLIGHT

#ifdef RND_64
	uint light_idx = light_buffer.size()*rnd_accurate(t64);
#else
	uint light_idx = light_buffer.size()*rnd_tea(t);
#endif

	float light_pdf = 1.0f / light_buffer.size();
	LightStruct direct_light = light_buffer[light_idx];

	float dist;
	float3 light_radiance;
	float3 w_l = make_float3(0.0f);
	float cos_theta_l = 0.0f;
	evaluate_direct_illumination(hit_pos, &direct_light, w_l, light_radiance, dist, t);
	light_radiance /= light_pdf;
	cos_theta_l = dot(ffnormal, w_l);

	if (!inside && cos_theta_l > 0.0)
	{
		float V = 1.0f;
		PerRayData_shadow shadow_prd;
		shadow_prd.attenuation = 1.0f;
		Ray shadow_ray(hit_pos, w_l, shadow_ray_type, scene_epsilon, dist);
		rtTrace(top_shadower, shadow_ray, shadow_prd);
		V = shadow_prd.attenuation;
		if (V > 0.0f)
		{
			float3 Li = V*light_radiance;
			if (microfacet_model == MULTISCATTERING_MODEL)
			{
				MyComplex eta = MyComplex{ recip_ior, 0 };
				MyComplex3 eta3 = { eta, eta, eta };
				float reflected_brdf = 0.0f;
				reflected_brdf = microfacet_multiscattering_dielectric_BSDF_eval(w_i, w_l, ffnormal, recip_ior, a_x, a_y, t, 0, normal_distribution);
				result += Li * reflected_brdf;
				prd_radiance.emit_light = 0;
			}
			else
			{
				//
				float3 refl_normal = normalize(w_i + w_l);
				float cos_theta_refl = dot(refl_normal, w_l);
				//// Compute Fresnel reflectance (R) 
				float R_refl = 1.0f;
				float sin_theta_refl_t_sqr = recip_ior*recip_ior*(1.0f - cos_theta_refl*cos_theta_refl);
				float cos_theta_refl_t = 0;
				if (sin_theta_refl_t_sqr < 1.0f)
				{
					cos_theta_refl_t = sqrtf(1.0f - sin_theta_refl_t_sqr);
					R_refl = fresnel_R(cos_theta_refl, cos_theta_refl_t, recip_ior);
				}

				float G_i_m_refl = masking_G1(w_i, refl_normal, ffnormal, a_x, a_y, normal_distribution);
				float G_o_m_refl = masking_G1(w_l, refl_normal, ffnormal, a_x, a_y, normal_distribution);
				float D_refl = microfacet_eval_visible_normal(w_i, refl_normal, ffnormal, a_x, a_y, normal_distribution);
				float weight_refl = G_i_m_refl * G_o_m_refl * D_refl / (4.0f * fabsf(dot(w_i, ffnormal) * dot(w_l, ffnormal)));
				result += Li* R_refl * weight_refl;
				prd_radiance.emit_light = 1;
			}
		}
	}
#endif
	//
	if (microfacet_model == WALTER_MODEL || microfacet_model == VISIBLE_NORMALS_MODEL)
	{
#ifdef TRANSMIT

		if (inside)
		{
			if (dipole_model == DIRECTIONAL_DIPOLE) {
				beam_T = expf(-t_hit*props.deltaEddExtinction);
			}
			else if (dipole_model == STANDARD_DIPOLE) {
				beam_T = expf(-t_hit*props.extinction);
			}
			float prob = (beam_T.x + beam_T.y + beam_T.z) / 3.0f;
			//

#ifdef RND_64
			if (rnd_accurate(t64) >= prob) return;
#else
			if (rnd_tea(t) >= prob) return;
#endif
			beam_T /= prob;
			recip_ior = props.relative_ior;
			normal = -normal;
			//microfacet_normal = -microfacet_normal;
			//cos_theta_i = -cos_theta_i;
		}

		float sin_theta_t_sqr = recip_ior*recip_ior*(1.0f - cos_theta_i*cos_theta_i);
		float cos_theta_t = 1.0f;
		float R = 1.0f;
		if (sin_theta_t_sqr < 1.0f)
		{
			cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
			R = fresnel_R(cos_theta_i, cos_theta_t, recip_ior);
		}
		if (reflect_xi >= R)
		{
			float3 w_t = recip_ior*(cos_theta_i*microfacet_normal - w_i) - microfacet_normal*cos_theta_t;
			PerRayData_radiance prd_refracted;
			prd_refracted.depth = prd_radiance.depth + 1;
			prd_refracted.seed = t;
			prd_refracted.seed64 = t64;
			prd_refracted.result = make_float3(0.0f);
			prd_refracted.emit_light = 1;
			Ray refracted(hit_pos, w_t, radiance_ray_type, scene_epsilon);
			rtTrace(top_object, refracted, prd_refracted);
			float3 weight = make_float3(1.0f);
			if (microfacet_model == WALTER_MODEL) {
				float G_i_m = masking_G1(w_i, microfacet_normal, ffnormal, a_x, normal_distribution);
				float abs_i_m = fabsf(dot(w_i, microfacet_normal));
				float abs_i_n = fabsf(dot(w_i, ffnormal));
				float abs_n_m = fabsf(dot(ffnormal, microfacet_normal));
				float G_o_m_refr = masking_G1(w_t, microfacet_normal, ffnormal, a_x, normal_distribution);
				weight *= abs_i_m * G_i_m *G_o_m_refr / (abs_i_n * abs_n_m);
			}
			else {
				float G_o_m_refr = masking_G1(w_t, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
				weight *= G_o_m_refr;
			}
			result += prd_refracted.result * weight;
			t = prd_refracted.seed;
			t64 = prd_refracted.seed64;
			if (!inside)
			{
#else
		float cos_theta_i = dot(w_i, microfacet_normal);
		float R = fresnel_R(cos_theta_i, recip_ior);
#endif

		//float chosen_transport_rr = props.mean_transport;
		float chosen_transport_rr = fminf(props.transport.x, fminf(props.transport.y, props.transport.z));
		float3 accumulate = make_float3(0.0f);
		uint N = samples_output_buffer.size();
		N = samples;
		for (uint i = 0; i < N; ++i)
		{
			PositionSample& sample = samples_output_buffer[i + translucent_index*N];
			float3 T12 = sample.weight;
			float3 w12 = sample.transmitted;
			// compute contribution if sample is non-zero
			if (dot(sample.L, sample.L) > 0.0f)
			{
				// Russian roulette
				float dist = length(hit_pos - sample.pos);
				float exp_term = exp(-dist * chosen_transport_rr);
				//exp_term = fmaxf(exp_term, 0.000001f);
#ifdef RND_64
				float rnd_number = rnd_accurate(t64);
#else
				float rnd_number = rnd_tea(t);
#endif
				if (rnd_number < exp_term && rnd_number > 0.001)
				{
					if (dipole_model == DIRECTIONAL_DIPOLE) {
						accumulate += T12*sample.L*dirpole_bssrdf(sample.pos, sample.normal, w12, hit_pos, normal, props) / exp_term * weight;
					}
					else if (dipole_model == STANDARD_DIPOLE) {
						accumulate += T12*sample.L*dipole_bssrdf(dist, props) / exp_term * weight;
					}
				}
				else {
					//rtPrintf("no dipole \n");
				}
			}
		}
#ifdef TRANSMIT
		result += accumulate*props.global_coeff / (float)N;
			}
		}
#else
		float T21 = 1.0f - R;
		prd_radiance.result += T21*accumulate*props.global_coeff / (float)N;
#endif

#ifdef REFLECT
		// Trace reflected ray
		if (reflect_xi < R)
		{
			float3 w_r = 2.0f*cos_theta_i*microfacet_normal - w_i;
			PerRayData_radiance prd_reflected;
			prd_reflected.depth = prd_radiance.depth + 1;
			prd_reflected.seed = t;
			prd_reflected.seed64 = t64;
			prd_reflected.result = make_float3(0.0f);
			prd_reflected.emit_light = prd_radiance.emit_light;
			Ray reflected(hit_pos, w_r, radiance_ray_type, scene_epsilon);
			rtTrace(top_object, reflected, prd_reflected);



			float3 weight = make_float3(1.0f);
			if (microfacet_model == WALTER_MODEL) {
				float G_i_m = masking_G1(w_i, microfacet_normal, ffnormal, a_x, normal_distribution);
				float abs_i_m = fabsf(dot(w_i, microfacet_normal));
				float abs_i_n = fabsf(dot(w_i, ffnormal));
				float abs_n_m = fabsf(dot(ffnormal, microfacet_normal));
				float G_o_m_refl = masking_G1(w_r, microfacet_normal, ffnormal, a_x, normal_distribution);
				weight *= abs_i_m * G_i_m *G_o_m_refl / (abs_i_n * abs_n_m);
			}
			else {
				float G_o_m_refl = masking_G1(w_r, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
				weight *= G_o_m_refl;
			}

			result += prd_reflected.result * weight;
			t = prd_reflected.seed;
			t64 = prd_reflected.seed64;
		}
#endif



		prd_radiance.seed = t;
		prd_radiance.seed64 = t64;
		prd_radiance.result = result;
		prd_radiance.result *= beam_T;
	}
	else if (microfacet_model == MULTISCATTERING_MODEL)
	{

#ifdef TRANSMIT



		uint scatteringOrder = 0;
		float3 w_o;
		float3 weight = make_float3(1.0f);
		float3 w_m;
		microfacet_multiscattering_dielectric_BSDF_sample(w_i, w_o, normal, 1.0f / props.relative_ior, a_x, a_y, t, scatteringOrder, weight, normal_distribution);

		bool is_refraction = dot(w_o, ffnormal) < 0.0f;

		if (inside)
		{
			if (dipole_model == DIRECTIONAL_DIPOLE) {
				beam_T = expf(-t_hit*props.deltaEddExtinction);
			}
			else if (dipole_model == STANDARD_DIPOLE) {
				beam_T = expf(-t_hit*props.extinction);
			}
			float prob = (beam_T.x + beam_T.y + beam_T.z) / 3.0f;
#ifdef RND_64
			if (rnd_accurate(t64) >= prob) return;
#else
			if (rnd_tea(t) >= prob) return;
#endif
			beam_T /= prob;
			recip_ior = props.relative_ior;
			normal = -normal;
		}

		if (is_refraction)
		{
			PerRayData_radiance prd_new_ray;
			prd_new_ray.depth = prd_radiance.depth + 1;
			prd_new_ray.result = make_float3(0.0f);
			prd_new_ray.seed = t;
			prd_new_ray.emit_light = 1;
			prd_new_ray.seed64 = t64;
			optix::Ray new_ray = optix::make_Ray(hit_pos, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, new_ray, prd_new_ray);
			prd_radiance.seed = prd_new_ray.seed;
			result += prd_new_ray.result * weight;
			t = prd_new_ray.seed;
			t64 = prd_new_ray.seed64;

			if (!inside)
			{
#else
		float cos_theta_i = dot(w_i, microfacet_normal);
		float R = fresnel_R(cos_theta_i, recip_ior);
#endif

		//float chosen_transport_rr = props.mean_transport;
		float chosen_transport_rr = fminf(props.transport.x, fminf(props.transport.y, props.transport.z));
		float3 accumulate = make_float3(0.0f);
		uint N = samples_output_buffer.size();
		N = samples;
		for (uint i = 0; i < N; ++i)
		{
			PositionSample& sample = samples_output_buffer[i + translucent_index*N];
			float3 T12 = sample.weight;
			float3 w12 = sample.transmitted;
			// compute contribution if sample is non-zero
			if (dot(sample.L, sample.L) > 0.0f)
			{
				// Russian roulette
				float dist = length(hit_pos - sample.pos);
				float exp_term = exp(-dist * chosen_transport_rr);
				//exp_term = fmaxf(exp_term, 0.000001f);
#ifdef RND_64
				float rnd_number = rnd_accurate(t64);
#else
				float rnd_number = rnd_tea(t);
#endif
				if (rnd_number < exp_term && rnd_number > 0.001)
				{
					if (dipole_model == DIRECTIONAL_DIPOLE) {
						accumulate += T12*sample.L*dirpole_bssrdf(sample.pos, sample.normal, w12, hit_pos, normal, props) / exp_term * weight;
					}
					else if (dipole_model == STANDARD_DIPOLE) {
						accumulate += T12*sample.L*dipole_bssrdf(dist, props) / exp_term * weight;
					}
				}
				else {
					//rtPrintf("no dipole \n");
				}
			}
		}
#ifdef TRANSMIT
		result += accumulate*props.global_coeff / (float)N;
			}
		}
#else
		float T21 = 1.0f - R;
		prd_radiance.result += T21*accumulate*props.global_coeff / (float)N;
#endif

#ifdef REFLECT
		// Trace reflected ray
		else
		{


			PerRayData_radiance prd_new_ray;
			prd_new_ray.depth = prd_radiance.depth + 1;
			prd_new_ray.result = make_float3(0.0f);
			prd_new_ray.seed = t;
			prd_new_ray.seed64 = t64;
			prd_new_ray.emit_light = 1;
			optix::Ray new_ray = optix::make_Ray(hit_pos, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, new_ray, prd_new_ray);
			prd_radiance.seed = prd_new_ray.seed;
			result += prd_new_ray.result * weight;
			t = prd_new_ray.seed;
			t64 = prd_new_ray.seed64;
		}
#endif

		prd_radiance.seed = t;
		prd_radiance.seed64 = t64;
		prd_radiance.result = result;
		prd_radiance.result *= beam_T;
	}
}


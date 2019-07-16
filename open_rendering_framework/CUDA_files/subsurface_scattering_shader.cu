// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#define REFLECT
#define TRANSMIT

#define RND_64

#include <optix.h>
#include <optix_math.h>
#include "../helpers.h"
#include "../random.h"
#include "../dipoles/directional_dipole.h"
#include "../dipoles/standard_dipole.h"
#include "../Fresnel.h"
#include "../structs.h"

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

#if defined REFLECT || defined TRANSMIT
// Recursive ray tracing variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
//rtDeclareVariable(unsigned int, radiance_ray_type, , );
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

	float3 no = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 xo = ray.origin + t_hit*ray.direction;
	float3 wo = -ray.direction;
	ScatteringMaterialProperties& props = scattering_properties;
	float recip_ior = 1.0f / props.relative_ior;
	float3 beam_T = make_float3(1.0f);
	uint& t = prd_radiance.seed;
	Seed64& t64 = prd_radiance.seed64;
	//
#ifdef RND_64
	float reflect_xi = rnd_accurate(t64);
#else
	float reflect_xi = rnd_tea(t);
#endif
	//
#ifdef TRANSMIT
	float cos_theta_o = dot(wo, no);
	bool inside = cos_theta_o < 0.0f;
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
		no = -no;
		cos_theta_o = -cos_theta_o;
	}
	float sin_theta_t_sqr = recip_ior*recip_ior*(1.0f - cos_theta_o*cos_theta_o);
	float cos_theta_t = 1.0f;
	float R = 1.0f;
	if (sin_theta_t_sqr < 1.0f)
	{
		cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
		R = fresnel_R(cos_theta_o, cos_theta_t, recip_ior);
	}
	if (reflect_xi >= R)
	{
		float3 wt = recip_ior*(cos_theta_o*no - wo) - no*cos_theta_t;
		PerRayData_radiance prd_refracted;
		prd_refracted.depth = prd_radiance.depth + 1;
		prd_refracted.seed64 = t64;
		prd_refracted.seed = t;
		prd_refracted.result = make_float3(0.0f);
		prd_refracted.emit_light = 1;
		Ray refracted(xo, wt, radiance_ray_type, scene_epsilon);
		rtTrace(top_object, refracted, prd_refracted);
		prd_radiance.result += prd_refracted.result;
		t = prd_refracted.seed;
		t64 = prd_refracted.seed64;
		if (!inside)
		{
#else
	float cos_theta_o = dot(wo, no);
	float R = fresnel_R(cos_theta_o, recip_ior);
#endif

	//float chosen_transport_rr = props.mean_transport;
	float chosen_transport_rr = fminf(props.transport.x, fminf(props.transport.y, props.transport.z));
	float3 accumulate = make_float3(0.0f);
	uint N = samples_output_buffer.size();
	N = samples;
	for (uint i = 0; i < N; ++i)
	{
		PositionSample& sample = samples_output_buffer[i + translucent_index*N];

		// compute direction of the transmitted light
		const float3& wi = sample.dir;
		float cos_theta_i = max(dot(wi, sample.normal), 0.0f);
		float cos_theta_i_sqr = cos_theta_i*cos_theta_i;
		float sin_theta_t_sqr = recip_ior*recip_ior*(1.0f - cos_theta_i_sqr);
		float cos_theta_t = sqrt(1.0f - sin_theta_t_sqr);
	    //float3 w12 = recip_ior*(cos_theta_i*sample.normal - wi) - sample.normal*cos_theta_t;
		//float T12 = 1.0f - fresnel_R(cos_theta_i, cos_theta_t, recip_ior);
		float3 T12 = sample.weight;
		float3 w12 = sample.transmitted;
		// compute contribution if sample is non-zero
		if (dot(sample.L, sample.L) > 0.0f)
		{
			// Russian roulette
			float dist = length(xo - sample.pos);
			float exp_term = exp(-dist * chosen_transport_rr);
			//exp_term = fmaxf(exp_term, 0.000001f);
#ifdef RND_64
			float rnd_number = rnd_accurate(t64);
#else
			float rnd_number = rnd_tea(t);
#endif
			if (rnd_number < exp_term )
			{
				
				if (dipole_model == DIRECTIONAL_DIPOLE) {
					accumulate += T12*sample.L*dirpole_bssrdf(sample.pos, sample.normal, w12, xo, no, props) / exp_term;
				}
				else if (dipole_model == STANDARD_DIPOLE) {
					accumulate += T12*sample.L*dipole_bssrdf(dist, props) / exp_term;
				}
			}
			else {
				//rtPrintf("no dipole \n");
			}
		}
	}
#ifdef TRANSMIT
	prd_radiance.result += accumulate*props.global_coeff / (float)N;
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
		float3 wr = 2.0f*cos_theta_o*no - wo;
		PerRayData_radiance prd_reflected;
		prd_reflected.depth = prd_radiance.depth + 1;
		prd_reflected.seed = t;
		prd_reflected.seed64 = t64;
		prd_reflected.result = make_float3(0.0f);
		prd_reflected.emit_light = 1;
		Ray reflected(xo, wr, radiance_ray_type, scene_epsilon);
		rtTrace(top_object, reflected, prd_reflected);
		prd_radiance.result += prd_reflected.result;
		t = prd_reflected.seed;
		t64 = prd_reflected.seed64;
	}
#endif
	prd_radiance.seed = t;
	prd_radiance.seed64 = t64;
	prd_radiance.result *= beam_T;
}

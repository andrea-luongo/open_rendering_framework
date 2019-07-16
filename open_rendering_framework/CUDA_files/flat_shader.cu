#include <optix.h>
#include <optix_math.h>
#include "../structs.h"
#include "../LightSampler.h"
#include "../Microfacet.h"
using namespace optix;

// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
//rtDeclareVariable(unsigned int, shadow_ray_type, , );
rtDeclareVariable(int, max_depth, , );

rtDeclareVariable(float3, diffuse_color, , );
rtDeclareVariable(float3, highlight_color, , );
rtDeclareVariable(uint, flat_shadow, , );
rtDeclareVariable(uint, highlight, , );
rtDeclareVariable(float, shininess, , );
rtDeclareVariable(float2, roughness, , );
rtDeclareVariable(uint, normal_distribution, , );
rtDeclareVariable(float, ior, , );
rtDeclareVariable(float, highlight_threshold, , );


// Any hit program for shadows
RT_PROGRAM void any_hit()
{
	// this material is opaque, so it fully attenuates all shadow rays
	prd_shadow.attenuation = 0.0f;
	rtTerminateRay();
}

// Closest hit program for Lambertian shading using the basic light as a directional source.
// This one includes shadows.
RT_PROGRAM void closest_hit()
{

	if (prd_radiance.depth > max_depth) return;
	float3 hit_pos = ray.origin + t_hit * ray.direction;
	float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(normal, -ray.direction, normal);
	float3 w_i = -ray.direction;
	uint& t = prd_radiance.seed;
	float3 result = diffuse_color ;

	//sampling microfacet normal
	float3 microfacet_normal;
	float z1 = rnd_tea(t);
	float z2 = rnd_tea(t);
	float n1_over_n2 = 1.0f / ior;
	microfacet_sample_visible_normal(w_i, ffnormal, microfacet_normal, roughness.x, roughness.y, z1, z2, normal_distribution);


	//sample light
	uint light_idx = light_buffer.size()*rnd_tea(t);
	float light_pdf = 1.0f / light_buffer.size();
	LightStruct direct_light = light_buffer[light_idx];
	float dist;
	float3 radiance;
	float3 w_l = make_float3(0.0f);
	evaluate_direct_illumination(hit_pos, &direct_light, w_l, radiance, dist, prd_radiance.seed);
	radiance /= light_pdf;

	float cos_theta = dot(ffnormal, w_l);

	float V = 0.0f;
	if (cos_theta > 0.0)
	{
		V = 1.0f;
		PerRayData_shadow shadow_prd;
		shadow_prd.attenuation = 1.0f;
		Ray shadow_ray(hit_pos, w_l, shadow_ray_type, scene_epsilon, dist - scene_epsilon);
		rtTrace(top_shadower, shadow_ray, shadow_prd);
		V = shadow_prd.attenuation;

	}

	if (flat_shadow == 1) {
		
			result *= V;
		
	}
	if (highlight == 1 && V > 0.0f) {

		float3 refl_normal = normalize(w_i + w_l);
		float cos_theta_refl = dot(refl_normal, w_l);
		//// Compute Fresnel reflectance (R) 
		float R_refl = 1.0f;
		float sin_theta_refl_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta_refl*cos_theta_refl);
		float cos_theta_refl_t = 0;
		if (sin_theta_refl_t_sqr < 1.0f)
		{
			cos_theta_refl_t = sqrtf(1.0f - sin_theta_refl_t_sqr);
			R_refl = fresnel_R(cos_theta_refl, cos_theta_refl_t, n1_over_n2);
		}
		float T_01_refl = 1.0f - R_refl;


		float G_i_m_refl = masking_G1(w_i, refl_normal, ffnormal, roughness.x, roughness.y, normal_distribution);
		float G_o_m_refl = masking_G1(w_l, refl_normal, ffnormal, roughness.x, roughness.y, normal_distribution);
		float D_refl = microfacet_eval_visible_normal(w_i, refl_normal, ffnormal,roughness.x, roughness.y, normal_distribution);
		float weight_refl = G_i_m_refl * G_o_m_refl * D_refl * R_refl / (4.0f * fabsf(dot(w_i, ffnormal) * dot(w_l, ffnormal)));
	
		
		//MyComplex eta = MyComplex{ n1_over_n2, 0 };
		//MyComplex3 eta3 = { eta, eta, eta };
		//float3 weight_refl = make_float3(0.0f);
		//weight_refl = microfacet_multiscattering_conductor_BSDF_eval(w_i, w_l, ffnormal, eta3, roughness.x, roughness.y, t, 0, normal_distribution);
	

		float3 specular_contribution = radiance * weight_refl;
		float3 luminance_weight = make_float3(0.2126f, 0.7152f, 0.0722f);
		float luminance = dot(luminance_weight, specular_contribution);
		//float3 specular_contribution = highlight_color * fmaxf(0.0f, powf(dot(refl_normal, microfacet_normal), shininess)) * V;
		//if (luminance.x > highlight_threshold || luminance.y>highlight_threshold || luminance.z > highlight_threshold)
		if (luminance > highlight_threshold)
			//result += specular_contribution *V*highlight_color;
			result = highlight_color *V;
	}

	prd_radiance.result = result;
}

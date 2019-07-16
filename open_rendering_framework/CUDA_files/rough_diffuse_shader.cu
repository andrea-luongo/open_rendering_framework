#include <optix.h>
#include <optix_math.h>
#include "../random.h"
#include "../structs.h"
#include "../sampler.h"
#include "../LightSampler.h"
#include "../Fresnel.h"
#include "../Microfacet.h"
#include "../MyComplex.h"
using namespace optix;



// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

// Variables for shading
//rtBuffer<LightStruct> light_buffer;
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(int, max_depth, , );

// Material properties 
rtTextureSampler<float4, 2> diffuse_map;
rtDeclareVariable(float3, emissive, , );
rtDeclareVariable(float3, diffuse_color, , );
rtDeclareVariable(float, ior, , );
rtDeclareVariable(uint, normal_distribution, , );
rtDeclareVariable(uint, microfacet_model, , );
rtDeclareVariable(float2, roughness, , );

// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
//rtDeclareVariable(unsigned int, shadow_ray_type, , );


// Recursive ray tracing variables
rtDeclareVariable(rtObject, top_object, , );
//rtDeclareVariable(unsigned int, radiance_ray_type, , );

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
	// Emission
	float3 result = make_float3(0.0f);
	uint& t = prd_radiance.seed;

	//refraction properties
	float n1_over_n2 = 1.0f / ior;
	float r_10 = two_C1(ior);
	float t_10 = 1.0f - r_10;

	float a_x = roughness.x;
	float a_y = roughness.y;
	//float3 up_normal = make_float3(0.0f, 1.0f, 0.0f);
	//float roughness_scale_factor = fabsf(dot(up_normal, ffnormal));
	//roughness_scale_factor = powf(roughness_scale_factor, 1.0f/2.0f);
	//a_x = (1 - a_x) * roughness_scale_factor + a_x;
	//a_y = (1 - a_y) * roughness_scale_factor + a_y;

	//rtPrintf("scale %f \n", roughness_scale_factor);
	//sampling microfacet normal
	float3 microfacet_normal;
	float z1 = rnd_tea(t);
	float z2 = rnd_tea(t);
	microfacet_sample_visible_normal(w_i, ffnormal, microfacet_normal, a_x, a_y, z1, z2, normal_distribution);

	float cos_theta_i = dot(w_i, microfacet_normal);
	//// Compute Fresnel reflectance (R)
	float R_i = 1.0f;
	float sin_theta_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta_i*cos_theta_i);
	float cos_theta_t = 0;
	if (sin_theta_t_sqr < 1.0f)
	{
		cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
		R_i = fresnel_R(cos_theta_i, cos_theta_t, n1_over_n2);
	}

	float T_01_i = 1.0f - R_i;
	// Direct illumination

	uint light_idx = light_buffer.size()*rnd_tea(t);
	float light_pdf = 1.0f / light_buffer.size();
	LightStruct direct_light = light_buffer[light_idx];

	float dist;
	float3 radiance;
	float3 w_l = make_float3(0.0f);
	float cos_theta_l = 0.0f;
	evaluate_direct_illumination(hit_pos, &direct_light, w_l, radiance, dist, prd_radiance.seed);
	radiance /= light_pdf;
	cos_theta_l = dot(ffnormal, w_l);
	if (cos_theta_l > 0.0)
	{
		float V = 1.0f;
		PerRayData_shadow shadow_prd;
		shadow_prd.attenuation = 1.0f;
		Ray shadow_ray(hit_pos, w_l, shadow_ray_type, scene_epsilon, dist);
		rtTrace(top_shadower, shadow_ray, shadow_prd);
		V = shadow_prd.attenuation;
		if (V > 0.0f) 
		{
			
			float3 Li = V*radiance;

			if (microfacet_model == MULTISCATTERING_MODEL)
			{
				float3 diffuse_brdf = make_float3(0.0f);
				diffuse_brdf = microfacet_multiscattering_diffuse_BSDF_eval(w_i, w_l, ffnormal, a_x, a_y, t, 0, diffuse_color, normal_distribution);
				result += Li * diffuse_brdf ;
				
				MyComplex eta = MyComplex{ n1_over_n2, 0 };
				MyComplex3 eta3 = { eta, eta, eta };
				float3 reflected_brdf = make_float3(0.0f);
				reflected_brdf = microfacet_multiscattering_conductor_BSDF_eval(w_i, w_l, ffnormal, eta3, a_x, a_y, t, 0, normal_distribution);
				
				result += Li * reflected_brdf ;
				prd_radiance.emit_light = 0;
			}
			else {

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

				float cos_theta_diff = dot(microfacet_normal, w_l);
				float R_diff = 1.0f;
				float sin_theta_diff_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta_diff*cos_theta_diff);
				float cos_theta_diff_t = 0;
				if (sin_theta_diff_t_sqr < 1.0f)
				{
					cos_theta_diff_t = sqrtf(1.0f - sin_theta_diff_t_sqr);
					R_diff = fresnel_R(cos_theta_diff, cos_theta_diff_t, n1_over_n2);
				}
				float T_01_diff = 1.0f - R_diff;

				float G_i_m_refl = masking_G1(w_i, refl_normal, ffnormal, a_x, a_y, normal_distribution);
				float G_o_m_refl = masking_G1(w_l, refl_normal, ffnormal, a_x, a_y, normal_distribution);
				float G_o_m_diff = masking_G1(w_l, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
				float D_refl = microfacet_eval_visible_normal(w_i, refl_normal, ffnormal, a_x, a_y, normal_distribution);
				float weight_refl = G_i_m_refl * G_o_m_refl * D_refl / (4.0f * fabsf(dot(w_i, ffnormal) * dot(w_l, ffnormal)));
				float weight_diff = G_o_m_diff;
			
				result += Li* M_1_PIf*cos_theta_l * diffuse_color * n1_over_n2 * n1_over_n2 * T_01_i * T_01_diff / (1.0f - diffuse_color * r_10) * weight_diff;
				result += Li* R_refl * weight_refl ;
				prd_radiance.emit_light = 0;
			}
		}
	}
	//}

	// Indirect illumination 

	float prob = R_i;
	float xi = rnd_tea(t);
	if (xi > prob)
	{
		if (microfacet_model == MULTISCATTERING_MODEL)
		{
			uint scatteringOrder = 0;
			float3 w_o;
			float3 w_m;
			float3 weight;
			microfacet_multiscattering_diffuse_BSDF_sample(w_i, w_o, w_m, ffnormal, a_x, a_y, t, scatteringOrder, weight, diffuse_color, normal_distribution);

			// Compute Fresnel reflectance (R) and compute diffuse contribution
			float cos_theta_r = dot(w_o, w_m);
			float R_r = 1.0f;
			float sin_theta_r_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta_r*cos_theta_r);
			float cos_theta_r_t = 0;
			if (sin_theta_r_t_sqr < 1.0f)
			{
				cos_theta_r_t = sqrtf(1.0f - sin_theta_r_t_sqr);
				R_r = fresnel_R(cos_theta_r, cos_theta_r_t, n1_over_n2);
			}
			float T_01_r = 1.0f - R_r;
			
			PerRayData_radiance prd_diffuse;
			prd_diffuse.depth = prd_radiance.depth + 1;
			prd_diffuse.seed = t;
			prd_diffuse.emit_light = prd_radiance.emit_light;
			prd_diffuse.result = make_float3(0.0f);
			Ray diffuse_ray(hit_pos, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, diffuse_ray, prd_diffuse);

			result += prd_diffuse.result * n1_over_n2 * n1_over_n2  * T_01_r / (1.0f - diffuse_color * r_10)  * weight;
			prd_radiance.seed = prd_diffuse.seed;

		}
		else {
			float3 diffuse_dir = sample_cosine_weighted(microfacet_normal, t);
			float cos_theta_r = dot(diffuse_dir, microfacet_normal);
			float G_o_m = masking_G1(diffuse_dir, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
			if (G_o_m < 0.0f) {
				prd_radiance.result = result;
				return;
			}
			// Compute Fresnel reflectance (R) and compute diffuse contribution
			float R_r = 1.0f;
			float sin_theta_r_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta_r*cos_theta_r);
			float cos_theta_r_t = 0;
			if (sin_theta_r_t_sqr < 1.0f)
			{
				cos_theta_r_t = sqrtf(1.0f - sin_theta_r_t_sqr);
				R_r = fresnel_R(cos_theta_r, cos_theta_r_t, n1_over_n2);
			}
			float T_01_r = 1.0f - R_r;

			PerRayData_radiance prd_diffuse;
			prd_diffuse.depth = prd_radiance.depth + 1;
			prd_diffuse.seed = t;
			prd_diffuse.emit_light = prd_radiance.emit_light;
			prd_diffuse.result = make_float3(0.0f);
			Ray diffuse_ray(hit_pos, diffuse_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, diffuse_ray, prd_diffuse);

			result += prd_diffuse.result * diffuse_color * n1_over_n2 * n1_over_n2 *T_01_r / (1.0f - diffuse_color * r_10)  * G_o_m;
			prd_radiance.seed = prd_diffuse.seed;
		}
	}
	else 
	{
		if (microfacet_model == MULTISCATTERING_MODEL)
		{
			MyComplex eta = MyComplex{ n1_over_n2, 0 };
			MyComplex3 eta3 = { eta, eta, eta };
			uint scatteringOrder = 10;
			float3 w_o;
			float3 w_m;
			float3 weight;
			microfacet_multiscattering_conductor_BSDF_sample(w_i, w_o, w_m, normal, eta3, a_x, a_y, t, scatteringOrder, weight, normal_distribution);
			PerRayData_radiance prd_new_ray;
			prd_new_ray.depth = prd_radiance.depth + 1;
			prd_new_ray.result = make_float3(0.0f);
			prd_new_ray.seed = t;
			prd_new_ray.emit_light = prd_radiance.emit_light;
			optix::Ray new_ray = optix::make_Ray(hit_pos, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, new_ray, prd_new_ray);
			result += prd_new_ray.result ;
		}
		else {
			//compute reflectance contribution
			float3 refl_dir = reflect(ray.direction, microfacet_normal);
			float G_o_m = masking_G1(refl_dir, microfacet_normal, ffnormal, a_x, a_y, normal_distribution);
			if (G_o_m < 0.0f) {
				prd_radiance.result = result;
				return;
			}
			PerRayData_radiance prd_refl;
			prd_refl.depth = prd_radiance.depth + 1;
			prd_refl.seed = prd_radiance.seed;
			prd_refl.emit_light = prd_radiance.emit_light;
			prd_refl.result = make_float3(0.0f);
			Ray refl_ray(hit_pos, refl_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, refl_ray, prd_refl);
			result += prd_refl.result * G_o_m;
			prd_radiance.seed = prd_refl.seed;
		}
	}

	//prd_radiance.depth++;
	prd_radiance.result = result;
}

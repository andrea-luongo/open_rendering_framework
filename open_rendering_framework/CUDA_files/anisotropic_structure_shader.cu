#include <optix.h>
#include <optix_math.h>
#include "../random.h"
#include "../structs.h"
#include "../sampler.h"
#include "../LightSampler.h"
#include "../Fresnel.h"
#include "../AnisotropicStructures.h"
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
rtDeclareVariable(int, max_depth, , );

// Material properties 
rtTextureSampler<float4, 2> diffuse_map;
rtDeclareVariable(float3, emissive, , );
rtDeclareVariable(float3, diffuse_color, , );
rtDeclareVariable(float, ior, , );
rtDeclareVariable(float, pattern_angle, , );
rtDeclareVariable(float, sinusoid_amplitude, , );
rtDeclareVariable(float2, roughness, , );
rtDeclareVariable(float2, sinusoid_wavelengths, , );
rtDeclareVariable(float, ridge_angle, , );
rtDeclareVariable(uint, structure_type, , );
rtDeclareVariable(rtObject, top_object, , );
rtTextureSampler<float4, 2> texture_sampler;
rtDeclareVariable(float3, texcoord, attribute texcoord, );
// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );

// Any hit program for shadows
RT_PROGRAM void any_hit()
{
	//// this material is opaque, so it fully attenuates all shadow rays
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
	prd_radiance.result = result;

	//hit point variables
	float3 hit_point = ray.origin + t_hit * ray.direction;
	float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(normal, -ray.direction, normal);
	float3 w_i = -ray.direction;
	
	uint& seed = prd_radiance.seed;
	float ior1_over_ior2 = 1.0f / ior;


	//used normal
	float3 n = ffnormal;
	float cos_i_n = dot(w_i, n);
	/*Texturized the plane in order to have chessboard pattern with different ridges orientation
	the ridge are going along the v direction*/
	float a_v = roughness.x;
	float a_u = roughness.y;
	float3 u, v;
	uint type = structure_type;
	float r_angle = ridge_angle;
	float p_angle = pattern_angle * M_PIf / 180.0f;
	float2 lambdas = sinusoid_wavelengths;
	if (type == TEXTURE_STRUCTURE)
	{
		float4 texture_data = tex2D(texture_sampler, texcoord.x, texcoord.y);
		
		if (texture_data.x < 0.5f + 0.01f && texture_data.x > 0.5f - 0.01f /*&& texture_data.y == 1.0f && texture_data.z == 1.0f*/) {

			type = RIDGED_STRUCTURE;
			r_angle = texture_data.y * 90.0f;
			p_angle = texture_data.z * 2.0f  * M_PIf ;
			//ridge_create_rotate_onb(n, u, v, p_angle);
		}
		else if (texture_data.x < 0.25f + 0.01f && texture_data.x > 0.25f - 0.01f /*&& texture_data.y == 1.0f && texture_data.z == 1.0f*/) {
			type = SINUSOIDAL_STRUCTURE;
			lambdas.x = texture_data.y ;
			lambdas.y = texture_data.z ;
			//ridge_create_rotate_onb(n, u, v, p_angle);
		}
		//ridge_create_onb(n, u, v, texcoord.x, texcoord.y);
	}


	if (type == RIDGED_STRUCTURE)
	{
		ridge_create_rotate_onb(n, u, v, p_angle);
	
	}
	else if (type == SINUSOIDAL_STRUCTURE)
	{
		ridge_create_rotate_onb(n, u, v, p_angle);
	}

	//rtPrintf("u %f %f %f v %f %f %f \n", u.x, u.y, u.z, v.x, v.y, v.z);
	//prd_radiance.result = (u)*0.5f + make_float3(0.5f);
	//return;
	//rtPrintf("angle %f \n", r_angle);
	if (type == RIDGED_STRUCTURE)
	{

		//indirect light
		float3 w_o = make_float3(0.0f);
		float3 brdf = make_float3(0.0f);
		float3 m = make_float3(0.0f);
		ridge_sample_BRDF(r_angle, u, v, n, w_i, diffuse_color, ior1_over_ior2, a_u, a_v, seed, m, w_o, brdf);

		if (length(brdf) > 0.0f) {
			PerRayData_radiance prd_new_ray;
			prd_new_ray.depth = prd_radiance.depth + 1;
			prd_new_ray.result = make_float3(0.0f);
			prd_new_ray.seed = seed;
			prd_new_ray.emit_light = 1;
			optix::Ray new_ray = optix::make_Ray(hit_point, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, new_ray, prd_new_ray);
			prd_radiance.seed = prd_new_ray.seed;
			//rtPrintf("brdf %f %f %f\n", brdf.x, brdf.y, brdf.z);
			result += prd_new_ray.result * brdf;
		}

		
		//direct light
		uint light_idx = light_buffer.size()*rnd_tea(seed);
		float light_pdf = 1.0f / light_buffer.size();
		LightStruct direct_light = light_buffer[light_idx];
		float dist;
		float3 radiance;
		float3 w_l = make_float3(0.0f);
		float cos_theta = 0.0f;
		evaluate_direct_illumination(hit_point, &direct_light, w_l, radiance, dist, prd_radiance.seed);
		radiance /= light_pdf;
		float cos_n_l = dot(ffnormal, w_l);
		if (cos_n_l > 0.0)
		{
			float V = 1.0f;
			PerRayData_shadow shadow_prd;
			shadow_prd.attenuation = 1.0f;
			Ray shadow_ray(hit_point, w_l, shadow_ray_type, scene_epsilon, dist - scene_epsilon);
			rtTrace(top_shadower, shadow_ray, shadow_prd);
			V = shadow_prd.attenuation;
			if (V > 0.0f) {
				float3 Li = V*radiance;
				//diffuse part

				float3 direct_brdf = make_float3(0.0f);
				
				ridge_eval_BRDF(r_angle, u, v, n, w_i, w_l, diffuse_color, ior1_over_ior2, a_u, a_v, seed, direct_brdf);
				result += direct_brdf * cos_n_l * Li ;

				//rtPrintf("direct_brdf %f %f %f\n", direct_brdf.x, direct_brdf.y, direct_brdf.z);
			}
		
		}
	}
	else if (type == SINUSOIDAL_STRUCTURE)
	{

		//indirect light
		float3 w_o = make_float3(0.0f);
		float3 brdf = make_float3(0.0f);
		float3 m = make_float3(0.0f);
		float A = sinusoid_amplitude;
		//rtPrintf("lambdas %f %f A %f\n", lambdas.x, lambdas.y, A);
		sinusoid_sample_BRDF(lambdas, A, u, v, n, w_i, diffuse_color, ior1_over_ior2, a_u, a_v, seed, m, w_o, brdf);

		if (length(brdf) > 0.0f) {
			PerRayData_radiance prd_new_ray;
			prd_new_ray.depth = prd_radiance.depth + 1;
			prd_new_ray.result = make_float3(0.0f);
			prd_new_ray.seed = seed;
			prd_new_ray.emit_light = 1;
			optix::Ray new_ray = optix::make_Ray(hit_point, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, new_ray, prd_new_ray);
			prd_radiance.seed = prd_new_ray.seed;
			//rtPrintf("brdf %f %f %f\n", brdf.x, brdf.y, brdf.z);
			result += prd_new_ray.result * brdf;
		}


		//direct light
		uint light_idx = light_buffer.size()*rnd_tea(seed);
		float light_pdf = 1.0f / light_buffer.size();
		LightStruct direct_light = light_buffer[light_idx];
		float dist;
		float3 radiance;
		float3 w_l = make_float3(0.0f);
		float cos_theta = 0.0f;
		evaluate_direct_illumination(hit_point, &direct_light, w_l, radiance, dist, prd_radiance.seed);
		radiance /= light_pdf;
		float cos_n_l = dot(ffnormal, w_l);
		if (cos_n_l > 0.0)
		{
			float V = 1.0f;
			PerRayData_shadow shadow_prd;
			shadow_prd.attenuation = 1.0f;
			Ray shadow_ray(hit_point, w_l, shadow_ray_type, scene_epsilon, dist - scene_epsilon);
			rtTrace(top_shadower, shadow_ray, shadow_prd);
			V = shadow_prd.attenuation;
			if (V > 0.0f) {
				float3 Li = V*radiance;
				//diffuse part

				float3 direct_brdf = make_float3(0.0f);

				sinusoid_eval_BRDF(lambdas, A, u, v, n, w_i, w_l, diffuse_color, ior1_over_ior2, a_u, a_v, seed, direct_brdf);
				result += direct_brdf * cos_n_l * Li;

				//rtPrintf("direct_brdf %f %f %f\n", direct_brdf.x, direct_brdf.y, direct_brdf.z);
			}

		}
	}
	else {

		float3 w_o = sample_cosine_weighted(ffnormal, seed);

		float cos_i_n = dot(w_i, ffnormal);
		float cos_o_n = dot(w_o, ffnormal);
		float F_r = fmaxf(0.0f, fresnel_R(fmaxf(0.0f, (dot(w_i, ffnormal))), ior1_over_ior2));
		float diff_weight = 1.0f;
		float F_t_i_m = 1.0f - fresnel_R(fabsf(cos_i_n), ior1_over_ior2);
		float F_t_o_m = 1.0f - fresnel_R(fabsf(cos_o_n), ior1_over_ior2);
		diff_weight *= F_t_i_m * F_t_o_m ;
		float3 brdf =  diff_weight *diffuse_color;

		PerRayData_radiance prd_diffuse;
		prd_diffuse.depth = prd_radiance.depth + 1;
		prd_diffuse.seed = seed;
		prd_diffuse.emit_light = prd_radiance.emit_light;
		prd_diffuse.result = make_float3(0.0f);
		Ray diffuse_ray(hit_point, w_o, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		rtTrace(top_object, diffuse_ray, prd_diffuse);
		result += prd_diffuse.result * brdf;
		prd_radiance.seed = prd_diffuse.seed;
		//float r_10 = two_C1(ior);
		//float t_10 = 1.0f - r_10;
		//float cos_theta_i = dot(-ray.direction, ffnormal);
		////// Compute Fresnel reflectance (R) and trace compute reflected and refracted directions
		//float R_i = 1.0f;
		//float sin_theta_t_sqr = ior1_over_ior2*ior1_over_ior2*(1.0f - cos_theta_i*cos_theta_i);
		//float cos_theta_t = 0;
		//if (sin_theta_t_sqr < 1.0f)
		//{
		//	cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
		//	R_i = fresnel_R(cos_theta_i, cos_theta_t, ior1_over_ior2);
		//}
		//float T_01_i = 1.0f - R_i;
		//// Direct illumination
		////for (int i = 0; i < light_buffer.size(); ++i)
		////{
		//uint light_idx = light_buffer.size()*rnd_tea(seed);
		//float light_pdf = 1.0f / light_buffer.size();
		//LightStruct direct_light = light_buffer[light_idx];
		////LightStruct direct_light = light_buffer[i];
		//float dist;
		//float3 radiance;
		//float3 w_l = make_float3(0.0f);
		//float cos_theta_l = 0.0f;
		//evaluate_direct_illumination(hit_point, &direct_light, w_l, radiance, dist, prd_radiance.seed);
		//radiance /= light_pdf;
		//cos_theta_l = dot(ffnormal, w_l);
		//if (cos_theta_l > 0.0)
		//{
		//	float V = 1.0f;
		//	PerRayData_shadow shadow_prd;
		//	shadow_prd.attenuation = 1.0f;
		//	Ray shadow_ray(hit_point, w_l, shadow_ray_type, scene_epsilon, dist);
		//	rtTrace(top_shadower, shadow_ray, shadow_prd);
		//	V = shadow_prd.attenuation;
		//	float3 Li = V*radiance;
		//	//// Compute Fresnel reflectance (R) and trace compute reflected and refracted directions
		//	float R_l = 1.0f;
		//	float sin_theta_l_t_sqr = ior1_over_ior2*ior1_over_ior2*(1.0f - cos_theta_l*cos_theta_l);
		//	float cos_theta_l_t = 0;
		//	if (sin_theta_l_t_sqr < 1.0f)
		//	{
		//		cos_theta_l_t = sqrtf(1.0f - sin_theta_l_t_sqr);
		//		R_l = fresnel_R(cos_theta_l, cos_theta_l_t, ior1_over_ior2);
		//	}
		//	float T_01_l = 1.0f - R_l;
		//	result += Li*M_1_PIf*cos_theta_l * diffuse_color * ior1_over_ior2 * ior1_over_ior2 * T_01_l * T_01_i / (1.0f - diffuse_color * r_10);
		//	prd_radiance.emit_light = 0;
		//}
		////}
		//// Indirect illumination 
		//float prob = 0.5f;
		//prob = (diffuse_color.x + diffuse_color.y + diffuse_color.z) / 3.0f;
		//prob = R_i;
		//float xi = rnd_tea(seed);
		//xi = 1;
		//if (xi > prob)
		//{
		//	float3 diffuse_dir = sample_cosine_weighted(ffnormal, seed);
		//	float cos_theta_r = dot(diffuse_dir, ffnormal);
		//	// Compute Fresnel reflectance (R) and compute diffuse contribution
		//	float R_r = 1.0f;
		//	float sin_theta_r_t_sqr = ior1_over_ior2*ior1_over_ior2*(1.0f - cos_theta_r*cos_theta_r);
		//	float cos_theta_r_t = 0;
		//	if (sin_theta_r_t_sqr < 1.0f)
		//	{
		//		cos_theta_r_t = sqrtf(1.0f - sin_theta_r_t_sqr);
		//		R_r = fresnel_R(cos_theta_r, cos_theta_r_t, ior1_over_ior2);
		//	}
		//	float T_01_r = 1.0f - R_r;
		//	PerRayData_radiance prd_diffuse;
		//	prd_diffuse.depth = prd_radiance.depth + 1;
		//	prd_diffuse.seed = seed;
		//	prd_diffuse.emit_light = prd_radiance.emit_light;
		//	prd_diffuse.result = make_float3(0.0f);
		//	Ray diffuse_ray(hit_point, diffuse_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		//	rtTrace(top_object, diffuse_ray, prd_diffuse);
		//	result += prd_diffuse.result * diffuse_color * ior1_over_ior2 * ior1_over_ior2 *T_01_r *(1-R_i)/ (1.0f - diffuse_color * r_10);
		//	prd_radiance.seed = prd_diffuse.seed;
		//}
		//else {
		//	//compute reflectance contribution
		//	float3 refl_dir = reflect(ray.direction, normal);
		//	float cos_theta_refl = dot(refl_dir, ffnormal);
		//	PerRayData_radiance prd_refl;
		//	prd_refl.depth = prd_radiance.depth + 1;
		//	prd_refl.seed = prd_radiance.seed;
		//	prd_refl.emit_light = 1;
		//	prd_refl.result = make_float3(0.0f);
		//	Ray refl_ray(hit_point, refl_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		//	rtTrace(top_object, refl_ray, prd_refl);
		//	result += prd_refl.result;
		//	prd_radiance.seed = prd_refl.seed;
		//}
	}


		prd_radiance.result = result;
	
}
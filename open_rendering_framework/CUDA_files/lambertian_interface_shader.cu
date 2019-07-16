#include <optix.h>
#include <optix_math.h>
#include "../random.h"
#include "../structs.h"
#include "../sampler.h"
#include "../LightSampler.h"
#include "../Fresnel.h"

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
	// Emission
	float3 result = make_float3(0.0f);
	uint& t = prd_radiance.seed;
	float n1_over_n2 = 1.0f / ior;
	float r_10 = two_C1(ior);
	float t_10 = 1.0f - r_10;


	float cos_theta_i = dot(-ray.direction, ffnormal);
	//// Compute Fresnel reflectance (R) and trace compute reflected and refracted directions
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
	//for (int i = 0; i < light_buffer.size(); ++i)
	//{
	uint light_idx = light_buffer.size()*rnd_tea(t);
	float light_pdf = 1.0f / light_buffer.size();
	LightStruct direct_light = light_buffer[light_idx];
		//LightStruct direct_light = light_buffer[i];
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
		float3 Li = V*radiance;
		//// Compute Fresnel reflectance (R) and trace compute reflected and refracted directions
		float R_l = 1.0f;
		float sin_theta_l_t_sqr = n1_over_n2*n1_over_n2*(1.0f - cos_theta_l*cos_theta_l);
		float cos_theta_l_t = 0;
		if (sin_theta_l_t_sqr < 1.0f)
		{
			cos_theta_l_t = sqrtf(1.0f - sin_theta_l_t_sqr);
			R_l = fresnel_R(cos_theta_l, cos_theta_l_t, n1_over_n2);
		}
		float T_01_l = 1.0f - R_l;

		result += Li*M_1_PIf*cos_theta_l * diffuse_color * n1_over_n2 * n1_over_n2 * T_01_l * T_01_i / (1.0f - diffuse_color * r_10);

		prd_radiance.emit_light = 0;
	}
	//}

	// Indirect illumination 
	
	float prob = 0.5f;
	prob = (diffuse_color.x + diffuse_color.y + diffuse_color.z) / 3.0f;
	prob = R_i;
	float xi = rnd_tea(t);
	if (xi > prob)
	{
		float3 diffuse_dir = sample_cosine_weighted(ffnormal, t);
		float cos_theta_r = dot(diffuse_dir, ffnormal);

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

		result += prd_diffuse.result * diffuse_color * n1_over_n2 * n1_over_n2 *T_01_r  / (1.0f - diffuse_color * r_10);
		prd_radiance.seed = prd_diffuse.seed;
	}
	else {
		//compute reflectance contribution
		float3 refl_dir = reflect(ray.direction, normal);
		float cos_theta_refl = dot(refl_dir, ffnormal);

		PerRayData_radiance prd_refl;
		prd_refl.depth = prd_radiance.depth + 1;
		prd_refl.seed = prd_radiance.seed;
		prd_refl.emit_light = 1;
		prd_refl.result = make_float3(0.0f);
		Ray refl_ray(hit_pos, refl_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		rtTrace(top_object, refl_ray, prd_refl);
		result += prd_refl.result ;
		prd_radiance.seed = prd_refl.seed;

	}

	//prd_radiance.depth++;
	prd_radiance.result = result;
}

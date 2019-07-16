#include <optix.h>
#include <optix_math.h>
#include "../random.h"
#include "../structs.h"
#include "../sampler.h"
#include "../LightSampler.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(int, max_depth, , );
// Material properties (corresponding to OBJ mtl params)
rtTextureSampler<float4, 2> diffuse_map;
rtDeclareVariable(float3, emissive, , );
rtDeclareVariable(float3, diffuse_color, , );
// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );

// Recursive ray tracing variables
rtDeclareVariable(rtObject, top_object, , );


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
	float3 rho_d = make_float3(tex2D(diffuse_map, texcoord.x, texcoord.y));
	uint& t = prd_radiance.seed;
	// Emission
	float3 result = /*prd_radiance.emit_light ? emissive :*/ make_float3(0.0f);

	uint light_idx = light_buffer.size()*rnd_tea(t);
	float light_pdf = 1.0f / light_buffer.size();
	LightStruct direct_light = light_buffer[light_idx];
	

	float dist;
	float3 radiance;
	float3 w_l = make_float3(0.0f);
	float cos_theta = 0.0f;

	evaluate_direct_illumination(hit_pos, &direct_light, w_l, radiance, dist, prd_radiance.seed);
	radiance /= light_pdf;
	cos_theta = dot(ffnormal, w_l);

	if (cos_theta > 0.0)
	{
		float V = 1.0f;
		PerRayData_shadow shadow_prd;
		shadow_prd.attenuation = 1.0f;
		Ray shadow_ray(hit_pos, w_l, shadow_ray_type, scene_epsilon, dist - scene_epsilon);
		rtTrace(top_shadower, shadow_ray, shadow_prd);
		V = shadow_prd.attenuation;
		float3 Li = V*radiance;
		result += Li*M_1_PIf*cos_theta*diffuse_color;

		prd_radiance.emit_light = 0;
	}

	// Indirect illumination 

	float prob = 1.0f;
	prob = (diffuse_color.x + diffuse_color.y + diffuse_color.z) / 3.0f;
	float xi = rnd_tea(t);
	if (xi < prob)
	{
		float3 new_dir = sample_cosine_weighted(ffnormal, t);

		PerRayData_radiance prd_new;
		prd_new.depth = prd_radiance.depth + 1;
		prd_new.seed = t;
		prd_new.result = make_float3(0.0f);
		prd_new.emit_light = prd_radiance.emit_light;
		Ray new_ray(hit_pos, new_dir, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		rtTrace(top_object, new_ray, prd_new);
		result += prd_new.result*diffuse_color / prob;

		prd_radiance.seed = prd_new.seed;
	}
	else
		prd_radiance.seed = t;

	prd_radiance.result = result;
}

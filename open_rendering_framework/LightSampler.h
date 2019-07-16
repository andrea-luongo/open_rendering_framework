//// 02576 OptiX Rendering Framework
//// Written by Jeppe Revall Frisvad, 2011
//// Copyright (c) DTU Informatics 2011
//
#ifndef LIGHTSAMPLER_H
#define LIGHTSAMPLER_H

#include <optix.h>
#include <optix_math.h>
#include "random.h"
#include "sampler.h"
#include "structs.h"
#include "helpers.h"
//
// Area light variables
rtBuffer<LightStruct> light_buffer;
rtBuffer<TriangleLight> triangle_light_buffer;
//

__device__ __inline__ void evaluate_point_light(const float3& pos, const PointLightStruct* point_light, float3& dir, float3& L, float& dist)
{
	float3 light_pos = point_light->position;
	dir = light_pos - pos;
	float r_sqr = dot(dir, dir);
	dist = sqrt(r_sqr);
	dir /= dist;
	L = point_light->emitted_radiance / r_sqr;
}


__device__ __inline__ void evaluate_directional_light(const float3& pos, const DirectionalLightStruct* directional_light, float3& dir, float3& L, float& dist)
{

	dir = normalize(-directional_light->direction);
	dist = RT_DEFAULT_MAX;
	L = directional_light->emitted_radiance;
}


__device__ __inline__ void evaluate_triangle_area_light(const float3& pos, const TrianglesAreaLightStruct* light_struct, float3& dir, float3& L, float& dist, uint& seed)
{
	  // sample a triangle
	  uint triangles = light_struct->triangle_count;
	  uint triangle_id = (uint)(rnd_tea(seed)*triangles) + light_struct->buffer_start_idx;
	  
	  TriangleLight triangle_light = triangle_light_buffer[triangle_id];
	  float3 uvw = sample_barycentric(seed);
	  float3 light_pos = triangle_light.v0*uvw.x + triangle_light.v1*uvw.y + triangle_light.v2*uvw.z;
	 
	  //float4 light_pos4 = light_struct->transformation_matrix * make_float4(light_pos, 1.0f);
	  //light_pos = make_float3(light_pos4);
	
	  // Compute normal
	  float3 n;
	  if (triangle_light.has_normals)
	  {
		  n = normalize(triangle_light.n0*uvw.x + triangle_light.n1*uvw.y + triangle_light.n2*uvw.z);
	  }
	  else {
		  n = normalize(cross(triangle_light.v1 - triangle_light.v0, triangle_light.v2 - triangle_light.v0));
	  }
	  //n = make_float3(light_struct->transformation_matrix.inverse().transpose() * make_float4(n, 0.0f));
	  // Find distance and direction
	  dir = light_pos - pos;
	  float sqr_dist = dot(dir, dir);
	  dist = sqrt(sqr_dist);
	  dir = normalize(dir);
	  float cos_theta_prime = fmaxf(dot(n, -dir), 0.0f);

	  L = light_struct->emitted_radiance*(triangle_light.area*cos_theta_prime/sqr_dist*triangles);
}

__device__ __inline__ void evaluate_disk_area_light(const float3& pos, const DiskLightStruct* disk_light, float3& dir, float3& L, float& dist, uint& seed)
{
	float3 light_pos = disk_light->position;
	float light_radius = disk_light->radius;
	float theta = disk_light->theta * M_PIf / 180.0f;
	float phi = disk_light->phi * M_PIf / 180.0f;

	float area = M_PIf * light_radius * light_radius;
	float pdf = 1.0f / area;

	float3 normal = make_float3(sinf(theta) * sinf(phi), cosf(theta), sinf(theta) * cosf(phi));
	float3 U, V;
	create_onb(normal, U, V);

	float u1 = rnd_tea(seed);
	float u2 = rnd_tea(seed);
	float r = sqrtf(u1) * light_radius;
	float t = 2.0f * M_PIf * u2;
	float3 disk_point = make_float3(r * cosf(t), r * sinf(t), 0.0f);
	disk_point = disk_point.x * U + disk_point.y * V + disk_point.z * normal;
	light_pos = light_pos + disk_point;
	dir = light_pos - pos;
	float sqr_dist = dot(dir, dir);
	dist = sqrt(sqr_dist);
	dir = normalize(dir);
	float cos_theta_prime = fmaxf(dot(normal, -dir), 0.0f);

	L = disk_light->emitted_radiance*(cos_theta_prime / sqr_dist) / pdf;
}

__device__ __inline__ void evaluate_spherical_area_light(const float3& pos, const SphericalLightStruct* spherical_light, float3& dir, float3& L, float& dist, uint& seed)
{
	float3 light_pos = spherical_light->position;
	float light_radius = spherical_light->radius;
	float3 radiance = spherical_light->emitted_radiance;

	float3 visible_normal = normalize(pos - light_pos);
	float3 U, V;
	create_onb(visible_normal, U, V);

	float area = 4 * M_PIf * light_radius * light_radius;
	float pdf = 1.0f / area;
	float u1 = rnd_tea(seed);
	float u2 = rnd_tea(seed);
	float z_coord =  u1;
	float3 sphere_point = light_radius * make_float3(cosf(2 * M_PIf*u2) * sqrtf(1 - z_coord * z_coord), sinf(2 * M_PIf*u2) * sqrtf(1 - z_coord * z_coord), z_coord);
	sphere_point = sphere_point.x * U + sphere_point.y * V + sphere_point.z * visible_normal + light_pos;
	float3 normal = normalize(sphere_point - light_pos);
	dir = sphere_point - pos;
	float sqr_dist = dot(dir, dir);
	dist = sqrt(sqr_dist);
	dir = normalize(dir);
	float cos_theta_prime = fmaxf(dot(normal, -dir), 0.0f);
	L = radiance*(cos_theta_prime / sqr_dist) / pdf;

	//float area = 4 * M_PIf * light_radius * light_radius;
	//float pdf = 1.0f / area;
	//float u1 = rnd_tea(seed);
	//float u2 = rnd_tea(seed);
	//float z_coord = 1 - 2 * u1;
	//float3 sphere_point = light_pos + light_radius * make_float3(cosf(2*M_PIf*u2) * sqrtf(1 - z_coord * z_coord), sinf(2 * M_PIf*u2) * sqrtf(1 - z_coord * z_coord), z_coord);
	//float3 normal = normalize(sphere_point - light_pos);
	//dir = sphere_point - pos;
	//float sqr_dist = dot(dir, dir);
	//dist = sqrt(sqr_dist);
	//dir = normalize(dir);
	//float cos_theta_prime = fmaxf(dot(normal, -dir), 0.0f);
	//L = radiance*(cos_theta_prime / sqr_dist) / pdf;
}

__device__ __inline__ void evaluate_direct_illumination(const float3& pos, LightStruct* light_struct, float3& dir, float3& L, float& dist, uint& seed)
{
	if (light_struct->light_type == POINT_LIGHT) {
		PointLightStruct* point_light = reinterpret_cast<PointLightStruct*>(light_struct);
		evaluate_point_light(pos, point_light, dir, L, dist);
	}
	if (light_struct->light_type == DIRECTIONAL_LIGHT) {
		DirectionalLightStruct* directional_light = reinterpret_cast<DirectionalLightStruct*>(light_struct);
		evaluate_directional_light(pos, directional_light, dir, L, dist);
	}
	if (light_struct->light_type == TRIANGLES_AREA_LIGHT) {
		TrianglesAreaLightStruct* triangle_light = reinterpret_cast<TrianglesAreaLightStruct*>(light_struct);
		evaluate_triangle_area_light(pos, triangle_light, dir, L, dist, seed);
	}
	if (light_struct->light_type == DISK_LIGHT) {
		DiskLightStruct* disk_light = reinterpret_cast<DiskLightStruct*>(light_struct);
		evaluate_disk_area_light(pos, disk_light, dir, L, dist, seed);
	}
	if (light_struct->light_type == SPHERICAL_LIGHT) {
		SphericalLightStruct* spherical_light = reinterpret_cast<SphericalLightStruct*>(light_struct);
		evaluate_spherical_area_light(pos, spherical_light, dir, L, dist, seed);
	}

}

#endif
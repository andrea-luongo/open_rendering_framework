
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "../helpers.h"
using namespace optix;

// This is to be plugged into an RTgeometry object to represent
// a triangle mesh with a vertex buffer of triangle soup (triangle list)
// with an interleaved position, normal, texturecoordinate layout.

rtDeclareVariable(float3, radiance, , );
rtDeclareVariable(float3, position, , );
rtDeclareVariable(float3, normal, , );
rtDeclareVariable(float, radius, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, scene_epsilon, , );
RT_PROGRAM void disk_intersect(int primIdx)
{

	float3 O = ray.origin - position;
	
	float den = dot(normal, ray.direction);
	if (fabsf(den) > scene_epsilon) {
		
		float t = -dot(O, normal) / den;
		//if(t > ray.tmin && t < ray.tmax){
		float3 point = ray.origin + t*ray.direction;
		float3 v = point - position;
		float square_dist = dot(v, v);
		if (square_dist <= radius*radius) {
			if (rtPotentialIntersection(t)) {
				shading_normal = geometric_normal = normal;
				
				rtReportIntersection(0);
			}
		}
	}

}

RT_PROGRAM void disk_bounds(int , float result[6])
{

	optix::Aabb* aabb = (optix::Aabb*)result;
	float3 extent = radius * sqrt(1.0f - normal*normal);
	if (radius > 0.0f && !isinf(radius)) {
		aabb->m_min = position - extent;
		aabb->m_max = position + extent;
	}
	else {
		aabb->invalidate();
	}
}



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
rtDeclareVariable(float, radius, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, scene_epsilon, , );

RT_PROGRAM void sphere_intersect(int primIdx)
{
	float3 center = position;
	float3 O = ray.origin - center;
	float3 D = ray.direction;
	// primitive_idx = 1;
	//intersection_length = 1.0f;
	float b = dot(O, D);
	float c = dot(O, O) - radius*radius;
	float disc = b*b - c;
	if (disc > 0.0f) {
		float sdisc = sqrtf(disc);
		float root1 = (-b - sdisc);

		bool do_refine = false;

		float root11 = 0.0f;

		if (fabsf(root1) > 10.f * radius) {
			do_refine = true;
		}

		if (do_refine) {
			// refine root1
			float3 O1 = O + root1 * ray.direction;
			b = dot(O1, D);
			c = dot(O1, O1) - radius*radius;
			disc = b*b - c;

			if (disc > 0.0f) {
				sdisc = sqrtf(disc);
				root11 = (-b - sdisc);
			}
		}
		bool check_second = true;
		if (rtPotentialIntersection(root1 + root11)) {
			shading_normal = geometric_normal = (O + (root1 + root11)*D) / radius;

			if (rtReportIntersection(0))
				check_second = false;
		}
		if (check_second) {
			float root2 = (-b + sdisc) + (do_refine ? root1 : 0);
			if (rtPotentialIntersection(root2)) {
				shading_normal = geometric_normal = (O + root2*D) / radius;

				rtReportIntersection(0);
			}
		}
	}
}

RT_PROGRAM void sphere_bounds(int , float result[6])
{

	optix::Aabb* aabb = (optix::Aabb*)result;
	float3 extent = make_float3(radius);
	if (radius > 0.0f && !isinf(radius)) {
		aabb->m_min = position - extent;
		aabb->m_max = position + extent;
	}
	else {
		aabb->invalidate();
	}
}


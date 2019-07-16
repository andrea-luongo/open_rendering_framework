#pragma once

#include <optix_world.h>
#include "../structs.h"
#include "../sampler.h"
#include "../fresnel.h"

using namespace optix;

//__device__ float3 rough_dipole_bssrdf(float dist, const float3& _ni, const ScatteringMaterialProperties& properties, uint& t)
//{
//  float3 real_source = properties.three_D*properties.three_D;
//  float3 extrapolation = 4.0f*properties.A*properties.D;
//  float3 virtual_source = extrapolation*extrapolation;
//  float3 corrected_mean_free = properties.three_D + extrapolation;
//  float3 r_sqr = make_float3(dist*dist);
//
//  // Compute distances to dipole sources
//  float3 d_r_sqr = r_sqr + real_source;
//  float3 d_r = sqrt(d_r_sqr);
//  float3 d_v_sqr = r_sqr + virtual_source;
//  float3 d_v = sqrt(d_v_sqr);
//
//  // Compute intensities for dipole sources
//  float3 tr_r = properties.transport*d_r;
//  float3 S_r = properties.three_D*(1.0f + tr_r)/(d_r_sqr*d_r);
//  S_r *= expf(-tr_r);
//  float3 tr_v = properties.transport*d_v;
//  float3 S_v = corrected_mean_free*(1.0f + tr_v)/(d_v_sqr*d_v);
//  S_v *= expf(-tr_v);
//  float3 S_d = S_r + S_v;
//
//  float3 S = make_float3(0.0f);
//  for(uint i = 0; i < 5; ++i)
//  {
//    float3 w12 = sample_cosine_weighted(-_ni, t);
//    float T12 = 1.0f - fresnel_R(dot(w12, -_ni), properties.indexOfRefraction);
//    S += T12*S_d;
//  }
//  return S*0.2f;
//}

__device__ float3 rough_dipole_bssrdf(float dist, const ScatteringMaterialProperties& properties)
{
	float3 real_source = properties.three_D*properties.three_D;
	float3 extrapolation = 4.0f*properties.A*properties.D;
	float3 virtual_source = extrapolation*extrapolation;
	float3 corrected_mean_free = properties.three_D + extrapolation;
	float3 r_sqr = make_float3(dist*dist);

	// Compute distances to dipole sources
	float3 d_r_sqr = r_sqr + real_source;
	float3 d_r = make_float3(sqrtf(d_r_sqr.x), sqrtf(d_r_sqr.y), sqrtf(d_r_sqr.z));
	float3 d_v_sqr = r_sqr + virtual_source;
	float3 d_v = make_float3(sqrtf(d_v_sqr.x), sqrtf(d_v_sqr.y), sqrtf(d_v_sqr.z));

	// Compute intensities for dipole sources
	float3 tr_r = properties.transport*d_r;
	float3 S_r = properties.three_D*(1.0f + tr_r) / (d_r_sqr*d_r);
	S_r *= expf(-tr_r);
	float3 tr_v = properties.transport*d_v;
	float3 S_v = corrected_mean_free*(1.0f + tr_v) / (d_v_sqr*d_v);
	S_v *= expf(-tr_v);
	return S_r + S_v;
}
__device__ float3 diffuser_dipole_bssrdf(float dist, const float3& _ni, const ScatteringMaterialProperties& properties, uint& t)
{
  float3 real_source = properties.three_D*properties.three_D;
  float3 extrapolation = 4.0f*properties.A*properties.D;
  float3 virtual_source = extrapolation*extrapolation;
  float3 corrected_mean_free = properties.three_D + extrapolation;
  float3 r_sqr = make_float3(dist*dist);

  // Compute distances to dipole sources
  float3 d_r_sqr = r_sqr + real_source;
  float3 d_r = sqrt(d_r_sqr);
  float3 d_v_sqr = r_sqr + virtual_source;
  float3 d_v = sqrt(d_v_sqr);

  // Compute intensities for dipole sources
  float3 tr_r = properties.transport*d_r;
  float3 S_r = properties.three_D*(1.0f + tr_r)/(d_r_sqr*d_r);
  S_r *= expf(-tr_r);
  float3 tr_v = properties.transport*d_v;
  float3 S_v = corrected_mean_free*(1.0f + tr_v)/(d_v_sqr*d_v);
  S_v *= expf(-tr_v);
  float3 S_d = S_r + S_v;

  float3 S = make_float3(0.0f);
  for(uint i = 0; i < 5; ++i)
  {
    float3 w12 = sample_cosine_weighted(-_ni, t);
    float T12 = 1.0f - fresnel_R(dot(w12, -_ni), properties.relative_ior);
    S += T12*S_d;
  }
  return S*0.2f;
}
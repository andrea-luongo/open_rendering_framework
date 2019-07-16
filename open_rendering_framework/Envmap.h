#ifndef ENVMAP_H
#define ENVMAP_H

#include <optix_world.h>
#include "random.h"
#include "structs.h"

// Environment map
rtTextureSampler<float4, 2> envmap;

// Environment importance sampling
rtBuffer<float> marginal_pdf;
rtBuffer<float, 2> conditional_pdf;
rtBuffer<float> marginal_cdf;
rtBuffer<float, 2> conditional_cdf;

__forceinline__ __device__ optix::float2 direction_to_uv_coord_cubemap(const optix::float3& direction, const optix::Matrix3x3& rotation = optix::Matrix3x3::identity())
{
  optix::float3 dir = rotation*direction;
  return optix::make_float2(0.5f + 0.5f*atan2f(dir.x, -dir.z)*M_1_PIf, acosf(-dir.y)*M_1_PIf);
}

__forceinline__ __device__ unsigned int cdf_bsearch_marginal(float xi)
{
  optix::uint table_size = marginal_cdf.size();
  optix::uint middle = table_size = table_size>>1;
  optix::uint odd = 0;
  while(table_size > 0)
  {
    odd = table_size&1;
    table_size = table_size>>1;
    optix::uint tmp = table_size + odd;
    middle = xi > marginal_cdf[middle]
      ? middle + tmp
      : (xi < marginal_cdf[middle - 1] ? middle - tmp : middle);
  }
  return middle;
}

__forceinline__ __device__ unsigned int cdf_bsearch_conditional(float xi, optix::uint offset)
{
  optix::size_t2 table_size = conditional_cdf.size();
  optix::uint middle = table_size.x = table_size.x>>1;
  optix::uint odd = 0;
  while(table_size.x > 0)
  {
    odd = table_size.x&1;
    table_size.x = table_size.x>>1;
    unsigned int tmp = table_size.x + odd;
    middle = xi > conditional_cdf[optix::make_uint2(middle, offset)]
      ? middle + tmp
      : (xi < conditional_cdf[optix::make_uint2(middle - 1, offset)] ? middle - tmp : middle);
  }
  return middle;
}

__forceinline__ __device__ optix::float3 env_lookup(const optix::float3& dir)
{
  float theta = acosf(dir.y);
  float phi = atan2f(dir.x, dir.z);
  float u = (phi + M_PIf)*0.5f*M_1_PIf;
  float v = 1.0 - theta*M_1_PIf;
  return make_float3(tex2D(envmap, u, v));
}

__device__ __inline__ void sample_environment(const optix::float3& pos, optix::float3& dir, optix::float3& L, optix::uint& t)
{
  const float M_2PIPIf = 2.0f*M_PIf*M_PIf;

  optix::size_t2 count = conditional_cdf.size();
  float xi1 = rnd_tea(t), xi2 = rnd_tea(t);

  optix::uint v_idx = cdf_bsearch_marginal(xi1);
  float dv = v_idx > 0
    ? (xi1 - marginal_cdf[v_idx - 1])/(marginal_cdf[v_idx] - marginal_cdf[v_idx - 1])
    : xi1/marginal_cdf[v_idx];
  float pdf_m = marginal_pdf[v_idx];
  float v = (v_idx + dv)/count.y;

  optix::uint u_idx = cdf_bsearch_conditional(xi2, v_idx);
  optix::uint2 uv_idx_prev = optix::make_uint2(u_idx - 1, v_idx);
  optix::uint2 uv_idx = optix::make_uint2(u_idx, v_idx);
  float du = u_idx > 0
    ? (xi2 - conditional_cdf[uv_idx_prev])/(conditional_cdf[uv_idx] - conditional_cdf[uv_idx_prev])
    : xi2/conditional_cdf[uv_idx];
  float pdf_c = conditional_pdf[uv_idx];
  float u = (u_idx + du)/count.x;

  float probability = pdf_m*pdf_c;
  float theta = (1.0f - v)*M_PIf;
  float phi = (2.0f*u - 1.0f)*M_PIf;
  float sin_theta, cos_theta, sin_phi, cos_phi;
  sincosf(theta, &sin_theta, &cos_theta);
  sincosf(phi, &sin_phi, &cos_phi);
  dir = make_float3(sin_theta*sin_phi, cos_theta, sin_theta*cos_phi);
  optix::float3 emission = make_float3(tex2D(envmap, u, v));
  L = emission*sin_theta*M_2PIPIf/probability;
}

#endif // ENVMAP_H
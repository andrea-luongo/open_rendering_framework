// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "envmap.h"

using namespace optix;

// Window variables
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );

// Buffers
rtBuffer<float, 2> env_luminance;
rtBuffer<float> marginal_f;

__host__ __device__ __inline__ float luminance_NTSC(optix::float3 & color)
{
  return optix::dot(color, make_float3(0.2989f, 0.5866f, 0.1145f));
}

RT_PROGRAM void env_luminance_camera()
{
  float2 uv = (make_float2(launch_index) + 0.5f)/make_float2(launch_dim);
  float theta = (1.0f - uv.y)*M_PIf;
  float3 texel = make_float3(tex2D(envmap, uv.x, uv.y));
  env_luminance[launch_index] = luminance_NTSC(texel)*sin(theta);
}

RT_PROGRAM void env_marginal_camera()
{
  if(launch_index.x == 0)
  {
    float c_f_sum = 0.0f;
    for(uint i = 0; i < launch_dim.x; ++i)
    {
      uint2 idx = make_uint2(i, launch_index.y);
      c_f_sum += env_luminance[idx];
    }
    marginal_f[launch_index.y] = c_f_sum/launch_dim.x;
  }
}

RT_PROGRAM void env_pdf_camera()
{
  conditional_pdf[launch_index] = env_luminance[launch_index]/marginal_f[launch_index.y];
  float cdf_sum = 0.0f;
  for(uint i = 0; i <= launch_index.x; ++i)
  {
    uint2 idx = make_uint2(i, launch_index.y);
    cdf_sum += env_luminance[idx];
  }
  cdf_sum /= launch_dim.x;
  conditional_cdf[launch_index] = cdf_sum/marginal_f[launch_index.y];
  if(launch_index == launch_dim - 1)
    conditional_cdf[launch_index] = 1.0f;  // handle numerical instability

  if(launch_index.x == 0)
  {
    float m_f_sum = 0.0f;
    for(uint i = 0; i < marginal_f.size(); ++i)
    {
      m_f_sum += marginal_f[i];
      if(i == launch_index.y)
        cdf_sum = m_f_sum;
    }
    m_f_sum /= launch_dim.y;
    cdf_sum /= launch_dim.y;
    marginal_pdf[launch_index.y] = marginal_f[launch_index.y]/m_f_sum;
    marginal_cdf[launch_index.y] = cdf_sum/m_f_sum;
    if(launch_index.y == launch_dim.y - 1)
      marginal_cdf[launch_index.y] = 1.0f; // handle numerical instability
  }
}

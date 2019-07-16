#ifndef FRESNEL_H
#define FRESNEL_H

#include <optixu/optixu_math_namespace.h>
#include <optix.h>
#include "MyComplex.h"
#include <optix_math.h>
__host__ __device__ __inline__ optix::float3 sqrtf(const optix::float3& v)
{
  return optix::make_float3(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z));
}

// Helper functions for computing Fresnel reflectance
__device__ __inline__ float fresnel_r_s(float cos_theta1, float cos_theta2, float ior1_over_ior2)
{
	// Compute the perpendicularly polarized component of the Fresnel reflectance
	return (ior1_over_ior2*cos_theta1 - cos_theta2) / (ior1_over_ior2*cos_theta1 + cos_theta2);
}

__device__ __inline__ float fresnel_r_p(float cos_theta1, float cos_theta2, float ior1_over_ior2)
{
	// Compute the parallelly polarized component of the Fresnel reflectance
	return (cos_theta1 - ior1_over_ior2*cos_theta2) / (cos_theta1 + ior1_over_ior2*cos_theta2);
}

__device__ __inline__ float fresnel_R(float cos_theta1, float cos_theta2, float ior1_over_ior2)
{
	// Compute the Fresnel reflectance using fresnel_r_s(...) and fresnel_r_p(...)
	float r_s = fresnel_r_s(cos_theta1, cos_theta2, ior1_over_ior2);
	float r_p = fresnel_r_p(cos_theta1, cos_theta2, ior1_over_ior2);
	return (r_s*r_s + r_p*r_p)*0.5f;
}

__host__ __device__ __inline__ float fresnel_R(float cos_theta, float ior1_over_ior2)
{
  float sin_theta_t_sqr = ior1_over_ior2*ior1_over_ior2*(1.0f - cos_theta*cos_theta);
  if(sin_theta_t_sqr >= 1.0f) return 1.0f;
  float cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
  return fresnel_R(cos_theta, cos_theta_t, ior1_over_ior2);
}

__host__ __device__ __inline__ float fresnel_R(const optix::float3& w_i, const optix::float3& w_m, const float ior1_over_ior2)
{
  // Compute the Fresnel reflectance using fresnel_r_s(...) and fresnel_r_p(...)
  float cos_theta1 = dot(w_i, w_m);
  float cos_theta22 = 1.0f - ior1_over_ior2 * ior1_over_ior2 * (1.0f - cos_theta1 * cos_theta1);
  if(cos_theta22 <= 0.0f)
    return 1.0f;

  float cos_theta2 = sqrtf(cos_theta22);
  float r_s = fresnel_r_s(cos_theta1, cos_theta2, ior1_over_ior2);
  float r_p = fresnel_r_p(cos_theta1, cos_theta2, ior1_over_ior2);
  return (r_s*r_s + r_p*r_p)*0.5f;
}

__host__ __device__ __inline__ optix::float3 fresnel_MyComplex_R(float cos_theta, const optix::float3& eta_sq, const optix::float3& kappa_sq)
{
  float cos_theta_sqr = cos_theta*cos_theta;
  float sin_theta_sqr = 1.0f - cos_theta_sqr;
  float tan_theta_sqr = sin_theta_sqr/cos_theta_sqr;

  optix::float3 z_real = eta_sq - kappa_sq - sin_theta_sqr;
  optix::float3 z_imag = 4.0f*eta_sq*kappa_sq;
  optix::float3 abs_z = sqrtf(z_real*z_real + z_imag*z_imag);
  optix::float3 two_a = sqrtf(2.0f*(abs_z + z_real));

  optix::float3 c1 = abs_z + cos_theta_sqr;
  optix::float3 c2 = two_a*cos_theta;
  optix::float3 R_s = (c1 - c2)/(c1 + c2);

  c1 = abs_z + sin_theta_sqr*tan_theta_sqr;
  c2 = two_a*sin_theta_sqr/cos_theta;
  optix::float3 R_p = R_s*(c1 - c2)/(c1 + c2);

  return (R_s + R_p)*0.5f;
}

__device__ __inline__ optix::float3 fresnel_MyComplex_R(const optix::float3 w_i, const optix::float3 w_m, const MyComplex3 ior1_over_ior2)
{

  float cos_theta_i = dot(w_i, w_m);
  float sin_theta_i = sqrtf(1 - cos_theta_i * cos_theta_i);

  MyComplex3 sin_theta_t = ior1_over_ior2 * sin_theta_i;
  MyComplex3 cos_theta_t = sqrt(1.0f - pow(sin_theta_t, 2));
  MyComplex3 r_ort = (ior1_over_ior2*cos_theta_i - cos_theta_t) / (ior1_over_ior2 *cos_theta_i + cos_theta_t);
  MyComplex3 r_par = (cos_theta_i - ior1_over_ior2*cos_theta_t) / (cos_theta_i + ior1_over_ior2*cos_theta_t);
  optix::float3 R = 0.5f*(fpowf(abs(r_ort), 2.0f) + fpowf(abs(r_par), 2));

  return R;
}

__host__ __device__ __inline__ float two_C1(float n)
{
  float r;
  if(n >= 1.0f)
    r = -9.23372f + n*(22.2272f + n*(-20.9292f + n*(10.2291f + n*(-2.54396f + n*0.254913f))));
  else
    r = 0.919317f + n*(-3.4793f + n*(6.75335f + n*(-7.80989f + n*(4.98554f - n*1.36881f))));
  return r;
}

__host__ __device__ __inline__ float three_C2(float n)
{
  float r;
  if(n >= 1.0f)
  {
    r = -1641.1f + n*(1213.67f + n*(-568.556f + n*(164.798f + n*(-27.0181f + n*1.91826f))));
    r += (((135.926f/n) - 656.175f)/n + 1376.53f)/n;
  }
  else
    r = 0.828421f + n*(-2.62051f + n*(3.36231f + n*(-1.95284f + n*(0.236494f + n*0.145787f))));
  return r;
}

__host__ __device__ __inline__ float C_phi(float ni)
{
  return 0.25f*(1.0f - two_C1(ni));
}

__host__ __inline__ float C_E(float ni)
{
  return 0.5f*(1.0f - three_C2(ni));
}


#endif // FRESNEL_H
#ifndef MICROFACET_H
#define MICROFACET_H

#include "MicrofacetBeckmann.h"
#include "MicrofacetGGX.h"


__inline__ __device__ float masking_G1(const float cos_theta, const float alpha_sqr, const uint normal_distribution)
{
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_G1(cos_theta, alpha_sqr);
	else
		return beckmann_G1(cos_theta, alpha_sqr);
}

__inline__ __device__ float masking_G1(const optix::float3 v, const optix::float3 m, const optix::float3 n, const float alpha, const uint normal_distribution)
{
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_G1(v, m, n, alpha);
	else
		return beckmann_G1(v, m, n, alpha);
}

__inline__ __device__ float masking_G1(const optix::float3 v, const optix::float3 m, const optix::float3 n, const float alpha_x, const float alpha_y, const uint normal_distribution)
{

	//	float alpha = alpha_i(transformToLocal(v, n), alpha_x, alpha_y);
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_G1(v, m, n, alpha_x, alpha_y);
	else
		return beckmann_G1(v, m, n, alpha_x, alpha_y);
}

__inline__ __device__ float masking_G(const float cos_theta_i, const float cos_theta_o, const float cosines, const float alpha, const uint normal_distribution)
{
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_G(cos_theta_i, cos_theta_o, cosines, alpha);
	else
		return beckmann_G(cos_theta_i, cos_theta_o, cosines, alpha);
}

__inline__ __device__ float masking_G(const float3 wi, const float h, const float a_x, const float a_y, const uint normal_distribution)
{
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_G(wi, h, a_x, a_y);
	else
		return beckmann_G(wi, h, a_x, a_y);
}

__inline__ __device__ void microfacet_sample_normal(const float3 normal, float3 &sampled_normal, const float z1, const float z2, const float alpha, const uint normal_distribution)
{
	if (normal_distribution == GGX_DISTRIBUTION) {
		//rtPrintf("GGX \n");
		ggx_sample_hemisphere(normal, sampled_normal, z1, z2, alpha);
	}
	else {
		//rtPrintf("beckmann \n");
		beckmann_sample_hemisphere(normal, sampled_normal, z1, z2, alpha);
	}
}

__inline__ __device__ float microfacet_eval_BSDF(const float3 w_i, const float3 w_o, const float3 n, const float alpha, const float ior1_over_ior2, const uint normal_distribution)
{
	float bsdf = 0.0f;
	if (normal_distribution == GGX_DISTRIBUTION)
		bsdf = ggx_microfacet_BSDF_eval(w_i, w_o, n, alpha, ior1_over_ior2);
	else
		bsdf = beckmann_microfacet_BSDF_eval(w_i, w_o, n, alpha, ior1_over_ior2);
	return bsdf;
}

__inline__ __device__ void microfacet_sample_visible_normal(const float3& wi, const float3& normal, float3 &sampled_normal,
	const float a_x, const float a_y, const float z1, const float z2, uint normal_distribution)
{
	float3 local_wi = transformToLocal(wi, normal);
	if (normal_distribution == GGX_DISTRIBUTION)
		sampled_normal = ggx_sample_VNDF(local_wi, a_x, a_y, z1, z2);
	else
		sampled_normal = beckmann_sample_VNDF(local_wi, a_x, a_y, z1, z2);

	sampled_normal = transformToWorld(sampled_normal, normal);
	return;
}

__inline__ __device__ float microfacet_eval_visible_normal(const float3& wi, const float3& wm, const float3& normal,
	const float a_x, const float a_y, uint normal_distribution)
{
	float3 local_wi = transformToLocal(wi, normal);
	float3 local_wm = transformToLocal(wm, normal);
	float D = 0.0f;
	if (normal_distribution == GGX_DISTRIBUTION)
		D = ggx_eval_VNDF(local_wi, local_wm, a_x, a_y);
	else
		D = beckmann_eval_VNDF(local_wi, local_wm, a_x, a_y);

	return D;
}


//Sample the multiscattering BSDF for dielectric
__inline__ __device__ void microfacet_multiscattering_dielectric_BSDF_sample(const float3& w_i, float3& w_o, const float3& normal,
	float eta, const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight, const uint normal_distribution)
{
	float3 local_wi = transformToLocal(w_i, normal);
	float3 local_wo;
	if (normal_distribution == GGX_DISTRIBUTION)
		ggx_multiscattering_dielectric_BSDF_sample(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder, weight);
	//ggx_multiscattering_dielectric_BSDF_sample_test(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder, weight);
	else
		beckmann_multiscattering_dielectric_BSDF_sample(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder, weight);
		//beckmann_multiscattering_dielectric_BSDF_sample_test(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder, weight);
	w_o = transformToWorld(local_wo, normal);
}

//Evaluate dielectric BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float microfacet_multiscattering_dielectric_BSDF_eval(const optix::float3& w_i, const optix::float3 w_o, const float3& normal,
	float eta, const float a_x, const float a_y, uint& seed, uint scatteringOrder,  const uint normal_distribution)
{
	float3 local_wi = transformToLocal(w_i, normal);
	float3 local_wo = transformToLocal(w_o, normal);
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_multiscattering_dielectric_BSDF_eval(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder);
	else
		return beckmann_multiscattering_dielectric_BSDF_eval(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder);
		//return beckmann_multiscattering_dielectric_BSDF_eval_test(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder);
}

//Sample the multiscattering BSDF for diffuse
__inline__ __device__ void microfacet_multiscattering_diffuse_BSDF_sample(const optix::float3& w_i, optix::float3& w_o, optix::float3& w_m,
	const float3& normal, const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight, const float3& rho_d, const uint normal_distribution)
{
	float3 local_wi = transformToLocal(w_i, normal);
	float3 local_wo;
	float3 local_wm;
	if (normal_distribution == GGX_DISTRIBUTION)
		ggx_multiscattering_diffuse_BSDF_sample(local_wi, local_wo, local_wm, a_x, a_y, seed, scatteringOrder, weight, rho_d);
	else
		beckmann_multiscattering_diffuse_BSDF_sample(local_wi, local_wo, local_wm, a_x, a_y, seed, scatteringOrder, weight, rho_d);

	w_o = transformToWorld(local_wo, normal);
	w_m = transformToWorld(local_wm, normal);
}

//Evaluate diffuse BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float3 microfacet_multiscattering_diffuse_BSDF_eval(const optix::float3& w_i, const optix::float3 w_o, const float3& normal,
	const float a_x, const float a_y, uint& seed, uint scatteringOrder, const float3& rho_d, const uint normal_distribution)
{
	float3 local_wi = transformToLocal(w_i, normal);
	float3 local_wo = transformToLocal(w_o, normal);
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_multiscattering_diffuse_BSDF_eval(local_wi, local_wo, a_x, a_y, seed, rho_d,scatteringOrder);
	else
		return beckmann_multiscattering_diffuse_BSDF_eval(local_wi, local_wo, a_x, a_y, seed, rho_d,scatteringOrder);
}

//Sample the multiscattering BSDF for conductor
__inline__ __device__ void microfacet_multiscattering_conductor_BSDF_sample(const optix::float3& w_i, optix::float3& w_o, optix::float3& w_m,
	const float3& normal, const MyComplex3 eta, const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight, const uint normal_distribution)
{
	float3 local_wi = transformToLocal(w_i, normal);
	float3 local_wo;
	float3 local_wm;
	if (normal_distribution == GGX_DISTRIBUTION)
		ggx_multiscattering_conductor_BSDF_sample(local_wi, local_wo, local_wm, eta, a_x, a_y, seed, scatteringOrder, weight);
	else
		beckmann_multiscattering_conductor_BSDF_sample(local_wi, local_wo, local_wm, eta, a_x, a_y, seed, scatteringOrder, weight);
	w_o = transformToWorld(local_wo, normal);
	w_m = transformToWorld(local_wm, normal);

}

//Evaluate conductor BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float3 microfacet_multiscattering_conductor_BSDF_eval(const optix::float3& w_i, const optix::float3 w_o, const optix::float3 normal, 
	const MyComplex3 eta, const float a_x, const float a_y, uint& seed, uint scatteringOrder, const uint normal_distribution)
{
	float3 local_wi = transformToLocal(w_i, normal);
	float3 local_wo = transformToLocal(w_o, normal);
	if (normal_distribution == GGX_DISTRIBUTION)
		return ggx_multiscattering_conductor_BSDF_eval(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder);
	else
		return beckmann_multiscattering_conductor_BSDF_eval(local_wi, local_wo, eta, a_x, a_y, seed, scatteringOrder);
}


#endif // MICROFACET_H
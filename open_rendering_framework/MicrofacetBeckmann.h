#ifndef BECKMANN_H
#define BECKMANN_H

#include "helpers.h"
#include <optixu/optixu_math_namespace.h>
#include <optix.h>
#include "fresnel.h"
#include "MyComplex.h"
#include "random.h"
using namespace optix;

// projected roughness, wi in local coordinates, the normal is (0,0,1)
__device__ __inline__ float beckmann_alpha_i(const optix::float3& wi, const float alpha_x, const float alpha_y){
	float invSinTheta2 = 1.0f / (1.0f - wi.z*wi.z);
	if (wi.z == 1.0f || wi.z == -1.0f)
		invSinTheta2 = 0.0f;
	float cosPhi2 = wi.x*wi.x*invSinTheta2;
	float sinPhi2 = wi.y*wi.y*invSinTheta2;
	float alpha = sqrtf(cosPhi2 * alpha_x * alpha_x + sinPhi2 * alpha_y * alpha_y);
	return alpha;
}

//Beckmann Smith Lambda function
__device__ __inline__ float beckmann_smith_lambda(const float3& wi, const float a_x, const float a_y){
	if (wi.z > 0.9999f){
		return 0.0f;
	}
	if (wi.z < -0.9999f){
		return -1.0f;
	}
	float theta_i = acosf(wi.z);
	float tan_theta_i = tanf(theta_i);
	if (isinf(tan_theta_i))
		return 0.0f;
	float a = 1.0f / (tan_theta_i * beckmann_alpha_i(wi, a_x, a_y));
	float lambda = 0.5f * ((float)erf(a) - 1.0f) + 0.25f * M_2_SQRTPIf / a *expf(-a*a);
	//if (isinf(lambda)){
	//	rtPrintf("theta %f, a %f, tanf %f, balpha %f \n", theta_i, a, tanf(theta_i), beckmann_alpha_i(wi, a_x, a_y));
	//	rtPrintf("wi %f %f %f \n", wi.x, wi.y, wi.z);
	//}
	return lambda;
}

//Beckmann Projected Area
__device__ __inline__ float beckmann_projected_area(const float3& wi, const float a_x, const float a_y){

	if (wi.z > 0.9999f)
		return 1.0f;
	if (wi.z < -0.9999f)
		return 0.0f;

	float alpha = beckmann_alpha_i(wi, a_x, a_y);
	float theta = acosf(wi.z);
	float a = 1.0f / tanf(theta) / alpha;

	float area = 0.5f * ((float)erf(a) + 1.0f)*wi.z + 0.25* M_2_SQRTPIf * alpha * sinf(theta) * expf(-a*a);
	return area;
}

//Beckmann slope distribution
__device__ __inline__ float beckmann_slope_pdf(const float slope_x, const float alpha_x, const float slope_y, const float alpha_y){

	float pdf = 1.0f / (M_PIf * alpha_x * alpha_y) *expf(-slope_x * slope_x / (alpha_x * alpha_x) - slope_y * slope_y / (alpha_y * alpha_y));
	return pdf;
}

//G1 masking shadowing function, Beckmann distribution
__inline__ __device__ float beckmann_G1(float cos_theta, float alpha_b_sqr)
{
	float cos_theta_sqr = cos_theta*cos_theta;
	float tan_theta_sqr = (1.0f - cos_theta_sqr) / cos_theta_sqr;
	float a_sqr = 1.0f / (alpha_b_sqr*tan_theta_sqr);
	float a = sqrtf(a_sqr);
	return a < 1.6f ? (3.535f*a + 2.181f*a_sqr) / (1.0f + 2.276f*a + 2.577*a_sqr) : 1.0f;
}

//G1 masking shadowing function, Beckmann distribution
__inline__ __device__ float beckmann_G1(const optix::float3& v,const optix::float3& m,const optix::float3& n, float width)
{
	float cos_theta_v = dot(v, n);
	if (optix::dot(v, m) * cos_theta_v <= 0)
		return 0.0f;

	float a_b_sqr = width * width;
	return beckmann_G1(cos_theta_v, a_b_sqr);
}

//G1 masking shadowing function, Beckmann distribution
__inline__ __device__ float beckmann_G1(const optix::float3& v, const optix::float3& m, const optix::float3& n, float a_x, float a_y)
{
	float width = beckmann_alpha_i(transformToLocal(v, n), a_x, a_y);
	float cos_theta_v = dot(v, n);
	if (optix::dot(v, m) * cos_theta_v <= 0)
		return 0.0f;

	float a_b_sqr = width * width;
	return beckmann_G1(cos_theta_v, a_b_sqr);
}

//G masking shadowing function, Beckmann distribution
__inline__ __device__ float beckmann_G(float cos_theta_i, float cos_theta_o, float cosines, float width)
{
  float alpha_b_sqr = width*width;
  return beckmann_G1(cos_theta_i, alpha_b_sqr)*beckmann_G1(cos_theta_o, alpha_b_sqr)*cosines;
}

//Evaluate the masking function at height h
__device__ __inline__ float beckmann_G(const optix::float3& wi, const float h, const float a_x, const float a_y){

	if (wi.z > 0.9999f)
		return 1.0f;
	if (wi.z <= 0.0f)
		return 0.0f;

	//height CDF
	float C1_h0 = GaussianHeightCDF(h);
	//rtPrintf("C1_h0=%f; h=%f\n", C1_h0, h);
	//Lambda function
	float Lambda = beckmann_smith_lambda(wi, a_x, a_y);

	float G = powf(C1_h0, Lambda);
	//if (isnan(G)){
	//	rtPrintf("C1 %f, lambda %f, G %f \n", C1_h0, Lambda, G);
	//	rtPrintf("w %f %f %f, h %f, ax %f ay %f; \n", wi.x, wi.y, wi.z, h, a_x, a_y);
	//}
	return G;
}

//Evaluate the masking function averaged over all heights h
__device__ __inline__ float beckmann_G(const optix::float3& wi, const float a_x, const float a_y){

	if (wi.z > 0.9999f)
		return 1.0f;
	if (wi.z <= 0.0f)
		return 0.0f;
	//Lambda function
	float Lambda = beckmann_smith_lambda(wi, a_x, a_y);

	float G = 1.0f / (1.0f + Lambda);
	return G;
}

//Beckmann distribution normal sampling
__device__ __inline__ void beckmann_sample_hemisphere(const optix::float3& normal,float3& sampled_normal, float z1, float z2, float width){

	float phi = 2 * M_PIf*z2;
	float theta;
	float tan_theta;
	tan_theta = -width*width * log(1 - z1);
	theta = atan(sqrtf(tan_theta));
	float costheta = cosf(theta);
	float sintheta = sinf(theta);
	sampled_normal = make_float3(sintheta * cosf(phi), sintheta * sinf(phi), costheta);
	//transform the sampled_normal coordinates from world to object
	float3 v1, v2;
	create_onb(normal, v1, v2);
	sampled_normal = v1 * sampled_normal.x + v2 * sampled_normal.y + normal * sampled_normal.z;
}

//Importance sampling the beckmann distribution of visible slopes
__device__ __inline__ float2 beckmann_sample_P22_11(const float theta_i, const float z1, const float z2){

	float2 slope;
	if (theta_i < 0.0001f){
		float r = sqrtf(-log(z1));
		float phi = M_PIf * 2 * z2;
		slope.x = r * cosf(phi);
		slope.y = r * sinf(phi);
		return slope;
	}
	float sin_theta_i = sinf(theta_i);
	float cos_theta_i = cosf(theta_i);
	float slope_i = cos_theta_i / sin_theta_i;
	float a = cos_theta_i / sin_theta_i;
	float proj_area = 0.5f * ((float)erf(a) + 1.0f) * cos_theta_i + 0.25f * M_2_SQRTPIf * sin_theta_i * expf(-a*a);
	
	if (proj_area < 0.0001f || proj_area != proj_area)
		return make_float2(0.0f, 0.0f);

	float c = 1.0f / proj_area;
	float erf_min = -0.9999f;
	float erf_max = max(erf_min, (float)erf(slope_i));
	float erf_current = 0.5f * (erf_min + erf_max);

	while (erf_max - erf_min > 0.00001f){
		if (!(erf_current >= erf_min && erf_current <= erf_max))
			erf_current = 0.5f * (erf_min + erf_max);
		float s = erfinv(erf_current);
		float CDF = (s >= slope_i) ? 1.0f : c * (0.25* M_2_SQRTPIf * sin_theta_i * expf(-s*s) + cos_theta_i * (0.5f + 0.5f * (float)erf(s)));
		float diff = CDF - z1;
		if (abs(diff) < 0.00001f)
			break;
		if (diff > 0.0f){
			if (erf_max == erf_current)
				break;
			erf_max = erf_current;
		}
		else{
			if (erf_min == erf_current)
				break;
			erf_min = erf_current;
		}
		float derivative = 0.5f * c * cos_theta_i - 0.5f * c * sin_theta_i * s;
		erf_current -= diff / derivative;
	}

	slope.x = erfinv(min(erf_max, max(erf_min, erf_current)));
	slope.y = erfinv(2.0f * z2 - 1.0f);
	return slope;
}

//Beckmann Visible normal importance sampling, wi is in local coordinates and the normal is (0,0,1)
__device__ __inline__ float3 beckmann_sample_VNDF(const float3& wi, const float a_x, const float a_y, const float z1, const float z2){

	float3 wi_11 = normalize(make_float3(a_x * wi.x, a_y * wi.y, wi.z));
	float2 slope_11 = beckmann_sample_P22_11(acosf(wi_11.z), z1, z2);
	float phi = atan2(wi_11.y, wi_11.x);
	float2 slope = make_float2(cosf(phi) * slope_11.x - sinf(phi) * slope_11.y, sinf(phi) * slope_11.x + cos(phi) * slope_11.y);

	slope.x *= a_x;
	slope.y *= a_y;

	if (!isfinite(slope.x) || slope.x != slope.x){
		if (wi.z > 0)
			return make_float3(0.0f, 0.0f, 1.0f);
		else
			return normalize(make_float3(wi.x, wi.y, 0.0f));
	}
	float3 wm = normalize(make_float3(-slope.x, -slope.y, 1.0f));
	return wm;
}

//Beckmann normal importance sampling, wi is in local coordinates and the normal is (0,0,1)
__device__ __inline__ float3 beckmann_sample_NDF(const float a_x, const float a_y, const float z1, const float z2){

	float2 slope;
	slope.x = a_x * sqrtf(-logf(z1)) * cos(2*M_PIf *z2);
	slope.y = a_y * sqrtf(-logf(z1)) * sin(2 * M_PIf *z2);
	float3 wm = normalize(make_float3(-slope.x, -slope.y, 1.0f));
	//if (isnan(wm.x) || isnan(wm.y) || isnan(wm.z)) {
	//	rtPrintf(" NAN\n");
	//	rtPrintf("m %f %f %f \n", wm.x, wm.y, wm.z);
	//	rtPrintf("slope.x %f slope.y %f log_z1 %f\n", slope.x, slope.y, -logf(z1));
	//}
	return wm;
}

//Beckmann distribution of normals
__device__ __inline__ float beckmann_eval_NDF(const float3& wm, const float a_x, const float a_y){

	if (wm.z <= 0.0f)
		return 0.0f;

	//slope of wm
	float slope_x = -wm.x / wm.z;
	float slope_y = -wm.y / wm.z;

	// value
	float D = beckmann_slope_pdf(slope_x, a_x, slope_y, a_y) / (wm.z * wm.z * wm.z * wm.z);
	return D;
}

//Beckmann distribution of visible normals
__device__ __inline__ float beckmann_eval_VNDF(const float3& wi, const float3& wm, const float a_x, const float a_y){


	if (wm.z <= 0.0f)
		return 0.0f;

	//normalization coefficient
	float area = beckmann_projected_area(wi, a_x, a_y);
	if (area == 0.0f)
		return 0.0f;

	float c = 1.0f / area;

	// value
	float D = beckmann_eval_NDF(wm, a_x, a_y);
	float result = c * max(0.0f, dot(wi, wm)) * D;
	return result;
}

//Sample height distribution
__device__ __inline__ float beckmann_sample_height(const float3& wr, const float hr, const float z, const float a_x, const float a_y){

	if (wr.z > 0.999f){
		return RT_DEFAULT_MAX;
	}
	if (wr.z < -0.999f){
		float value = GaussianHeightInvCDF(z * GaussianHeightCDF(hr));
		return value;
	}
	if (fabsf(wr.z) < 0.00001f)
		return hr;

	//probability of intersection
	float G_1 = beckmann_G(wr,  a_x, a_y) ;
	//rtPrintf("G=%f, z=%f\n", G_1, z);
	//Leave the microsurface
	if (z > 1.0f - G_1)
		return RT_DEFAULT_MAX;
	float gaussCDF = GaussianHeightCDF(hr);
	float lambda = beckmann_smith_lambda(wr, a_x, a_y);
	float pow = powf((1.0f - z), 1.0f / beckmann_smith_lambda(wr, a_x, a_y));
	float h = GaussianHeightInvCDF(gaussCDF / pow);
	return h;
}

//Sample dielectric phase function p(wi,wo)
__device__ __inline__ float3 beckmann_sample_dielectric_phase(const optix::float3& w_i, optix::float3& w_m, const float& m_eta, uint& seed,
	const bool wi_outside, bool& wo_outside, float& F, const float a_x, const float a_y)
{
	float z1 = rnd_tea(seed);
	float z2 = rnd_tea(seed);

	float eta = wi_outside ? m_eta : 1.0f / m_eta;
	//float eta = m_eta;
	w_m = wi_outside ? beckmann_sample_VNDF(w_i, a_x, a_y, z1, z2) : -beckmann_sample_VNDF(-w_i, a_x, a_y, z1, z2);

	float cos_theta = dot(w_m, w_i);
	cos_theta = fabsf(cos_theta);
	float sin_theta_t_sqr = eta*eta*(1.0f - cos_theta*cos_theta);
	float cos_theta_t = sqrtf(1.0f - sin_theta_t_sqr);
	F = sin_theta_t_sqr < 1.0f ? fresnel_R(cos_theta, cos_theta_t, eta) : 1.0f;

	if (rnd_tea(seed) <= F){
		float3 w_o = -w_i + 2.0f * w_m * dot(w_i, w_m);
		//rtPrintf("eta=%f; w_i=[%f,%f,%f]; w_m=[%f,%f,%f]; wi_outside=%u; wo_outside=%u; w_o=[%f,%f,%f];\n", eta, w_i.x, w_i.y, w_i.z, w_m.x, w_m.y, w_m.z, wi_outside, wo_outside,w_o.x,w_o.y,w_o.z);
		return normalize(w_o);
	}
	else{
		wo_outside = !wi_outside;
		float3 w_o = -eta*w_i + w_m*(eta*cos_theta - cos_theta_t);
		//rtPrintf("eta=%f; w_i=[%f,%f,%f]; w_m=[%f,%f,%f]; wi_outside=%u; wo_outside=%u; w_o=[%f,%f,%f];\n", eta, w_i.x, w_i.y, w_i.z, w_m.x, w_m.y, w_m.z, wi_outside, wo_outside, w_o.x, w_o.y, w_o.z);
		return normalize(w_o);
	}

}

//Evaluate dielectric phase function p(wi,wo)
__device__ __inline__ float beckmann_eval_dielectric_phase(const optix::float3& w_i, const optix::float3& w_o,
	const bool wi_outside, const bool wo_outside, const float& m_eta, const float a_x, const float a_y, float3& w_h)
{
	float eta = wi_outside ? m_eta : 1.0f / m_eta;
	if (wi_outside == wo_outside){
		//reflection
		//half vector
		 w_h = normalize(w_i + w_o);
		float D = (wi_outside) ? beckmann_eval_VNDF(w_i, w_h, a_x, a_y) : beckmann_eval_VNDF(-w_i, -w_h, a_x, a_y);
		float F = (wi_outside) ? fresnel_R(w_i, w_h, eta) : fresnel_R(-w_i, -w_h, eta);
		float a = (wi_outside) ? 0.25f / dot(w_i, w_h) : 0.25f / dot(-w_i, -w_h);
		float value = F * D * a;
		return value;
	}
	else{
		//refraction
		 w_h = -normalize(w_i + w_o / eta);
		w_h *= (wi_outside) ? w_h.z / fabsf(w_h.z) : -w_h.z / fabsf(w_h.z); //?
		float value = 0.0f;
		if (dot(w_h, w_i) < 0.0f)
			return value;
		float D = (wi_outside) ? beckmann_eval_VNDF(w_i, w_h, a_x, a_y) : beckmann_eval_VNDF(-w_i, -w_h, a_x, a_y);
		float F = (wi_outside) ? fresnel_R(w_i, w_h, eta) : fresnel_R(-w_i, -w_h, eta);
		float b = 1 / (eta*eta);
		float c = (wi_outside) ? fmaxf(0.0f, -dot(w_o, w_h)) : fmaxf(0.0f, -dot(-w_o, -w_h));
		float e = (wi_outside) ? dot(w_i, w_h) + dot(w_o, w_h) / eta : dot(-w_i, -w_h) + dot(-w_o, -w_h) / eta;
		float d = e * e;
		float a = b * c / d;
		value = a * D * (1 - F);
		return value;
	}
}

//Sample diffuse phase function p(wi,wo)
__device__ __inline__ float3 beckmann_sample_diffuse_phase(const optix::float3& w_i, optix::float3& w_m, uint& seed,
	const float a_x, const float a_y)
{
	float z1 = rnd_tea(seed);
	float z2 = rnd_tea(seed);
	float z3 = rnd_tea(seed);
	float z4 = rnd_tea(seed);

	w_m = beckmann_sample_VNDF(w_i, a_x, a_y, z1, z2);
	float3 w_1, w_2;
	create_onb(w_m, w_1, w_2);

	float r1 = 2.0f * z3 - 1.0f;
	float r2 = 2.0f * z4 - 1.0f;

	float phi, r;
	if (r1 == 0 && r2 == 0){
		r = phi = 0;
	}
	else if (r1 * r1 > r2 * r2){
		r = r1;
		phi = (M_PIf / 4.0f) * (r2 / r1);
	}
	else{
		r = r2;
		phi = (M_PIf / 2.0f) - (r1 / r2) * (M_PIf / 4.0f);
	}
	float x = r * cosf(phi);
	float y = r * sinf(phi);
	float z = sqrtf(fmaxf(0.0f, 1.0f - x*x - y*y));
	float3 w_o = x * w_1 + y * w_2 + z * w_m;

	return w_o;
}

//Evaluate diffuse phase function p(wi,wo)
__device__ __inline__ float beckmann_eval_diffuse_phase(const optix::float3& w_i, const optix::float3& w_o,
	float3& w_m, const float a_x, const float a_y, uint& seed)
{
	float z1 = rnd_tea(seed);
	float z2 = rnd_tea(seed);
	w_m = beckmann_sample_VNDF(w_i, a_x, a_y, z1, z2);
	//w_m = make_float3(0, 0, 1);
	float value = 1.0f * M_1_PIf * fmaxf(0.0f, dot(w_o, w_m));
	return value;
}

//Sample conductor phase function p(wi,wo)
__device__ __inline__ float3 beckmann_sample_conductor_phase(const optix::float3& w_i, optix::float3& w_m, uint& seed,
	const float a_x, const float a_y)
{
	float z1 = rnd_tea(seed);
	float z2 = rnd_tea(seed);
	w_m = beckmann_sample_VNDF(w_i, a_x, a_y, z1, z2);
	float3 w_o = -w_i + 2.0f*w_m*dot(w_i, w_m);
	return w_o;
}

//Evaluate conductor phase function p(wi,wo)
__device__ __inline__ float beckmann_eval_conductor_phase(const optix::float3& w_i, const optix::float3& w_o,
	float3& w_m, const float a_x, const float a_y)
{
	w_m = normalize(w_i + w_o);
	if (w_m.z < 0.0f)
		return 0.0f;
	float value = 0.25f * beckmann_eval_VNDF(w_i, w_m, a_x, a_y);
	return value;
}

//Sample the multiscattering BSDF for dielectric
__inline__ __device__ void beckmann_multiscattering_dielectric_BSDF_sample(const float3& w_i, float3& w_o,
	float eta, const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight)
{
	float hr = 1.0f + GaussianHeightInvCDF(0.9999f);
	w_o = -w_i;
	float F;
	float3 w_m;
	bool outside = (-w_o.z) > 0;
	weight = make_float3(1.0f);
	//importance sampling w_o;
	while (true){
		//next height
		float z = rnd_tea(seed);
		hr = (outside) ? beckmann_sample_height(w_o, hr, z, a_x, a_y) : -beckmann_sample_height(-w_o, -hr, z, a_x, a_y);
		//leave the microsurface?
		if (hr == RT_DEFAULT_MAX || hr == -RT_DEFAULT_MAX){
			break;
		}
		else{
			scatteringOrder++;
		}
		// next event estimation
		w_o = beckmann_sample_dielectric_phase(-w_o, w_m, eta, seed, outside, outside, F, a_x, a_y);
		if (hr != hr || w_o.z != w_o.z){
		//	weight *= 0.0f;
			w_o = make_float3(0, 0, 1);
			break;
		}
	}
	//rtPrintf("scattery %u \n", scatteringOrder);
	
}

//Evaluate dielectric BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float beckmann_multiscattering_dielectric_BSDF_eval(const optix::float3& w_i, 
	const optix::float3& w_o, float eta, const float a_x, const float a_y, uint& seed, uint scatteringOrder)
{
	float3 w_r = -w_i;
	float hr = 1.0f + GaussianHeightInvCDF(0.9999f);;
	int current_scatteringOrder = 0;
	float sum = 0.0f;
	float3 w_m;
	float shadowing = 1;
	float phasefunction = 1;
	float F;
	bool wo_out = w_o.z > 0;
	bool wr_out = (-w_r.z>0);
	float I = 0;
	//rtPrintf("w_i=[%f %f %f]; \nw_o=[%f %f %f];\neta=%f;\nax=%f;\nseed=%u;\nhr=%f;\n", w_i.x, w_i.y, w_i.z, w_o.x, w_o.y, w_o.z, eta, a_x, seed,hr);
	while (scatteringOrder == 0 || current_scatteringOrder <= scatteringOrder){
		//next height
		float z = rnd_tea(seed);	
		//wr_out = (-w_r.z>0);
		hr = (wr_out) ? beckmann_sample_height(w_r, hr, z, a_x, a_y) : -beckmann_sample_height(-w_r, -hr, z, a_x, a_y);
		//leave the microsurface?
		if (hr == RT_DEFAULT_MAX || hr == -RT_DEFAULT_MAX || isinf(hr) || isinf(-hr)){
			break;
		}
		else{
			current_scatteringOrder++;
		}
		// next event estimation
		float3 w_h;
		phasefunction = beckmann_eval_dielectric_phase(-w_r, w_o, wr_out, wo_out, eta, a_x, a_y,w_h);
		shadowing = (wo_out) ? beckmann_G(w_o, hr, a_x, a_y) : beckmann_G(-w_o, -hr, a_x, a_y);
		I = phasefunction * shadowing;
		if (isfinite(I) && (scatteringOrder == 0 || current_scatteringOrder == scatteringOrder))
		{		
			sum += I;
		}
		
		//next direction
		w_r = beckmann_sample_dielectric_phase(-w_r, w_m, eta, seed, wr_out, wr_out, F, a_x, a_y);
		//rtPrintf("seed=%u; hr=%f; phase=%f; shadow=%f;w_r=[%f,%f,%f];\n",seed,hr,phasefunction,shadowing,w_r.x,w_r.y,w_r.z);
		if (hr != hr || w_r.z != w_r.z){
			sum = 0.0f;
			break;
		}
	}

	//rtPrintf("finalSeed=%u;\niterations=%u; \nbsdf = %f;\n\n", seed,current_scatteringOrder, sum);
	return sum;
}

//Sample the multiscattering BSDF for dielectric
__inline__ __device__ void beckmann_multiscattering_dielectric_BSDF_sample_test(const float3& w_i, float3& w_o,
	float eta, const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight)
{
	//float hr = 1.0f + GaussianHeightInvCDF(0.9999f);
	float F;
	float3 w_m;
	scatteringOrder = 0;
	w_o = -w_i;
	bool outside = (-w_o.z) > 0;
	weight = make_float3(1.0f);
	float3 n = make_float3(0, 0, 1);
	float G = 1;
	float z;
	//importance sampling w_o;
	while (scatteringOrder < 10){
		//next height
		z = rnd_tea(seed);
		// next event estimation
		w_o = beckmann_sample_dielectric_phase(-w_o, w_m, eta, seed, outside, outside, F, a_x, a_y);
		//leave the microsurface?
		G = beckmann_G1(-w_o, w_m, n, a_x, a_y);
		//G = 1;
		if (z < G){
			//leave surface
			weight *= 1; // *G/G
			break;
		}
		else{
			scatteringOrder++;
		}
	}
	//rtPrintf("\n");
}

//Evaluate dielectric BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float beckmann_multiscattering_dielectric_BSDF_eval_test(const optix::float3& w_i,
	const optix::float3& w_o, float eta, const float a_x, const float a_y, uint& seed, uint scatteringOrder)
{
	float3 w_r = -w_i;
	
	int current_scatteringOrder = 0;
	float sum = 0.0f;
	float3 w_m;
	float G = 1;
	float phasefunction = 1;
	float F;
	bool wo_out = w_o.z > 0;
	bool wr_out = (-w_r.z>0);
	float3 n = make_float3(0, 0, 1);
	float I = 0;
	while (scatteringOrder == 0 || current_scatteringOrder <= scatteringOrder){
		//next height
		float z = rnd_tea(seed);
		//wr_out = (-w_r.z>0);

		// next event estimation
		float3 w_h;
		phasefunction = beckmann_eval_dielectric_phase(-w_r, w_o, wr_out, wo_out, eta, a_x, a_y,w_h);
		G = beckmann_G1(-w_r, w_h, n, a_x, a_y)*beckmann_G1(w_o, w_m, n, a_x, a_y);
		I = phasefunction * G;
		//leave the microsurface?
		if (z < G){
			if (isfinite(I) && (scatteringOrder == 0 || current_scatteringOrder == scatteringOrder))
			{
				sum += I/G;
			}
			break;
		}
		else{
			if (isfinite(I) && (scatteringOrder == 0 || current_scatteringOrder == scatteringOrder))
			{
				sum += I/(1-G);
			}
			current_scatteringOrder++;
		}

		//next direction
		w_r = beckmann_sample_dielectric_phase(-w_r, w_m, eta, seed, wr_out, wr_out, F, a_x, a_y);

	}
	return sum;
}
//Sample the diffuse BSDF with a random walk, return the outgoing direction wo
__device__ __inline__ void beckmann_multiscattering_diffuse_BSDF_sample(const optix::float3& w_i, optix::float3& w_o, optix::float3& w_m,
	const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight, const float3& rho_d)
{
	weight = make_float3(1.0f);
	float hr = 1.0f + GaussianHeightInvCDF(0.9999f);
	w_o = -w_i;
	while (true && scatteringOrder < 100){
		float u = rnd_tea(seed);
		hr = beckmann_sample_height(w_o, hr, u, a_x, a_y);
		if (hr == RT_DEFAULT_MAX){
			break;
		}
		else{
			scatteringOrder++;
			weight = weight * rho_d;
		}
		w_o = beckmann_sample_diffuse_phase(-w_o, w_m, seed, a_x, a_y);

		if (hr != hr || w_o.z != w_o.z){
			w_o = make_float3(0, 0, 1);
			break;
		}
	}
	//weight = weight * rho_d;
	//float z1 = rnd_tea(seed);
	//float z2 = rnd_tea(seed);
	//w_m = beckmann_sample_VNDF(w_i, a_x, a_y, z1, z2);
}

//Evaluate diffuse BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float3 beckmann_multiscattering_diffuse_BSDF_eval(const optix::float3& w_i, const optix::float3& w_o,
	const float a_x, const float a_y, uint& seed, const float3& rho_d, uint scatteringOrder)
{
	float3 w_r = -w_i;
	float hr = 1.0f + GaussianHeightInvCDF(0.9999f);
	int current_scatteringOrder = 0;
	float3 sum = make_float3(0.0f);
	float3 w_m;
	while ((scatteringOrder == 0 || current_scatteringOrder < scatteringOrder)){
		float u = rnd_tea(seed);
		hr = beckmann_sample_height(w_r, hr, u, a_x, a_y);
		if (hr == RT_DEFAULT_MAX){
			break;
		}
		else{
			current_scatteringOrder++;
		}
		float phaseFunction = beckmann_eval_diffuse_phase(-w_r, w_o, w_m, a_x, a_y, seed);
		float shadowing = beckmann_G(w_o, hr, a_x, a_y);
		float3 I = rho_d*phaseFunction * shadowing;
		if (isfinite(I.x) && isfinite(I.y) && isfinite(I.z)) {
			sum +=  I;
		}
		w_r = beckmann_sample_diffuse_phase(-w_r, w_m, seed, a_x, a_y);
		if (hr != hr || w_r.z != w_r.z){
			sum = make_float3(0.0f);
			break;
		}
	}

	return sum;
}

//Evaluate conductor BSDF with a random walk, return sum(phase*G)
__device__ __inline__ float3 beckmann_multiscattering_conductor_BSDF_eval(const optix::float3& w_i, const optix::float3& w_o,
	const MyComplex3 eta, const float a_x, const float a_y, uint& seed, uint scatteringOrder)
{
	float3 w_r = -w_i;
	float hr = 1.0f + GaussianHeightInvCDF(0.9999f);
	int current_scatteringOrder = 0;
	float3 sum = make_float3(0.0f);
	float3 w_m;
	while ((scatteringOrder == 0 || current_scatteringOrder < scatteringOrder)){
		float u = rnd_tea(seed);
		hr = beckmann_sample_height(w_r, hr, u, a_x, a_y);
		if (hr == RT_DEFAULT_MAX){
			break;
		}
		else{
			current_scatteringOrder++;
		}
		float phaseFunction = beckmann_eval_conductor_phase(-w_r, w_o, w_m, a_x, a_y);
		float shadowing = beckmann_G(w_o, hr, a_x, a_y);
		float3 F = fresnel_MyComplex_R(-w_r, w_m, (-w_r.z > 0) ? eta : 1.0f / eta);
		float3 I = F*phaseFunction * shadowing;
		if (isfinite(I.x) && isfinite(I.y) && isfinite(I.z)){
			sum += I;
		}
		w_r = beckmann_sample_conductor_phase(-w_r, w_m, seed, a_x, a_y);
		if (hr != hr || w_r.z != w_r.z){
			sum = make_float3(0.0f);
			break;
		}
	}
	return sum;
}

//Sample the conductor BSDF with a random walk, return the outgoing direction wo
__device__ __inline__ void beckmann_multiscattering_conductor_BSDF_sample(const optix::float3& w_i, optix::float3& w_o, optix::float3& w_m,
	const MyComplex3 eta, const float a_x, const float a_y, uint& seed, uint& scatteringOrder, float3& weight)
{
	float hr = 1.0f + GaussianHeightInvCDF(0.9999f);
	w_o = -w_i;
	weight = make_float3(1, 1, 1);
	while (true){
		float u = rnd_tea(seed);
		hr = beckmann_sample_height(w_o, hr, u, a_x, a_y);
		if (hr == RT_DEFAULT_MAX){
			break;
		}
		else{
			scatteringOrder++;
		}
		float3 w_temp = -w_o;
		w_o = beckmann_sample_conductor_phase(-w_o, w_m, seed, a_x, a_y);
		float3 F = fresnel_MyComplex_R(w_temp, w_m, (w_temp.z > 0) ? eta : 1.0f / eta);
		weight = weight * F;
		if (hr != hr || w_o.z != w_o.z){
			w_o = make_float3(0, 0, 1);
			break;
		}
	}

}

//Evaluate Beckmann Distribution
__device__ __inline__ float beckmann_distribution_eval(const float3& m, const float3& n, const float alpha)
{
	float D = 0.0f;
	float cos_theta = dot(m, n);
	if (cos_theta > 0.0001f){
		float cos_theta_sqr = cos_theta * cos_theta;
		float sin_theta_sqr = 1 - cos_theta_sqr;
		float alpha_sqr = alpha * alpha;
		D = exp(-sin_theta_sqr / (cos_theta_sqr * alpha_sqr)) *M_1_PIf / (alpha_sqr * cos_theta_sqr * cos_theta_sqr);
	}
		return D;
}

//Evaluate Walter BSDF
__device__ __inline__ float beckmann_microfacet_BSDF_eval(const float3& w_i, const float3& w_o, const float3& n, const float alpha, const float ior1_over_ior2)
{

	uint isReflection = dot(w_o, n) > 0.0f ? 1 : 0;
	float3 m;
	
	if (isReflection){
		m = normalize(w_o + w_i);
	}
	else{
		m = -normalize(ior1_over_ior2 * w_i + w_o);
	}
	float bsdf = 0.0f;
	float R = fresnel_R(w_i, m, ior1_over_ior2);
	float i_dot_n = fabsf(dot(w_i, n));
	float o_dot_n = fabsf(dot(w_o, n));
	float G_i_m = beckmann_G1(w_i, m, n, alpha);
	float G_o_m = beckmann_G1(w_o, m, n, alpha);
	float G = G_i_m * G_o_m;
	float T = 1 - R;
	float D = beckmann_distribution_eval(m, n, alpha);
	if (isReflection){
		bsdf = R  * G * D / (4 * i_dot_n * o_dot_n);
	}
	else{
		float i_dot_m = fabsf(dot(w_i, m));
		float o_dot_m = fabsf(dot(w_o, m));
		float coeff = (i_dot_m * o_dot_m) / (i_dot_n * o_dot_n);
		float den =  (ior1_over_ior2 * i_dot_m + o_dot_m);
		bsdf = T * G * D * coeff / (den * den);
	}

	return bsdf;
}



#endif // BECKMANN_H